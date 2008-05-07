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
 */
#include "LiftDictionaryFile.h"

bool 
LiftDictionaryFile::GetWordsFromFileSetup() const
{
    return false;
}

bool 
LiftDictionaryFile::FileHasAnotherWord() const
{
    return false;
}

std::basic_string<gunichar> 
LiftDictionaryFile::GetNextWordFromFile() const
{
    return false;
}

bool 
LiftDictionaryFile::GetWordsFromFileTeardown() const
{
    return false;
}
