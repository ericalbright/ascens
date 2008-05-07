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
#include <glib.h>

class IDictionaryFile; // don't include IDictionaryFile.h since IDictionaryFile.h includes SpellDictionary.h for wordlist_type

#pragma warning(disable: 4786) //identifier was truncated to '255' characters in the browser information

class SpellDictionary
{
public:
  typedef trie::trie_set<std::basic_string<gunichar>>     wordlist_type;
  typedef wordlist_type::iterator                         wordlist_iterator;

public:
  SpellDictionary();
  ~SpellDictionary();

//     Load                     -- associates the dictionary file --
// the dictionaryFile's lifetime is then managed by this class
void Load (IDictionaryFile * const pDictionaryFile);

//     IsWordPresent            -- determines if the word exists in the dictionary --
bool IsWordPresent(const std::basic_string<gunichar>& strWord);
bool IsWordPresentUtf8(const std::string& strWord);
bool IsWordPresentUtf16(const std::basic_string<gunichar2>& strWord);
  
//     GetSuggestionsFromWord   -- gives a list of words which a similar by N distinctions --
std::vector<std::basic_string<gunichar> > GetSuggestionsFromWord(const std::basic_string<gunichar>& strWord);
std::vector<std::string> GetSuggestionsFromWordUtf8(const std::string& strWord);
std::vector<std::basic_string<gunichar2> > GetSuggestionsFromWordUtf16(const std::basic_string<gunichar2>& strWord);

//     AddWord                  -- adds the word to the dictionary if it does not exist --
  void AddWord(const std::basic_string<gunichar>& strWord);
  void AddWordUtf8(const std::string& strWord);
  void AddWordUtf16(const std::basic_string<gunichar2>& strWord);

//     RemoveWord               -- removes the word from the dictionary if it exists --
  void RemoveWord(const std::basic_string<gunichar>& strWord);
  void RemoveWordUtf8(const std::string& strWord);
  void RemoveWordUtf16(const std::basic_string<gunichar2>& strWord);
  
  void RemoveAllWords();

  size_t GetEntryCount();

  unsigned short GetSuggestionErrorTolerance() const
  {
      return nErrorTolerance_;
  }
  void SetSuggestionErrorTolerance(unsigned short value){
      nErrorTolerance_ = value;
  }

  unsigned short GetSuggestionBestErrorTolerance() const
  {
      return nBestErrorTolerance_;
  }
  void SetSuggestionBestErrorTolerance(unsigned short value){
      nBestErrorTolerance_ = value;
  }

private:
    void ReleaseDictionaryFile();
    void Load();
    void Save();

  wordlist_type rgWordList_;
  IDictionaryFile* pDictionaryFile_;
  unsigned short nErrorTolerance_;
  unsigned short nBestErrorTolerance_;
};