#include <wchar.h>
#include <stdlib.h>
#include <string.h>

/* Minimal compatibility shim for wcsrtombs when the C runtime lacks it.
   This implementation uses wcstombs to perform the conversion. It is
   sufficient for miniaudio.h usage in this project. */

size_t wcsrtombs(char *dst, const wchar_t **src, size_t len, mbstate_t *ps) {
    (void)ps; /* unused in this shim */
    if (src == NULL || *src == NULL) return 0;

    /* If dst is NULL, return required length (like wcsrtombs semantics) */
    if (dst == NULL) {
        /* wcstombs with NULL may return required length or (size_t)-1 on failure */
        return wcstombs(NULL, *src, 0);
    }

    size_t res = wcstombs(dst, *src, len);
    if (res == (size_t)-1) {
        return (size_t)-1;
    }

    /* Advance src to end of string converted */
    *src = *src + wcslen(*src);
    return res;
}

#if defined(_WIN32) || defined(__MINGW32__)
/* Some MinGW/MSVCRT builds reference an import-thunk symbol named __imp_wcsrtombs.
   Provide a forwarding function to satisfy the linker. */
__declspec(dllexport) size_t __cdecl __imp_wcsrtombs(char *dst, const wchar_t **src, size_t len, mbstate_t *ps) {
    return wcsrtombs(dst, src, len, ps);
}
#endif
