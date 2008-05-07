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
#pragma once
#include "DictionaryFileBase.h"

class LineDictionaryFile : public DictionaryFileBase
{
public:
    LineDictionaryFile(const std::string & sDictionaryFilePath):DictionaryFileBase(sDictionaryFilePath)
    {
        Init();
    }

    LineDictionaryFile(const std::basic_string<gunichar2> & sDictionaryFilePath):DictionaryFileBase(sDictionaryFilePath)
    {
        Init();
    }

private:
    void Init()
    {
        handle_ = NULL;
        cbBuffer_ = 2048;
        rgbBuffer_ = new char[cbBuffer_];
    }

    ~LineDictionaryFile()
    {
        if(rgbBuffer_ != NULL)
        {
            delete rgbBuffer_;
        }
    }

    virtual bool IsReadOnly() const
    {
        // todo: need to verify that the file itself isn't read-only
        return false; 
    }

protected:
    virtual bool SaveWordsToFileSetup();
    virtual bool SaveWordToFile(std::basic_string<gunichar> s);
    virtual bool SaveWordsToFileTeardown();

    virtual bool GetWordsFromFileSetup();
    virtual bool FileHasAnotherWord();
    virtual std::basic_string<gunichar> GetNextWordFromFile();
    virtual bool GetWordsFromFileTeardown();

private:
  void Load();
  void Save();
  
  void SaveAsUTF8(const std::basic_string<gunichar>& s);
  void SaveAsUTF16(const std::basic_string<gunichar>& s);
  
  void LoadUTF8File();
  void LoadUTF16File();

  std::basic_string<gunichar> GetLineUtf8();
  std::basic_string<gunichar> GetLineUtf16();

  static std::basic_string<gunichar> TrimWhitespace(const std::basic_string<gunichar>& s);


  enum ByteOrder{
    LittleEndian,
    BigEndian
  } byteOrder_;

  FILE*         handle_;
  bool          fIsUtf8_;
  char*         rgbBuffer_;
  size_t        cbBuffer_;
  std::basic_string<gunichar>  nextWord_;
};