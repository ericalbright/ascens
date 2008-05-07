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
 * The DictionaryFileBase abstract type 
 */
#pragma once

#include "IDictionaryFile.h"
#include <string>
#include <glib.h>
#include "Convert.h"

class DictionaryFileBase : public IDictionaryFile
{
  std::string   _sDictionaryFilePath;
  time_t        _ftLastKnownWrite;

public:
    DictionaryFileBase(const std::string & sDictionaryFilePath)
    {
        Init(sDictionaryFilePath);
    }

    DictionaryFileBase(const std::basic_string<gunichar2> & sDictionaryFilePath)
    {
        Init(Convert::ToUtf8(sDictionaryFilePath));
    }

private:
    void Init(const std::string & sDictionaryFilePath)
    {
        _sDictionaryFilePath = sDictionaryFilePath;
        _ftLastKnownWrite = NULL; // forces a load
    }

public:

    // returns true if the file has changed since the last time
    // that GetWordsFromFile was called and so should be rescanned
    bool HasFileChanged() const
    {
        return _ftLastKnownWrite != GetLastKnownWrite();
    }

    // return true if file is read only or format is read only
    virtual bool IsReadOnly() const = 0;

    bool SaveWordsToFile(const SpellDictionary::wordlist_type & dictionary)
    {
        if(IsReadOnly())
        {
            return false;
        }

        if (!SaveWordsToFileSetup())
            return false;

        if(HasFileChanged())
        {
            // if the file has changed since the last time we read
            // it, then we can't save the file unless we re-read it first
            return false;
        }

        for(SpellDictionary::wordlist_type::const_iterator it = dictionary.begin(); 
            it != dictionary.end(); 
            ++it)
        {
            if (!SaveWordToFile(*it))
            {
                return false;
            }
        }

        if (!SaveWordsToFileTeardown())
        {
            return false;
        }

        SetLastKnownWrite();
        return true;
    }

    bool GetWordsFromFile(SpellDictionary::wordlist_type & dictionary)
    {
        SetLastKnownWrite();

        if(!GetWordsFromFileSetup())
        {
            return false;
        }

        while(FileHasAnotherWord())
        {
            dictionary.insert(GetNextWordFromFile());
        }

        if(!GetWordsFromFileTeardown())
        {
            return false;
        }

        return true;
    }

protected:
    const std::string& GetDictionaryFilePath() const
    {
        return _sDictionaryFilePath;
    }

    virtual bool SaveWordsToFileSetup() = 0;
    // if returns false, should do teardown
    virtual bool SaveWordToFile(std::basic_string<gunichar> s) = 0;
    // teardown will only be called if all calls to SaveWordToFileCore return true;
    virtual bool SaveWordsToFileTeardown() = 0;

    virtual bool GetWordsFromFileSetup() = 0;
    virtual bool FileHasAnotherWord() = 0;
    virtual std::basic_string<gunichar> GetNextWordFromFile() = 0;
    virtual bool GetWordsFromFileTeardown() = 0;

private:
    void SetLastKnownWrite()
    {
        _ftLastKnownWrite = GetLastKnownWrite();
    }

    time_t GetLastKnownWrite() const
    {
	    struct stat stats;

		if(g_stat(_sDictionaryFilePath.c_str(), &stats)!=0)
        {
            // file not found
            return NULL;
        }

        return stats.st_mtime;
    }
};