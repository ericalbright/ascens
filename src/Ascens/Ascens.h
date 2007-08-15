#pragma once

typedef const wchar_t *      LPCWSTR;
typedef wchar_t *            LPWSTR;

typedef void*             DHANDLE;
typedef unsigned short    USHORT;
typedef int               BOOL;

#define NULL        0
#if defined(ASCENSDLL)
#define DllExport    __declspec( dllexport ) 
#else
#define DllExport
#endif

#define ASCENSAPI       __stdcall
  
// We provide a set of C operations that do not directly manipulate the dictionary
//   objects but use a handle to the dictionary object.

// This is the public interface to the DLL from the Task Master appplication
#if defined(__cplusplus)
extern "C" {
#endif

  DllExport DHANDLE ASCENSAPI LoadDictionary(LPCWSTR szDictionaryFilespec);
  DllExport void    ASCENSAPI UnloadDictionary(DHANDLE hDictionary);

  // Word checking
  DllExport BOOL  ASCENSAPI IsWordInDictionary(DHANDLE hDictionary, LPCWSTR strWord, size_t cchWord);

  DllExport BOOL  ASCENSAPI GetSuggestionsFromWord(DHANDLE hDictionary, 
                                         LPCWSTR szWord, size_t cchWord, 
                                         LPWSTR szBuffer, size_t cbBuffer, 
                                         USHORT nEditDistanceMax = 1);
  DllExport BOOL  ASCENSAPI AddWordToDictionary(DHANDLE hDictionary, LPCWSTR szWord, size_t cchWord);
  DllExport BOOL  ASCENSAPI RemoveWordFromDictionary(DHANDLE hDictionary, LPCWSTR szWord, size_t cchWord);

#if defined(__cplusplus)
}
#endif