#include "filesystem.h"

BOOLEAN 
BLAPI
BlInitFileSystem(
    VOID
)
{
    EFI_GUID LoadedImageProtocolGUID = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    FILE_SYSTEM_STATUS = gBS->HandleProtocol( gImageHandle, &LoadedImageProtocolGUID, (VOID**)&LoadedImage );

    if( EFI_ERROR( FILE_SYSTEM_STATUS ) )
    {
        Print(L"[ %r ] - Failed to get loaded image protocol in BlInitFileSystem", FILE_SYSTEM_STATUS);
        return FALSE;
    }

    return TRUE;
}

BOOLEAN
BLAPI
BlGetRootDirectory(
    _Out_opt_ EFI_FILE_PROTOCOL** Directory
)
{
    if (!LoadedImage)
    {
        Print(L"[ %r ] - Loaded image was null, maybe failed to get it?", BlGetLastFileError( ) );
        return FALSE;
    }

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileProtocol;
    FILE_SYSTEM_STATUS = gBS->HandleProtocol(LoadedImage->DeviceHandle, &__FileSystemProtoclGUID__, (VOID**)&FileProtocol);

    if ( EFI_ERROR( FILE_SYSTEM_STATUS ) )
    {
        Print(L"[ %r ] - Could not get file protocol in BlGetRootDirectory", FILE_SYSTEM_STATUS);
        return FALSE;
    }

    EFI_FILE_PROTOCOL* Root;
    FILE_SYSTEM_STATUS = FileProtocol->OpenVolume(FileProtocol, &Root );

    if( EFI_ERROR( FILE_SYSTEM_STATUS )  )
    {
        Print(L"[ %r ] - Could not open the root directory in BlGetRootDirectory", FILE_SYSTEM_STATUS);
        return FALSE;
    }

    CurrentDirectory = Root;

    if (Directory)
    {
        *Directory = Root;
    }

    return TRUE;
}

BOOLEAN
BLAPI
BlGetRootDirectoryByIndex(
    _In_ FILE_SYSTEM Index,
    _Out_opt_ EFI_FILE_PROTOCOL** Directory
)
{
    EFI_HANDLE* FileSystemHandles;
    ULONG64 HandleCount = 0;

    // Get all handles to all file systems on this system
    FILE_SYSTEM_STATUS = gBS->LocateHandleBuffer(ByProtocol, &__FileSystemProtoclGUID__, NULL, &HandleCount, &FileSystemHandles);

    if( EFI_ERROR( FILE_SYSTEM_STATUS ) || !HandleCount )
    {
        return FALSE;
    }

    // well this just shouldn't happen!
    if( (ULONG64)Index > HandleCount )
    {
        FreePool( FileSystemHandles );
        return FALSE;
    }

    // open the file system by handle index
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileProtocol;
    FILE_SYSTEM_STATUS = gBS->OpenProtocol( FileSystemHandles[ Index ], &__FileSystemProtoclGUID__, (VOID**)&FileProtocol, gImageHandle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL );
    
    if( EFI_ERROR( FILE_SYSTEM_STATUS ) || !FileProtocol )
    {
        FreePool( FileSystemHandles );
        return FALSE;
    }

    // now actually get the directory!
    EFI_FILE_PROTOCOL* Root;
    FILE_SYSTEM_STATUS = FileProtocol->OpenVolume(FileProtocol, &Root);

    if (EFI_ERROR(FILE_SYSTEM_STATUS))
    {
        return FALSE;
    }

    CurrentFileSystemHandle = FileSystemHandles[ Index ];
    CurrentDirectory = Root;

    if ( Directory )
    {
        *Directory = Root;
    }

    return TRUE;
}

EFI_STATUS
BLAPI
BlOpenSubDirectory(
    _In_  EFI_FILE_PROTOCOL* BaseDir,
    _In_  CHAR16* Path,
    _Out_ EFI_FILE_PROTOCOL** OutDir
)
{
    if (!BaseDir || !Path || !OutDir) 
    {
        return EFI_INVALID_PARAMETER;
    }

    EFI_FILE_PROTOCOL* NewDir = NULL;

    // passing 0 as attributes returns if it is directory or not
    EFI_STATUS Status = BaseDir->Open(
        BaseDir,
        &NewDir,
        Path,
        EFI_FILE_MODE_READ,
        0
    );

    if (Status != EFI_FILE_DIRECTORY)
    {
        Print(L"[ %r ] - Passed in path '%s' is not a directory to open!!!!", Status, Path );
        return EFI_INVALID_PARAMETER;
    }

    Status = BaseDir->Open(
        BaseDir,
        &NewDir,
        Path,
        EFI_FILE_MODE_READ,
        EFI_FILE_DIRECTORY
    );

    if (!EFI_ERROR(Status)) 
    {
        *OutDir = NewDir;
    }
    return Status;
}

BOOLEAN
BLAPI
BlFindFile(
    _In_ LPCSTR File,
    _Out_ EFI_FILE_PROTOCOL** OutFile
)
{
    if (File == NULL)
        return;

    // Make sure we have a CurrentDirectory
    if (CurrentDirectory == NULL) 
    {
        // default to fs0: root
        CurrentDirectory = BlGetRootDirectory( NULL );
    }

    // if this happens then...well...
    if (CurrentDirectory == NULL)
    {
        return FALSE;
    }

    EFI_FILE_PROTOCOL* OpenedFile = NULL;
    FILE_SYSTEM_STATUS = CurrentDirectory->Open(
        CurrentDirectory,
        &OpenedFile,
        File,
        EFI_FILE_MODE_READ,
        0
    );

    if (EFI_ERROR(FILE_SYSTEM_STATUS))
    {
        Print(L"[ %r ] - Failed to open file in BlFindFile", FILE_SYSTEM_STATUS);
        return FALSE;
    }

    *OutFile = OpenedFile;
    return TRUE;
}

BOOLEAN
BLAPI
BlListAllFiles(
    VOID
)
{
    if (!CurrentDirectory) 
    {
        FILE_SYSTEM_STATUS = EFI_INVALID_PARAMETER;
        return FALSE;
    }

    // Reset the directory position to the beginning
    CurrentDirectory->SetPosition(CurrentDirectory, 0);

    UINTN BufferSize = 1024; // 4kb should be enough right?
    EFI_FILE_INFO* FileInfo = AllocateZeroPool(BufferSize);
    if (!FileInfo) 
    {
        FILE_SYSTEM_STATUS = EFI_OUT_OF_RESOURCES;
        return FALSE;
    }

    Print(L"\nDirectory listing:\n");

    while ( TRUE ) 
    {
        // every read resets 'Size' to the size of our buffer.
        ULONG64 Size = BufferSize;
        FILE_SYSTEM_STATUS = CurrentDirectory->Read(CurrentDirectory, &Size, FileInfo);
        if ( EFI_ERROR( FILE_SYSTEM_STATUS ) )
        {
            // Some error reading the file
            Print(L"[ %r ] - Failed to read file\n", FILE_SYSTEM_STATUS);
            FreePool(FileInfo);
            return FALSE;
        }

        if (Size == 0) 
        {
            // Reached the end of the directory
            break;
        }

        // FileInfo->Attribute defines EFI_FILE_DIRECTORY
        if (FileInfo->Attribute & EFI_FILE_DIRECTORY) 
        {
            Print(L"  <DIR>  %s\n", FileInfo->FileName);
        }
        else 
        {
            Print(L"  %d  %s\n", FileInfo->FileSize, FileInfo->FileName);
        }
    }

    FreePool(FileInfo);
    return TRUE;
}

EFI_STATUS 
BLAPI
BlGetLastFileError(
    VOID
)
{
    return FILE_SYSTEM_STATUS;
}
 
BOOLEAN
BLAPI
BlSetWorkingDirectory(
    _In_ LPCSTR Directory
)
{
    if (Directory == NULL || Directory[0] == '\0')
        return;

    // Convert ASCII to Unicode
    CHAR16 Path[256];
    UINTN i = 0;
    for (; Directory[i] != '\0' && i < 255; i++) 
    {
        Path[i] = (CHAR16)Directory[i];
    }
    Path[i] = L'\0';

    // if no set directory set it!
    if ( CurrentDirectory == NULL ) 
    {
        if( !BlGetRootDirectory( NULL ) )
        {
            if (EFI_ERROR(FILE_SYSTEM_STATUS))
            {
                Print( L"[% r] - Failed to get root directory of fs0", FILE_SYSTEM_STATUS );
                return FALSE;
            }
        }
    }

    if ( CurrentDirectory ) 
    {
        EFI_FILE_PROTOCOL* NewDir = NULL;
        EFI_STATUS Status = BlOpenSubDirectory(CurrentDirectory, Path, &NewDir);
        if (!EFI_ERROR(Status) && NewDir != NULL)
        {
            // Success
            CurrentDirectory = NewDir;
            CurrentDirectoryString = Directory;  // Store pointer or make a copy
        }
        else
        {
            FILE_SYSTEM_STATUS = Status;
            return FALSE;
        }
    }

    return TRUE;
}

EFI_STATUS
PrintFileName(
    EFI_FILE_PROTOCOL* FileProtocol
)
{

    if (FileProtocol == NULL) 
    {
        return EFI_INVALID_PARAMETER;
    }

    UINTN          InfoSize = 512;
    EFI_FILE_INFO* FileInfo = AllocateZeroPool(InfoSize);
    if (FileInfo == NULL) 
    {
        return EFI_OUT_OF_RESOURCES;
    }

    EFI_STATUS Status = FileProtocol->GetInfo(
        FileProtocol,
        &gEfiFileInfoGuid,
        &InfoSize,
        FileInfo
    );

    if (EFI_ERROR(Status)) 
    {
        FreePool(FileInfo);
        return Status;
    }

    Print(L"File Name: %s\n", FileInfo->FileName);

    FreePool(FileInfo);
    return EFI_SUCCESS;
}