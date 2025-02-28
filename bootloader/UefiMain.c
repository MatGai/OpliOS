#pragma once
#include "filesystem.h"

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

    getc();

    if ( !BlInitFileSystem( ) )
    {
        return 1;
    }

    if( BlGetRootDirectory( NULL ) )
    { 
        BlListAllFiles();
    }
    else
    {
        if( EFI_ERROR( FILE_SYSTEM_STATUS ) )
        {
            Print( L"[ %r ] Failed to get root directory of current FS\n", BlGetLastFileError() );
        }
    }

    EFI_FILE_PROTOCOL* File = NULL;
    if( BlFindFile( L"kernal.exe", &File ) )
    {
        Print(L"Found kernal.exe file!\n");
        PrintFileName(File);
    }
    
    getc();

    return EFI_SUCCESS;
}