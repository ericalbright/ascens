#include <UnitTest++.h>
#include "../Ascens/Ascens.h"

#include <Windows.h>
#include <vector>
#include <algorithm>

struct DictionaryTestFixture{
  //Setup
  DictionaryTestFixture(){
    wchar_t szTempPath[MAX_PATH];
    GetTempPath(MAX_PATH, szTempPath);
    GetTempFileName(szTempPath, NULL, 0, szDictionaryFileName);

    hDictionary = LoadDictionary(szDictionaryFileName);
  }

  //Teardown
  ~DictionaryTestFixture(){
    UnloadDictionary(hDictionary);
    DeleteFile(szDictionaryFileName);
  }

  void AddWordsToDictionary(const std::vector<const std::wstring>& sWords){
    for(std::vector<const std::wstring>::const_iterator itWord = sWords.begin();
      itWord != sWords.end();
      ++itWord){
      AddWordToDictionary(hDictionary, itWord->c_str(), itWord->size());
    }
  }

  DHANDLE hDictionary;
  wchar_t szDictionaryFileName[MAX_PATH];
};


//// LoadDictionary
TEST(LoadDictionary_NullFileName_NullDictionaryHandle){
  DHANDLE hDictionary = LoadDictionary(NULL);
  CHECK_EQUAL((DHANDLE)NULL, hDictionary); 
}

//// UnloadDictionary
TEST(UnloadDictionary_NullHandle_NoCrash){
  UnloadDictionary(NULL);
}

//// IsWordInDictionary
TEST_FIXTURE(DictionaryTestFixture, IsWordInDictionary_NotExists_False){
  std::wstring s(L"I'mnotinadictionary");
  CHECK(!IsWordInDictionary(hDictionary, s.c_str(), s.size()));
}

TEST_FIXTURE(DictionaryTestFixture, IsWordInDictionary_Exists_True){
  std::wstring s(L"dictionary");
  AddWordToDictionary(hDictionary, s.c_str(), s.size());
  CHECK(IsWordInDictionary(hDictionary, s.c_str(), s.size()));
  RemoveWordFromDictionary(hDictionary, s.c_str(), s.size());
}

//// AddWordToDictionary
TEST_FIXTURE(DictionaryTestFixture, AddWordToDictionary_AlreadyExists_Successful){
  std::wstring s(L"dictionary");
  AddWordToDictionary(hDictionary, s.c_str(), s.size());
  CHECK(IsWordInDictionary(hDictionary, s.c_str(), s.size()));
  CHECK(AddWordToDictionary(hDictionary, s.c_str(), s.size()));
  CHECK(IsWordInDictionary(hDictionary, s.c_str(), s.size()));
}

TEST_FIXTURE(DictionaryTestFixture, AddWordToDictionary_NotAlreadyExists_Successful){
  std::wstring s(L"dictionary");
  CHECK(!IsWordInDictionary(hDictionary, s.c_str(), s.size()));
  CHECK(AddWordToDictionary(hDictionary, s.c_str(), s.size()));
  CHECK(IsWordInDictionary(hDictionary, s.c_str(), s.size()));
}

TEST_FIXTURE(DictionaryTestFixture, AddWordToDictionary_NullWord_NotSuccessful){
  CHECK(!AddWordToDictionary(hDictionary, NULL, 1));
}

TEST_FIXTURE(DictionaryTestFixture, AddWordToDictionary_0SizeWord_NotSuccessful){
  CHECK(!AddWordToDictionary(hDictionary, L"word", 0));
}

TEST_FIXTURE(DictionaryTestFixture, AddWordToDictionary_Empty_NotSuccessful){
  CHECK(!AddWordToDictionary(hDictionary, L"", 0));
}

//// RemoveWordFromDictionary
TEST_FIXTURE(DictionaryTestFixture, RemoveWordFromDictionary_Exists_Successful){
  std::wstring s(L"dictionary");
  AddWordToDictionary(hDictionary, s.c_str(), s.size());
  CHECK(RemoveWordFromDictionary(hDictionary, s.c_str(), s.size()));
  CHECK(!IsWordInDictionary(hDictionary, s.c_str(), s.size()));
}

TEST_FIXTURE(DictionaryTestFixture, RemoveWordFromDictionary_NotExists_Successful){
  std::wstring s(L"dictionary");
  CHECK(!IsWordInDictionary(hDictionary, s.c_str(), s.size()));
  CHECK(RemoveWordFromDictionary(hDictionary, s.c_str(), s.size()));
  CHECK(!IsWordInDictionary(hDictionary, s.c_str(), s.size()));
}

TEST_FIXTURE(DictionaryTestFixture, RemoveWordFromDictionary_NullWord_NotSuccessful){
  CHECK(!RemoveWordFromDictionary(hDictionary, NULL, 1));
}

TEST_FIXTURE(DictionaryTestFixture, RemoveWordFromDictionary_0SizeWord_NotSuccessful){
  CHECK(!RemoveWordFromDictionary(hDictionary, L"word", 0));
}

TEST_FIXTURE(DictionaryTestFixture, RemoveWordFromDictionary_Empty_NotSuccessful){
  CHECK(!RemoveWordFromDictionary(hDictionary, L"", 0));
}

//// GetSuggestionsFromWord
TEST_FIXTURE(DictionaryTestFixture, GetSuggestionsFromWord_WordInDictionary_Successful){
  std::wstring s(L"dictionary");
  AddWordToDictionary(hDictionary, s.c_str(), s.size());

  const size_t cchBuffer = 4096;
  wchar_t szBuffer[cchBuffer];
  CHECK(GetSuggestionsFromWord(hDictionary, s.c_str(), s.size(), szBuffer, cchBuffer));

  std::vector<std::wstring> sSuggestions;
  for(size_t i=0; szBuffer[i] != L'\0'; ++i){
    std::wstring sSuggestion(szBuffer);
    sSuggestions.push_back(sSuggestion);
    i+= sSuggestion.size();
  }

  CHECK_EQUAL(1, sSuggestions.size());
  CHECK(s == sSuggestions[0]);
}

TEST_FIXTURE(DictionaryTestFixture, GetSuggestionsFromWord_MultipleSuggestions_Successful){
  std::vector<const std::wstring> sNoiseWords;
  sNoiseWords.push_back(std::wstring(L"spat"));
  sNoiseWords.push_back(std::wstring(L"tots"));
  sNoiseWords.push_back(std::wstring(L"tater"));
  sNoiseWords.push_back(std::wstring(L"ton"));
  sNoiseWords.push_back(std::wstring(L"gnat"));

  std::vector<const std::wstring> sWords;
  sWords.push_back(std::wstring(L"cat"));
  sWords.push_back(std::wstring(L"hat"));
  sWords.push_back(std::wstring(L"that"));
  sWords.push_back(std::wstring(L"bat"));
  sWords.push_back(std::wstring(L"tot"));

  AddWordsToDictionary(sWords);
  AddWordsToDictionary(sNoiseWords);

  std::wstring s(L"tat");
  const size_t cchBuffer = 4096;
  wchar_t szBuffer[cchBuffer];
  CHECK(GetSuggestionsFromWord(hDictionary, s.c_str(), s.size(), szBuffer, cchBuffer));

  std::vector<const std::wstring> sSuggestions;
  for(size_t i=0; szBuffer[i] != L'\0'; ++i){
    std::wstring sSuggestion(&szBuffer[i]);
    sSuggestions.push_back(sSuggestion);
    i+= sSuggestion.size();
  }

  CHECK_EQUAL(sWords.size(), sSuggestions.size());

  for(std::vector<const std::wstring>::const_iterator itWord = sWords.begin(); itWord != sWords.end(); ++itWord){
    CHECK(std::find(sSuggestions.begin(), sSuggestions.end(), *itWord) != sSuggestions.end());
  }
}

TEST_FIXTURE(DictionaryTestFixture, GetSuggestionsFromWord_NullWord_NotSuccessful){
  const size_t cchBuffer = 4096;
  wchar_t szBuffer[cchBuffer];
  CHECK(!GetSuggestionsFromWord(hDictionary, NULL, 4, szBuffer, cchBuffer));
}

TEST_FIXTURE(DictionaryTestFixture, GetSuggestionsFromWord_EmptyWord_NotSuccessful){
  const size_t cchBuffer = 4096;
  wchar_t szBuffer[cchBuffer];
  CHECK(!GetSuggestionsFromWord(hDictionary, L"", 0, szBuffer, cchBuffer));
}

TEST_FIXTURE(DictionaryTestFixture, GetSuggestionsFromWord_0SizeWord_NotSuccessful){
  const size_t cchBuffer = 4096;
  wchar_t szBuffer[cchBuffer];
  CHECK(!GetSuggestionsFromWord(hDictionary, L"word", 0, szBuffer, cchBuffer));
}

TEST_FIXTURE(DictionaryTestFixture, GetSuggestionsFromWord_NullBuffer_NotSuccessful){
  CHECK(!GetSuggestionsFromWord(hDictionary, L"word", 4, NULL, 4096));
}

TEST_FIXTURE(DictionaryTestFixture, GetSuggestionsFromWord_0LengthBuffer_NotSuccessful){
  wchar_t szBuffer[4096];
  CHECK(!GetSuggestionsFromWord(hDictionary, L"word", 4, szBuffer, 0));
}

TEST_FIXTURE(DictionaryTestFixture, GetSuggestionsFromWord_SuggestionsCannotFitInBuffer_SuccessfulButTrimmed){
  std::vector<const std::wstring> sWords;
  sWords.push_back(std::wstring(L"cat"));
  sWords.push_back(std::wstring(L"hat"));
  sWords.push_back(std::wstring(L"that"));
  sWords.push_back(std::wstring(L"bat"));
  sWords.push_back(std::wstring(L"tot"));

  AddWordsToDictionary(sWords);

  std::wstring s(L"tat");
  const size_t cchBuffer = 17;
  wchar_t szBuffer[cchBuffer];
  CHECK(GetSuggestionsFromWord(hDictionary, s.c_str(), s.size(), szBuffer, cchBuffer));

  std::vector<const std::wstring> sSuggestions;
  for(size_t i=0; szBuffer[i] != L'\0'; ++i){
    std::wstring sSuggestion(&szBuffer[i]);
    sSuggestions.push_back(sSuggestion);
    i+= sSuggestion.size();
  }
  
  sWords.erase(std::find(sWords.begin(), sWords.end(), L"that")); // too long to fit

  CHECK_EQUAL(sWords.size(), sSuggestions.size());

  for(std::vector<const std::wstring>::const_iterator itWord = sWords.begin(); itWord != sWords.end(); ++itWord){
    CHECK(std::find(sSuggestions.begin(), sSuggestions.end(), *itWord) != sSuggestions.end());
  }
}

TEST_FIXTURE(DictionaryTestFixture, GetSuggestionsFromWord_NoSuggestionsWithinEditDistance_NoSuggestions){
  std::vector<const std::wstring> sWords;
  sWords.push_back(std::wstring(L"cat"));
  sWords.push_back(std::wstring(L"hat"));
  sWords.push_back(std::wstring(L"that"));
  sWords.push_back(std::wstring(L"tot"));

  AddWordsToDictionary(sWords);

  std::wstring s(L"bad");
  const size_t cchBuffer = 4096;
  wchar_t szBuffer[cchBuffer];
  CHECK(GetSuggestionsFromWord(hDictionary, s.c_str(), s.size(), szBuffer, cchBuffer, 1, 0));

  std::vector<const std::wstring> sSuggestions;
  for(size_t i=0; szBuffer[i] != L'\0'; ++i){
    std::wstring sSuggestion(&szBuffer[i]);
    sSuggestions.push_back(sSuggestion);
    i+= sSuggestion.size();
  }
  
  CHECK_EQUAL(0, sSuggestions.size());
}
