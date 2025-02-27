#pragma once

#include "stdint.h"

typedef LONG BL_STATUS;

#define BL_STATUS_WARNING_BASE 0xA0000000
#define BL_STATUS_ERROR_BASE 0xC0000000

#define BL_STATUS_OK ( LONG )0
#define BL_STATUS_GENERIC_ERROR ( LONG )BL_STATUS_ERROR_BASE

#define BL_SUCCESS( Status ) ( Status == BL_STATUS_OK )
#define BL_WARNING( Status ) ( ((Status) & 0xF0000000) == BL_STATUS_WARNING_BASE )
#define BL_ERROR( Status ) ( ((Status) & 0xF0000000) == BL_STATUS_ERROR_BASE )