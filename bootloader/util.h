#ifndef _UTIL_H
#define _UTIL_H
#include "boot.h"

/**
* Pauses process essentially until a keyboard event has occured and returns key pressed
* 
* @return A structure containing the key that was pressed
*/
EFI_INPUT_KEY
BLAPI
getc(
    VOID
);

//realised most of these are useless, EDK2 already defines these functions...

/**
*  Gets the length of the string
* 
* @return the length of the string
*/
INTN
BLAPI
strlength(
    _In_ CONST CHAR16* str
);

#pragma function(strlen)

/**
* Copies the string from one buffer to the other
* 
* @param dst The string to move to
* @param src The string to copy from
*/
INTN
BLAPI
strcopy(
    _In_ CHAR16* dst,
    _In_ CONST CHAR16* src
);

#pragma function(strcpy)

/**
* Case sensitive string comparison
* 
* @param s0 String to compare
* @param s1 Sting to compare against
* 
* @return 0 if they are the same else they are not the same
*/
INTN
BLAPI
strcompare(
    _In_ CONST CHAR16* s0,
    _In_ CONST CHAR16* s1
);

/**
*   Outputs a formated string to the buffer
* 
* @param Out The buffer to place formatted string into
* @param Format The base string to format against
* @param ... Variadic arguments to be used with Format string
* 
* @return if the function failed or not
*/
BOOLEAN
BLAPI
strfmt(
    _Out_ CHAR16** Out,
    _In_ CONST CHAR16* Format,
    ...
);

#endif// !_UTIL_H