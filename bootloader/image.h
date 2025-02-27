#pragma once

#include "boot.h"

BL_STATUS
BLAPI
BlLoadPEImage64(
	_In_ EFI_FILE_HANDLE ImageHandle
);