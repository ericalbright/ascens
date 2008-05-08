/* Convert.h
 * Copyright (c) 2008 Eric S. Albright
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Eric S. Albright makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 *
 */
#pragma once

#include <glib.h>


class Convert
{
public:

    inline
    static std::basic_string<gunichar> 
    ToUcs4(const std::string& s)
    {
        std::basic_string<gunichar> result;

        gunichar* ucs4 = g_utf8_to_ucs4(s.c_str(), 
            static_cast<glong>(s.length()), NULL, NULL, NULL);
        result = std::basic_string<gunichar>(ucs4);
        g_free(ucs4);
        return result;
    }

    inline
    static std::basic_string<gunichar> 
    ToUcs4(const std::basic_string<gunichar2>& s)
    {
        std::basic_string<gunichar> result;

        gunichar* ucs4 = g_utf16_to_ucs4(s.c_str(), 
            static_cast<glong>(s.length()), NULL, NULL, NULL);
        result = std::basic_string<gunichar>(ucs4);
        g_free(ucs4);
        return result;
    }

    inline
    static std::basic_string<gunichar> 
    ToUcs4(const std::basic_string<wchar_t>& s)
    {
        std::basic_string<gunichar> result;

        gunichar* ucs4 = g_utf16_to_ucs4(
            reinterpret_cast<const gunichar2*>(s.c_str()), 
            static_cast<glong>(s.length()), NULL, NULL, NULL);
        result = std::basic_string<gunichar>(ucs4);
        g_free(ucs4);
        return result;
    }

    inline
    static std::string 
    ToUtf8(const std::basic_string<gunichar>& s)
    {
        std::string result;

        gchar* utf8 = g_ucs4_to_utf8(s.c_str(), 
            static_cast<glong>(s.length()), NULL, NULL, NULL);
        result = std::string(utf8);
        g_free(utf8);
        return result;
    }

    inline
    static std::string 
    ToUtf8(const std::basic_string<gunichar2>& s)
    {
        std::string result;

        gchar* utf8 = g_utf16_to_utf8(s.c_str(), 
            static_cast<glong>(s.length()), NULL, NULL, NULL);
        result = std::string(utf8);
        g_free(utf8);
        return result;
    }

    inline 
    static std::basic_string<gunichar2> 
    ToUtf16(const std::basic_string<gunichar>& s)
    {
        std::basic_string<gunichar2> result;

        gunichar2* utf16 = g_ucs4_to_utf16(s.c_str(), 
            static_cast<glong>(s.length()), NULL, NULL, NULL);
        result = std::basic_string<gunichar2>(utf16);
        g_free(utf16);
        return result;
    }
};