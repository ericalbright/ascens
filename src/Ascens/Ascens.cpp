/* Spell Checking Engine
 * Copyright (c) 1997-2000 Eric S. Albright
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
 * This spell checker operates by comparing a given word against a dictionary.
 * It uses approximate string matching algorithms to provide hints.
 * The dictionary is an implementation of a trie.
 * When serializing the dictionary's trie, the tree structure is separated from the content.
 *
 * Design goals:
 *   Language independent (configurable to work across any language of the world)
 *   Able to create dictionaries from word lists
 *   "Slave" DLL to be called by "task master"
 *   Use the Standard Template Library (STL) where possible
 *
 * Much of the theoretical and practical implementation for the approximate string
 * matching components comes from the article:
 *
 * H. Shang and T.H. Merrettal, "Tries for Approximate String Matching", IEEE
 * Transactions on Knowledge and Data Engineering, Vol. 8, No. 4, August 1996.
 *
 */

#pragma warning(disable: 4786) //identifier was truncated to '255' characters in the browser information

#include "SpellDictionary.h"
#define ASCENSDLL
#include "Ascens.h"
#include "LineDictionaryFile.h"

/////////////////
// LoadDictionary
// returns NULL if DictionaryFileSpec is invalid or another error occurs
DllExport DHANDLE ASCENSAPI LoadDictionary(LPCWSTR szDictionaryFileSpec)
{ 
  if (szDictionaryFileSpec == NULL) { 
    return NULL; 
  }
  SpellDictionary * pDictionary = new SpellDictionary();
  try{
    pDictionary->Load(new LineDictionaryFile(
        reinterpret_cast<const gunichar2*>(szDictionaryFileSpec)));
  }
  catch(...)
  {
      delete pDictionary;
      pDictionary = NULL;
  }

  return pDictionary;
}

///////////////////
// UnloadDictionary
DllExport void ASCENSAPI UnloadDictionary(DHANDLE hDictionary) 
{
  SpellDictionary * pDictionary = (SpellDictionary*)hDictionary;

  if(pDictionary != NULL) {
    delete pDictionary;
  }
}

// Word checking

/////////////////////
// IsWordInDictionary
DllExport BOOL ASCENSAPI IsWordInDictionary(DHANDLE hDictionary, LPCWSTR strWord, size_t cchWord) 
{ 
  SpellDictionary* pDictionary = (SpellDictionary*) hDictionary;
  if (strWord == NULL || cchWord == 0 || pDictionary == NULL) { 
    return false; 
  }
  
  std::basic_string<gunichar2> sWord(reinterpret_cast<const gunichar2*>(strWord), cchWord);
  try{
    return pDictionary->IsWordPresentUtf16(sWord);
  }
  catch(...){
  }
  return false;
}

////////////////
// GetSuggestionsFromWord
DllExport BOOL ASCENSAPI GetSuggestionsFromWord(DHANDLE hDictionary, LPCWSTR strWord, size_t cchWord, 
        LPWSTR szBuffer, size_t cchBuffer, USHORT nErrorTolerance, USHORT nBestErrorTolerance) 
{
  if (strWord == NULL || cchWord == 0) { return false; }
  if (szBuffer == NULL || cchBuffer < 2) { return false; }
  if (hDictionary == NULL) { return false; }
  
  SpellDictionary* pDictionary = (SpellDictionary*) hDictionary;

  std::basic_string<gunichar2> sWord(reinterpret_cast<const gunichar2*>(strWord), cchWord);
  pDictionary->SetSuggestionErrorTolerance(nErrorTolerance);
  pDictionary->SetSuggestionBestErrorTolerance(nBestErrorTolerance);
  try{
      std::vector<const std::basic_string<gunichar2> > rgstrSuggestions = 
        pDictionary->GetSuggestionsFromWordUtf16(sWord);

    size_t pos = 0;
    for(std::vector<const std::basic_string<gunichar2> >::iterator 
                            itrgstrSuggestions=rgstrSuggestions.begin(); 
        itrgstrSuggestions!=rgstrSuggestions.end(); 
        ++itrgstrSuggestions) { 
      if(cchBuffer - pos - 1 < itrgstrSuggestions->size()+1){
        continue;
      }

#pragma warning(suppress:4996)
      wcsncpy(&szBuffer[pos], reinterpret_cast<LPCWSTR>(itrgstrSuggestions->c_str()), itrgstrSuggestions->size());
      pos += itrgstrSuggestions->size();
      szBuffer[pos] = L'\0';
      ++pos;
    }

    szBuffer[pos] = L'\0';
    if(pos==0){
      szBuffer[1] = L'\0';
    }
  }
  catch(...){
    return false;
  }

  
  return true;
}

//////////////////////
// AddWordToDictionary
DllExport BOOL ASCENSAPI AddWordToDictionary(DHANDLE hDictionary, LPCWSTR strWord, size_t cchWord)
{
  if (strWord == NULL || cchWord == 0) { return false; }
  if (hDictionary == NULL) { return false; }
  
  SpellDictionary* pDictionary = (SpellDictionary*) hDictionary;

  std::basic_string<gunichar2> sWord(reinterpret_cast<const gunichar2*>(strWord), 
                                     cchWord);

  try {
    pDictionary->AddWordUtf16(sWord);
    return true;
  }
  catch (...){
  }
  return false;
}

///////////////////////////
// RemoveWordFromDictionary
DllExport BOOL ASCENSAPI RemoveWordFromDictionary(DHANDLE hDictionary, LPCWSTR strWord, size_t cchWord)
{
  if (strWord == NULL || cchWord == 0) { return false; }
  if (hDictionary == NULL) { return false; }
  
  SpellDictionary* pDictionary = (SpellDictionary*) hDictionary;
  std::basic_string<gunichar2> sWord(reinterpret_cast<const gunichar2*>(strWord), 
                                     cchWord);
  try{
    pDictionary->RemoveWordUtf16(sWord);
    return true;
  }
  catch(...){
  }
  return false;
}

