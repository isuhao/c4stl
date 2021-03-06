// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "c4/string.hpp"
#include <cwchar>

C4_BEGIN_NAMESPACE(c4)

#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-declarations" //  warning : 'mbsrtowcs' is deprecated: This function or variable may be unsafe. Consider using sscanf_s instead
#endif

void s2ws(char const* mbstr, size_t len, wchar_t *output)
{
    std::mbstate_t state = std::mbstate_t();
    std::mbsrtowcs(&output[0], &mbstr, len, &state);
}
void ws2s(wchar_t const* mbstr, size_t len, char *output)
{
    std::mbstate_t state = std::mbstate_t();
    std::wcsrtombs(&output[0], &mbstr, len, &state);
}

#ifdef __clang__
#   pragma clang diagnostic pop
#endif

C4_END_NAMESPACE(c4)

//-----------------------------------------------------------------------------

#ifdef C4_VSNPRINTF
/// Visual Studio does not have vsnprintf
/// @see http://stackoverflow.com/questions/2915672/snprintf-and-visual-studio-2010
int vsnprintf(char* str, size_t size, const char* format, va_list ap)
{
    int count = -1;

    if (size != 0)
    {
        count = _vsnprintf_s(str, size, _TRUNCATE, format, ap);
    }

    if (count == -1)
    {
        count = _vscprintf(format, ap);
    }

    return count;
}
#endif // C4_VSNPRINTF

#ifdef C4_SNPRINTF
/// Visual Studio does not have snprintf
/// @see http://stackoverflow.com/questions/2915672/snprintf-and-visual-studio-2010
int snprintf(char* str, size_t size, const char* format, ...)
{
    int count;
    va_list ap;

    va_start(ap, format);
#if defined(C4_WIN) || defined(C4_XBOX)
    count = _vsnprintf_s(str, size, _TRUNCATE, format, ap);
#else
    count = vsnprintf(str, size, format, ap);
#endif
    va_end(ap);

    return count;
}
#endif // C4_SNPRINTF
