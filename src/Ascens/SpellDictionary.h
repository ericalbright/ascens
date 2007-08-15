/* Dictionary.h
 * Copyright (c) 1997 Eric S. Albright
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
 * This implementation uses the Standard Template Library (STL)
 *
 *
 * Much of the theoretical and practical implementation for the approximate string
 * matching components comes from the article:
 *
 * H. Shang and T.H. Merrettal, "Tries for Approximate String Matching", IEEE
 * Transactions on Knowledge and Data Engineering, Vol. 8, No. 4, August 1996.
 */

#pragma once

#include <string>
#include <vector>
#include "trie_set"
#include <Windows.h>

extern "C" {
#include "CVTUTF.H"
}

#pragma warning(disable: 4786) //identifier was truncated to '255' characters in the browser information

class SpellDictionary
{
  typedef trie::trie_set<std::wstring>     wordlist_type;
  typedef wordlist_type::iterator          wordlist_iterator;

public:
  SpellDictionary();

//     Load                     -- associates the dictionary file --
void Load (const std::wstring& strDictionaryFilePath);

//     IsWordPresent            -- determines if the word exists in the dictionary --
bool IsWordPresent(const std::wstring& strWord);
  
//     GetSuggestionsFromWord   -- gives a list of words which a similar by N distinctions --
std::vector<std::wstring> GetSuggestionsFromWord(const std::wstring& strWord, USHORT nErrorTolerance = 2, USHORT nBestErrorTolerance = 6);

//     AddWord                  -- adds the word to the dictionary if it does not exist --
  void AddWord(const std::wstring& strWord);

//     RemoveWord               -- removes the word from the dictionary if it exists --
  void RemoveWord(const std::wstring& strWord);
  
  void RemoveAllWords();

  size_t GetEntryCount();

private:
  void Load();
  void Save();
  
  void SaveAsUTF8(HANDLE hFile) const;
  void SaveAsUTF16(HANDLE hFile) const;
  
  void GetLastWriteTime(FILETIME * ftLastWriteTime) const;
  void LoadUTF8File(HANDLE hFile);

  enum ByteOrder{
    LittleEndian,
    BigEndian
  };

  void LoadUTF16File(HANDLE hFile, ByteOrder fIsByteReversed);

private:
  wordlist_type rgWordList_;
  std::wstring  strDictionaryFilePath_;
  bool          fIsUtf8_;
  FILETIME      ftLastKnownWrite_;
};