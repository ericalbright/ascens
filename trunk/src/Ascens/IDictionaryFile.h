/* Spell Checking Engine
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
 * The IDictionaryFile interface presents an abstraction that allows
 * the general SpellDictionary class to use different file formats
 */
#pragma once
#include "SpellDictionary.h"

// the interface assumes that your constructor will take
// a file (file path) as a parameter
class IDictionaryFile
{
public:
    // returns true if the file has changed since the last time
    // that GetWordsFromFile was called and so should be rescanned
    virtual bool HasFileChanged() const = 0;

    // return true if file is read only or format is read only
    virtual bool IsReadOnly() const = 0;

    // returns true if was successfully able to save words to file
    virtual bool SaveWordsToFile(const SpellDictionary::wordlist_type& dictionary) = 0;

    // returns true if was successfully able to get words from file
    virtual bool GetWordsFromFile(SpellDictionary::wordlist_type & dictionary) = 0;
};