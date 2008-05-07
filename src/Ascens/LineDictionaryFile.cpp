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
#include "LineDictionaryFile.h"
#include "Convert.h"
bool 
LineDictionaryFile::SaveWordsToFileSetup()
{
  char* mode;

  if(fIsUtf8_)  { mode = "wt"; }
  else          { mode = "wb"; }
  
  handle_ = g_fopen(GetDictionaryFilePath().c_str(), mode);
  if(handle_ == NULL)  { return false; }

  if(!fIsUtf8_)
  {
      const wchar_t BOM = 0xFEFF;
      if(fputwc(BOM, handle_) == WEOF)
      {
          // it wasn't able to write the BOM
          SaveWordsToFileTeardown();
          return false;
      }
  }

  return true;
}

bool 
LineDictionaryFile::SaveWordToFile(std::basic_string<gunichar> s)
{
    if(handle_ == NULL) { return false; }

    try
    {
        if(fIsUtf8_) 
        {
            SaveAsUTF8(s);
        }
        else {
            SaveAsUTF16(s);
        }
    }
    catch(...)
    {
        SaveWordsToFileTeardown();
        return false;
    }

    return true;
}

bool 
LineDictionaryFile::SaveWordsToFileTeardown()
{
    if(handle_ == NULL) { return false; }

    fclose(handle_);
    handle_ = NULL;
    return true;
}

void 
LineDictionaryFile::SaveAsUTF16(const std::basic_string<gunichar> & s) 
{
    if(handle_ == NULL) { throw "handle_ has not been set"; }

    gunichar2* utf16s = g_ucs4_to_utf16(s.c_str(), s.length(), NULL, NULL, NULL);
    if(utf16s == NULL) { 
      throw "problem converting to utf16"; 
    }

    std::basic_string<gunichar2> sUtf16(utf16s);
    g_free(utf16s);

    sUtf16 += '\n';

    if(fwrite(sUtf16.c_str(), sizeof(gunichar2), sUtf16.length(), handle_) != sUtf16.length())
    {
        throw "error writing string as utf16";
    }
}

void 
LineDictionaryFile::SaveAsUTF8(const std::basic_string<gunichar> & s) 
{
    if(handle_ == NULL) { throw "handle_ has not been set"; }
    gchar* utf8s = g_ucs4_to_utf8(s.c_str(), s.length(), NULL, NULL, NULL);
    if(utf8s == NULL) { 
        throw "problem converting to utf8"; 
    }

    std::string sUtf8(utf8s);
    g_free(utf8s);

    sUtf8 += '\n';

    if(fwrite(sUtf8.c_str(), sizeof(gchar), sUtf8.length(), handle_) != sUtf8.length())
    {
        throw "error writing string as utf8";
    }
}

bool 
LineDictionaryFile::GetWordsFromFileSetup()
{
    //needs to be open in binary mode to properly handle unicode
    handle_ = g_fopen(GetDictionaryFilePath().c_str(), "rb");
    if(handle_ == NULL)  { return false; }

    fIsUtf8_ = false;
    // peak at the first two bytes to see which encoding we are using.
    wchar_t bom = fgetwc(handle_);

    switch(bom) {
        case WEOF:
            GetWordsFromFileTeardown();
            return false;
            break;
        case 0xFEFF: // Byte Order Mark (UTF-16)
            byteOrder_ = LittleEndian;
            break;
        case 0xFFFE: // byte-reversed Byte Order Mark
            byteOrder_ = BigEndian;
            break;
        default: // assume UTF-8
            if(ungetwc(bom, handle_) == WEOF)
            {
                GetWordsFromFileTeardown();
                return false;
            }
            // see if there is a utf8 bom (EF BB BF)
            bool hasUtf8Bom = false;
            unsigned char utf8bomBuffer[3];
            if(fread(utf8bomBuffer, sizeof(char), 3, handle_) == 3)
            {
                if(utf8bomBuffer[0] == 0xef &&
                   utf8bomBuffer[1] == 0xbb &&
                   utf8bomBuffer[2] == 0xbf)
                {
                    hasUtf8Bom = true;
                }
            }

            if(!hasUtf8Bom)
            {
                //go back to beginning of file
                if(fseek(handle_, 0, SEEK_SET) != 0)
                {
                    GetWordsFromFileTeardown();
                    return false;
                }
            }

            fIsUtf8_ = true;
            break;
    }

    return true;
}

bool 
LineDictionaryFile::FileHasAnotherWord()
{
    do{
        // get a line of text (terminated by '\r', '\n' or "\r\n"
        if(fIsUtf8_)
        {
            nextWord_ = GetLineUtf8();
        }
        else
        {
            nextWord_ = GetLineUtf16();
        }
    }
    // if line is empty (and we still have more to read on the file) start over
    while(nextWord_.empty() && feof(handle_) == 0);

    if(nextWord_.empty())
    {
      return false;
    }
    return true;
}

std::basic_string<gunichar> 
LineDictionaryFile::GetNextWordFromFile()
{
    return nextWord_;
}

bool 
LineDictionaryFile::GetWordsFromFileTeardown()
{
    if(handle_ == NULL) { return false; }

    fclose(handle_);
    handle_ = NULL;
    return true;
}

std::basic_string<gunichar> 
LineDictionaryFile::TrimWhitespace(const std::basic_string<gunichar>& s)
{
    std::basic_string<gunichar> result;
    const gunichar whitespace[] = {'\n',
                                   '\r',
                                   '\t',
                                   ' '};
    std::basic_string<gunichar>::size_type indexStart, indexEnd;

    indexStart = s.find_first_not_of(whitespace);
    if(indexStart != std::basic_string<gunichar>::npos)
    {
        result = s.substr(indexStart);
        indexEnd = result.find_last_not_of(whitespace);
        if(indexEnd != std::basic_string<gunichar>::npos)
        {
            result.resize(indexEnd+1);
        }
    }
    return result;
}


std::basic_string<gunichar>
LineDictionaryFile::GetLineUtf8()
{
    std::basic_string<gunichar> result;
    char* s;
    do{
        s = fgets(rgbBuffer_, cbBuffer_/sizeof(char), handle_);
        if(s != NULL)
        {
            result = Convert::ToUcs4(s);
        }
#if defined(_DEBUG)
        std::basic_string<gunichar2> utf16 = Convert::ToUtf16(result);
#endif

        result = TrimWhitespace(result);

#if defined(_DEBUG)
        utf16 = Convert::ToUtf16(result);
#endif

    } 
    while(s != NULL && result.empty());

    return result;
}

//todo: this needs a lot more work
std::basic_string<gunichar>
LineDictionaryFile::GetLineUtf16()
{
    std::basic_string<gunichar> result;
    wchar_t* s;
    do{
        s = fgetws(reinterpret_cast<wchar_t*>(rgbBuffer_), 
                        cbBuffer_/sizeof(wchar_t), 
                        handle_);
        if(s != NULL)
        {
            result = Convert::ToUcs4(reinterpret_cast<gunichar2*>(s));
        }
#if defined(_DEBUG)
        std::basic_string<gunichar2> utf16 = Convert::ToUtf16(result);
#endif
        result = TrimWhitespace(result);

#if defined(_DEBUG)
        utf16 = Convert::ToUtf16(result);
#endif

    } 
    while(s != NULL && result.empty());

    return result;
}
