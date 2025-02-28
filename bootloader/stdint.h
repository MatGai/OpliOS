#pragma once

//
// standard types and defines
//

#define CONST const
#define CONSTEXPR constexpr
#define VOLATILE volatile
#define EXTERN extern

typedef int LONG;
typedef unsigned int ULONG;
typedef LONG* PLONG;
typedef ULONG* PULONG;

typedef char CHAR;
typedef CHAR* PCHAR;
typedef CONST CHAR* LPCSTR;

//typedef wchar_t WCHAR;
//typedef WCHAR* PWCHAR;
//typedef CONST PWCHAR LPCWSTR;
//
typedef unsigned char BYTE;
typedef BYTE* PBYTE;

typedef long long LONG64;
typedef LONG64* PLONG64;

typedef unsigned long long ULONG64;
typedef ULONG64* PULONG64;

typedef ULONG ULONG32;
typedef ULONG32* PULONG32;

typedef unsigned short WORD;
typedef WORD* PWORD;

#define PVOID void*
//#define NULL 0

#define _In_
#define _In_opt_
#define _Inout_
#define _Inout_opt_
#define _Out_
#define _Out_opt_

#define offsetof(type, member) ((unsigned long long) &(((type*)0)->member))