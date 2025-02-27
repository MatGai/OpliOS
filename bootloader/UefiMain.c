#pragma once
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PrintLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>
#include <IndustryStandard/PeImage.h>
#include <Guid/GlobalVariable.h>
#include <Guid/Acpi.h>

CHAR8* gEfiCallerBaseName = "OpliOS";
const UINT32 _gUefiDriverRevision = 0x1;

#define _DEBUG_ 1;
#define _DEBUG_INFO_ 1;

EFI_STATUS LAST_ERROR;

BOOLEAN
TRY(
    IN EFI_STATUS Status,
    IN CHAR16* Message
)
{
    if (EFI_ERROR(Status))
    {
        Print(L"[ %r ] - %s\n", Status, Message);
        LAST_ERROR = Status;
        return FALSE;
    }
    return TRUE;
}

VOID
DEBUG_LOG(
    EFI_STATUS Status,
    CHAR16* Message
)
{
#ifdef _DEBUG_
    if (EFI_ERROR(Status))
    {
        Print(L"\n[ %r ] - %s\n", Status, Message);
        LAST_ERROR = Status;
    }
#endif
}

VOID
INFO(
    CHAR16* Message
)
{
#ifdef _DEBUG_INFO_
    Print(L"\ns\n", Message);
#endif
};

/**
* @brief Needed for VisualUefi for some reason?
*/
EFI_STATUS
EFIAPI 
UefiUnload(
    EFI_HANDLE ImageHandle
)
{
    return EFI_SUCCESS;
}

static EFI_SYSTEM_TABLE*                  ST = NULL;
static EFI_BOOT_SERVICES*                 BS = NULL;
static EFI_RUNTIME_SERVICES*              RT = NULL;
static EFI_SIMPLE_TEXT_INPUT_PROTOCOL*   CIN = NULL;
static EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* COUT = NULL;



EFI_INPUT_KEY
getc(
    VOID
)
{
    EFI_EVENT e[1];

    EFI_INPUT_KEY k;
    memset(&k, 0, sizeof(EFI_INPUT_KEY));

    e[0] = CIN->WaitForKey;
    UINTN index = 0;
    BS->WaitForEvent(1, e, &index);

    if (!index)
        CIN->ReadKeyStroke(CIN, &k);

    return k;
}


UINT8 compare(const void* firstitem, const void* seconditem, UINT64 comparelength)
{
    // Using const since this is a read-only operation: absolutely nothing should be changed here.
    const UINT8* one = firstitem, * two = seconditem;
    for (UINT64 i = 0; i < comparelength; i++)
    {
        if (one[i] != two[i])
        {
            return 0;
        }
    }
    return 1;
}

STATIC CONST CHAR16 mem_types[16][27] = {
      L"EfiReservedMemoryType     ",
      L"EfiLoaderCode             ",
      L"EfiLoaderData             ",
      L"EfiBootServicesCode       ",
      L"EfiBootServicesData       ",
      L"EfiRuntimeServicesCode    ",
      L"EfiRuntimeServicesData    ",
      L"EfiConventionalMemory     ",
      L"EfiUnusableMemory         ",
      L"EfiACPIReclaimMemory      ",
      L"EfiACPIMemoryNVS          ",
      L"EfiMemoryMappedIO         ",
      L"EfiMemoryMappedIOPortSpace",
      L"EfiPalCode                ",
      L"EfiPersistentMemory       ",
      L"EfiMaxMemoryType          "
};

VOID print_memmap()
{
    EFI_STATUS memmap_status;
    UINTN MemMapSize = 0, MemMapKey, MemMapDescriptorSize;
    UINT32 MemMapDescriptorVersion;
    EFI_MEMORY_DESCRIPTOR* MemMap = NULL;
    EFI_MEMORY_DESCRIPTOR* Piece;
    UINT16 line = 0;

    memmap_status = BS->GetMemoryMap(&MemMapSize, MemMap, &MemMapKey, &MemMapDescriptorSize, &MemMapDescriptorVersion);
    if (memmap_status == EFI_BUFFER_TOO_SMALL)
    {
        MemMapSize += MemMapDescriptorSize;
        memmap_status = BS->AllocatePool(EfiBootServicesData, MemMapSize, (void**)&MemMap); // Allocate pool for MemMap
        if (EFI_ERROR(memmap_status)) // Error! Wouldn't be safe to continue.
        {
            Print(L"MemMap AllocatePool error. 0x%llx\r\n", memmap_status);
            return;
        }
        memmap_status = BS->GetMemoryMap(&MemMapSize, MemMap, &MemMapKey, &MemMapDescriptorSize, &MemMapDescriptorVersion);
    }
    if (EFI_ERROR(memmap_status))
    {
        Print(L"Error getting memory map for printing. 0x%llx\r\n", memmap_status);
    }

    Print(L"MemMapSize: %llu, MemMapDescriptorSize: %llu, MemMapDescriptorVersion: 0x%x\r\n", MemMapSize, MemMapDescriptorSize, MemMapDescriptorVersion);

    // There's no virtual addressing yet, so there's no need to see Piece->VirtualStart
    // Multiply NumOfPages by EFI_PAGE_SIZE or do (NumOfPages << EFI_PAGE_SHIFT) to get the end address... which should just be the start of the next section.
    for (Piece = MemMap; Piece < (EFI_MEMORY_DESCRIPTOR*)((UINT8*)MemMap + MemMapSize); Piece = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)Piece + MemMapDescriptorSize))
    {
        if (line % 20 == 0)
        {
            getc();
            Print(L"#   Memory Type                Phys Addr Start   Num Of Pages   Attr\r\n");
        }

        Print(L"%2d: %s 0x%016llx 0x%llx 0x%llx\r\n", line, mem_types[Piece->Type], Piece->PhysicalStart, Piece->NumberOfPages, Piece->Attribute);
        line++;
    }

    memmap_status = BS->FreePool(MemMap);
    if (EFI_ERROR(memmap_status))
    {
        Print(L"Error freeing print_memmap pool. 0x%llx\r\n", memmap_status);
    }
}

typedef struct {
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE*  GPUArray;             // This array contains the EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE structures for each available framebuffer
    UINT64                              NumberOfFrameBuffers; // The number of pointers in the array (== the number of available framebuffers)
} GPU_CONFIG;

/**
* @brief The entry point for the UEFI application.
* @param[in] EFI_HANDLE        - The image handle of the UEFI application
* @param[in] EFI_SYSTEM_TABLE* - The system table of the UEFI application
* @return EFI_STATUS - The status of the UEFI application (useful if driver loads this app)
*/
EFI_STATUS 
EFIAPI 
UefiMain(
    EFI_HANDLE ImageHandle, 
    EFI_SYSTEM_TABLE* SystemTable
)
{
    ST   = SystemTable;
    BS   = ST->BootServices;
    RT   = ST->RuntimeServices;

    CIN  = SystemTable->ConIn;
    COUT = SystemTable->ConOut;

    if( !TRY( COUT->ClearScreen(COUT), L"Failed to clear screen" ) )
    {
        getc();
        return LAST_ERROR;
    }

    COUT->SetAttribute(COUT, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK) );

    CHAR16 Buffer[48 + 1];
    UnicodeSPrint(Buffer, 48, L"Hello, %s\n", L"world");

    Print( Buffer );

    EFI_TIME time;
    if( !TRY( RT->GetTime( &time, NULL ), L"Failed to get time" ) )
    {
        getc();
        return LAST_ERROR;
    }

    Print(L"%02d/%02d/%04d - %02d:%02d:%0d.%d\r\n\n", time.Month, time.Day, time.Year, time.Hour, time.Minute, time.Second, time.Nanosecond);

    EFI_STATUS Status;
    EFI_EVENT                         TimerEvent;
    UINTN                             WaitIndex;

    UINT64 timeout_seconds = 10; // 10 seconds
    EFI_INPUT_KEY key = { 0 };

    if (!TRY( 
            gBS->CreateEvent(EVT_TIMER, TPL_CALLBACK, NULL, NULL, &TimerEvent),
            L"Failed to create timer event\n"
        )
    )
    {
        getc();
        return LAST_ERROR;
    }

    while (timeout_seconds > 0) 
    {
        Print(L"Continuing in %llu, press 's' to stop timer or press any other key to continue. \r", timeout_seconds);

        // Set the timer event to fire after 1 second (10,000,000 x 100ns)
        if (!TRY(gBS->SetTimer(TimerEvent, TimerRelative, 10000000), L"\nError setting timer event"))
        {
            gBS->CloseEvent(TimerEvent);
            getc();
            return LAST_ERROR;
        }

        // Wait on both the key event and the timer event
        EFI_EVENT WaitList[2] = { CIN->WaitForKey, TimerEvent };
        Status = gBS->WaitForEvent(2, WaitList, &WaitIndex);

        // Cancel the timer so it doesn't fire again
        gBS->SetTimer(TimerEvent, TimerCancel, 0);

        // check if the key event occurred
        if (Status != EFI_TIMEOUT)
        {
            if (WaitIndex == 0) 
            {  // Key event occurred

                if( !TRY( CIN->ReadKeyStroke(CIN, &key ), L"Error reading keystroke") )
                {
                    gBS->CloseEvent(TimerEvent);
                    getc();
                    return LAST_ERROR;
                }

                if (key.UnicodeChar == L's') 
                {
                    Print(L"\nTimer stopped.\n");
                    getc();
                }
                else 
                {
                    Print(L"\n");
                    if ( !TRY( CIN->Reset(CIN, FALSE), L"Error resetting input buffer") )
                    {
                        gBS->CloseEvent(TimerEvent);
                        getc();
                        return LAST_ERROR;
                    }
                }
                break;  // Exit the loop if a key was pressed
            }
        }

        // Timer event occurred: decrement the timeout counter.
        timeout_seconds--;
    }

    if (!timeout_seconds)
    {
        Print(L"\n");
    }
    Print(L"\r\n");

#ifdef _DEBUG_
    Print(L"EFI System Table Info\r\n   Signature: 0x%lx\r\n   UEFI Revision: 0x%08x\r\n   Header Size: %u Bytes\r\n   CRC32: 0x%08x\r\n   Reserved: 0x%x\r\n", ST->Hdr.Signature, ST->Hdr.Revision, ST->Hdr.HeaderSize, ST->Hdr.CRC32, ST->Hdr.Reserved);
#else
    Print(L"EFI System Table Info\r\n   Signature: 0x%lx\r\n   UEFI Revision: %u.%u", ST->Hdr.Signature, ST->Hdr.Revision >> 16, (ST->Hdr.Revision & 0xFFFF) / 10);
    if ((ST->Hdr.Revision & 0xFFFF) % 10)
    {
        Print(L".%u\r\n", (ST->Hdr.Revision & 0xFFFF) % 10); // UEFI major.minor version numbers are defined in BCD (in a 65535.65535 format) and are meant to be displayed as 2 digits if the minor ones digit is 0. Sub-minor revisions are included in the minor number. See the "EFI_TABLE_HEADER" section in any UEFI spec.
        // The spec also states that minor versions are limited to a max of 99, even though they get to have a whole 16-bit number.
    }
    else
    {
        Print(L"\r\n");
    }
#endif

    Print(L"   Firmware Vendor: %s\r\n   Firmware Revision: 0x%08x\r\n\n", ST->FirmwareVendor, ST->FirmwareRevision);

    // Configuration table info
    Print(L"%llu system configuration tables are available.\r\n", ST->NumberOfTableEntries);
   
    getc();

    // Search for ACPI tables
    UINT8 RSDPfound = 0;
    UINTN RSDP_index = 0;

    // This print is for debugging
    for (UINTN i = 0; i < ST->NumberOfTableEntries; i++)
    {
        Print(L"Table %llu GUID: %08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x\r\n", i,
            ST->ConfigurationTable[i].VendorGuid.Data1,
            ST->ConfigurationTable[i].VendorGuid.Data2,
            ST->ConfigurationTable[i].VendorGuid.Data3,
            ST->ConfigurationTable[i].VendorGuid.Data4[0],
            ST->ConfigurationTable[i].VendorGuid.Data4[1],
            ST->ConfigurationTable[i].VendorGuid.Data4[2],
            ST->ConfigurationTable[i].VendorGuid.Data4[3],
            ST->ConfigurationTable[i].VendorGuid.Data4[4],
            ST->ConfigurationTable[i].VendorGuid.Data4[5],
            ST->ConfigurationTable[i].VendorGuid.Data4[6],
            ST->ConfigurationTable[i].VendorGuid.Data4[7]);

        if (compare(&ST->ConfigurationTable[i].VendorGuid, &(EFI_GUID)EFI_ACPI_20_TABLE_GUID, 16))
        {
            Print(L"RSDP 2.0 found!\r\n");
            RSDP_index = i;
            RSDPfound = 2;
        }
    }
    // If no RSDP 2.0, check for 1.0
    if (!RSDPfound)
    {
        for (UINTN i = 0; i < ST->NumberOfTableEntries; i++)
        {
            if (compare(&ST->ConfigurationTable[i].VendorGuid, &(EFI_GUID)ACPI_TABLE_GUID, 16))
            {
                Print(L"RSDP 1.0 found!\r\n");
                RSDP_index = i;
                RSDPfound = 1;
            }
        }
    }

    if (!RSDPfound)
    {
        Print(L"System has no RSDP.\r\n");
    }

    getc(); 

    // View memmap before too much happens to it
    print_memmap();
    
    Print(L"\nDone printing memmap\n");
    getc();

    // Create graphics structure
    //GPU_CONFIG* Graphics;
    //Status = ST->BootServices->AllocatePool(EfiLoaderData, sizeof(GPU_CONFIG), (void**)&Graphics);
    //if (EFI_ERROR(Status))
    //{
    //    Print(L"Graphics AllocatePool error. 0x%llx\r\n", Status);
    //    return Status;
    //}

    getc();

    return EFI_SUCCESS;
}