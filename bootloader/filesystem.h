#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include "boot.h"
#include "util.h"

//
//
// Wrapper functions for interacting with EFI file systems such as getting a file, writing a file,
// setting working directories and more.
//
//

static EFI_GUID __FileSystemProtoclGUID__ = SIMPLE_FILE_SYSTEM_PROTOCOL;

static LPCSTR             CurrentDirectoryString;
static EFI_FILE_PROTOCOL* CurrentDirectory;
static EFI_HANDLE*        CurrentFileSystemHandle;
static EFI_STATUS         FILE_SYSTEM_STATUS;

static EFI_LOADED_IMAGE_PROTOCOL* LoadedImage;

typedef enum _FILE_SYSTEM 
{
    FS0 = 0,
    FS1 = 1,
    FS2 = 2,
    FS3 = 3
} FILE_SYSTEM;

/**
* Initialises some global variables used by filesystem.
* 
* @return TRUE on success, FALSE on error. Get error through BlGetLastFileError() if needed.
*/
BOOLEAN
BLAPI
BlInitFileSystem(
    VOID
);

/**
* Basically gets root or '\\' of the current file system, usually fs0.
* 
* @param Directory, A pointer to the directory. Optional, set to NULL if you don't need it.
* 
* @return TRUE on success, FALSE on error. Get error through BlGetLastFileError() if needed.
*/
BOOLEAN
BLAPI
BlGetRootDirectory(
    _Out_opt_ EFI_FILE_PROTOCOL** Directory
);

/**
* Gets root directory of chosen file system... "fs0", "fs1" etc...
*
* @param Index     A file system index, fs0, fs1.
* @param Directory A pointer to the EFI_FILE_PROTOCOL* directory. Optional, set to NULL if you don't need it.
* 
* @return TRUE on success, FALSE on error. Get error through BlGetLastFileError() if needed.
*/
BOOLEAN
BLAPI
BlGetRootDirectoryByIndex(
    _In_ FILE_SYSTEM Index,
    _Out_opt_ EFI_FILE_PROTOCOL** Directory
);

/**
* Sets a new working directory.
* 
* @param Directory A string of the directory etc... "\\efi\\boot".
*/
BOOLEAN
BLAPI
BlSetWorkingDirectory(
    _In_ LPCSTR Directory
);

/**
* Recursively list files from a given directory.
*
* @param Directory An open EFI_FILE_PROTOCOL* for the directory to list.
* @param Depth     Recursion depth for indentation or other tracking.
*
* @return TRUE on success, FALSE on error. Get error through BlGetLastFileError() if needed.
*/
BOOLEAN
BLAPI
BlListDirectoryRecursive(
    _In_ EFI_FILE_PROTOCOL* Directory,
    _In_ ULONG64 Depth
);

/**
* List all files and directories in this file system.
* 
* @return TRUE on success, FALSE on error. Get error through BlGetLastFileError() if needed.
*/
BOOLEAN
BLAPI
BlListAllFiles(
    VOID
);

/**
* Goes into a directory and sets out directory to opened directory.
* 
* @param BaseDirectory
* @param Path          
* @param OutDirectory  A pointer to the EFI_FILE_PROTOCOL directory. Optional, set to NULL if you don't need it.
* 
* @return TRUE on success, FALSE on error. Get error through BlGetLastFileError() if needed.
*/
BOOLEAN
BLAPI
BlOpenSubDirectory(
    _In_  EFI_FILE_PROTOCOL* BaseDirectory,
    _In_  CHAR16* Path,
    _Out_ EFI_FILE_PROTOCOL** OutDirectory
);


/**
* Finds a specfic file within this file system
*
* @param Path, A string for the file name to look for.
* @param Out, A pointer to the file found.
* 
* @return TRUE on success, FALSE on error. Get error through BlGetLastFileError() if needed.
*/
BOOLEAN
BLAPI
BlFindFile(
    _In_ LPCSTR File,
    _Out_ EFI_FILE_PROTOCOL** Out
);

/**
* Gets the last set error for filesystem.
* 
* @return The error/sucess of the file system status.
*/
EFI_STATUS
BLAPI
BlGetLastFileError(
    VOID
);

/**
* Gets the name of the specified file. !!!MAKE SURE TO FREE STRING BUFFER AFTER USE!!!
* 
* @param FileProtocol A  EFI_FILE_PROTOCOL pointer to the file.
* @param Out          A string buffer for the file name.
* 
* @return TRUE on success, FALSE on error. Get error through BlGetLastFileError() if needed.
*/
BOOLEAN
BlGetFileName(
    _In_ EFI_FILE_PROTOCOL* FileProtocol,
    _Out_ CHAR16** Out
);

//  ------------------------------ //
//       NOT IMPLEMENTED YET       //
//  ------------------------------ //

/**
* Finds the directory of a specific file.
* 
* @param File, A string of the file to look for.
*/
VOID
BLAPI
BlFindFileDirectory(
    _In_ LPCSTR File
);

#endif // !_FILESYSTEM_H
