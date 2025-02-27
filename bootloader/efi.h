#pragma once

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PrintLib.h>
#include <Library/ShellLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>
#include <IndustryStandard/PeImage.h>
#include <Guid/GlobalVariable.h>

typedef EFI_SYSTEM_TABLE* PEFI_SYSTEM_TABLE;
typedef EFI_HANDLE* PEFI_HANDLE;
typedef EFI_FILE_IO_INTERFACE* PEFI_FILE_IO_INTERFACE;
typedef EFI_IMAGE_NT_HEADERS64 EFI_IMAGE_NT_HEADERS;

#define EFI_IMAGE_FIRST_SECTION(ntheader) \
    ( \
      (EFI_IMAGE_SECTION_HEADER *) \
        ( \
          (UINT64) ntheader + \
          offsetof (EFI_IMAGE_NT_HEADERS, OptionalHeader) + \
          ((EFI_IMAGE_NT_HEADERS *) (ntheader))->FileHeader.SizeOfOptionalHeader \
        ) \
    )