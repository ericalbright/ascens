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


class Normalize
{
public:

    inline
    static std::basic_string<gunichar> 
    ToNFD(const std::basic_string<gunichar>& s)
    {
        std::basic_string<gunichar> result;

        for(size_t i = 0; i != s.length(); ++i)
        {
            gsize len;
            gunichar* decomposition = g_unicode_canonical_decomposition(s[i], &len);
            result.append(decomposition, len);
            g_free(decomposition);
        }

        
        gunichar* decomposed = new gunichar[result.size() + 1];
        memcpy(decomposed, result.c_str(), (result.size() + 1) * sizeof(gunichar));
        g_unicode_canonical_ordering(decomposed, static_cast<gsize>(result.size()));
        result = std::basic_string<gunichar>(decomposed);

        delete[] decomposed;
        return result;
    }


};