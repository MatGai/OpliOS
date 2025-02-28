#include "util.h"

EFI_INPUT_KEY
BLAPI
getc(
    VOID
)
{
    EFI_EVENT e[1];

    EFI_INPUT_KEY k;
    memset(&k, 0, sizeof(EFI_INPUT_KEY));

    e[0] = gST->ConIn->WaitForKey;
    UINTN index = 0;
    gBS->WaitForEvent(1, e, &index);

    if (!index)
    {
        gST->ConIn->ReadKeyStroke(gST->ConIn, &k);
    }

    return k;
}

//INTN
//BLAPI
//strlen(
//    _In_ CONST CHAR16* str
//)
//{
//    if (str == NULL)
//        return 0;
//
//    CHAR16* p = str;
//    while (*p != '\0')
//        p++;
//
//    return (INTN)(p - str);
//}
//
//INTN
//BLAPI
//strcpy(
//    _In_ CHAR16* dst,
//    _In_ CONST CHAR16* src
//)
//{
//    while (*src)
//        *(dst)++ = *(src)++;
//
//    *dst = 0;
//
//    return 0;
//}
//
//INTN 
//BLAPI
//strcmp(
//    _In_ CONST CHAR16* s0,
//    _In_ CONST CHAR16* s1
//)
//{
//    while (*s0)
//    {
//        if (*s0 != *s1)
//            break;
//
//        s0 += 1;
//        s1 += 1;
//    }
//
//    return *s0 - *s1;
//}

BOOLEAN
BLAPI
strfmt(
    _Out_ CHAR16*      Out,
    _In_ CONST CHAR16* Format,
    ...
)
{
    if (!Format) 
    {
        return FALSE;
    }

    // make sure it is null
    Out = NULL;

    VA_LIST Args;
    VA_LIST ArgsCopy;
    CHAR16* Buffer = NULL;
    UINTN   BufferSize = 256;  // random guess
    UINTN   StringLength;

    VA_START(Args, Format);
    VA_COPY(ArgsCopy, Args);

    // allocate buffer for buffer size
    Buffer = AllocateZeroPool(BufferSize * sizeof(CHAR16));
    if (Buffer == NULL)
    {
        // Out of memory
        VA_END(ArgsCopy);
        VA_END(Args);
        return FALSE;
    }

    // try make the string
    StringLength = UnicodeVSPrint(Buffer, BufferSize * sizeof(CHAR16), Format, ArgsCopy);

    // If the printed length fits (CharCount < BufSize - 1), we're good
    if (StringLength >= (BufferSize - 1))
    {
        FreePool( Buffer );
        BufferSize = StringLength + 1;

        // allocate buffer for buffer size
        Buffer = AllocateZeroPool( (BufferSize * sizeof(CHAR16)));
        if (Buffer == NULL)
        {
            // Out of memory
            VA_END(ArgsCopy);
            VA_END(Args);
            return FALSE;
        }

        VA_END(ArgsCopy);
        VA_COPY(ArgsCopy, Args);

        // if this isn't properlly made then oh well...
        UnicodeVSPrint(Buffer, BufferSize * sizeof(CHAR16), Format, ArgsCopy);
    }

    VA_END(ArgsCopy);
    VA_END(Args);

    // hopefully buffer is properly formated
    Out = Buffer;

    return TRUE;
}