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
 */

#include "DictionaryFileBase.h"
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

class LiftDictionaryFile : DictionaryFileBase
{
    std::string sXPathThatSelectsWords_;

    xmlDocPtr doc_;
    xmlXPathContextPtr xpathCtx_; 
    xmlXPathObjectPtr xpathObj_; 
    int index_;

public:
    LiftDictionaryFile(const std::string& sDictionaryFilePath,
                       const std::string& sXPathThatSelectsWords)
        :DictionaryFileBase(sDictionaryFilePath),
        sXPathThatSelectsWords_(sXPathThatSelectsWords),
        doc_(NULL),
        xpathCtx_(NULL), 
        xpathObj_(NULL),
        index_(0)
    {
    }

    virtual bool IsReadOnly() const
    {
        return true;
    }

    // save is not supported by lift (since it is readonly)
    virtual bool SaveWordsToFileSetup() const
    {
        return false;
    }
    virtual bool SaveWordToFile(std::basic_string<gunichar> s) const
    {
        return false;
    }
    virtual bool SaveWordsToFileTeardown() const
    {
        return false;
    }

    virtual bool GetWordsFromFileSetup();
    virtual bool FileHasAnotherWord() const;
    virtual std::basic_string<gunichar> GetNextWordFromFile() const;
    virtual bool GetWordsFromFileTeardown();
};