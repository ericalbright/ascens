#include "SpellDictionary.h"
#include "IDictionaryFile.h"
#include "Convert.h"
#include "Normalize.h"

SpellDictionary::SpellDictionary(void)
: nErrorTolerance_(2), nBestErrorTolerance_(6)
{
    pDictionaryFile_ = NULL;
}

SpellDictionary::~SpellDictionary(void)
{
    ReleaseDictionaryFile();
}

void 
SpellDictionary::ReleaseDictionaryFile()
{
    if(pDictionaryFile_ != NULL)
    {
        delete pDictionaryFile_;
    };
    pDictionaryFile_ = NULL;
}

void 
SpellDictionary::Load (IDictionaryFile*const pDictionaryFile)
{
    ReleaseDictionaryFile();
    pDictionaryFile_ = pDictionaryFile;
    Load();
}

bool 
SpellDictionary::IsWordPresent(const std::basic_string<gunichar>& word) 
{
  Load();
  std::basic_string<gunichar> strWord = Normalize::ToNFD(word);
  return rgWordList_.find(strWord) != rgWordList_.end(); 
}

bool 
SpellDictionary::IsWordPresentUtf8(const std::string& strWord) 
{
    return IsWordPresent(Convert::ToUcs4(strWord));
}

bool 
SpellDictionary::IsWordPresentUtf16(const std::basic_string<gunichar2>& strWord) 
{
    return IsWordPresent(Convert::ToUcs4(strWord));
}

std::vector<const std::basic_string<gunichar>> 
SpellDictionary::GetSuggestionsFromWord(const std::basic_string<gunichar>& word) 
{
  std::vector<const std::basic_string<gunichar>>        rgstrWords;
  std::vector<wordlist_type::iterator>            rgIt;
  std::vector<wordlist_type::iterator>::iterator  itrgIt;
  std::basic_string<gunichar> strWord = Normalize::ToNFD(word);
  Load();
  
  // break word and check if two newly created words are both words.
  // If so, add this to suggestions
  std::basic_string<gunichar>::const_iterator it = strWord.begin();
  for(++it; it != strWord.end(); ++it) {
    if( (rgWordList_.find(strWord.begin(), it) != rgWordList_.end()) &&
      (rgWordList_.find(it, strWord.end()) != rgWordList_.end())) {
      rgstrWords.push_back(strWord);
      (*rgstrWords.rbegin()).insert((it-strWord.begin()), 1, L' ');
    }
  }

  rgIt = rgWordList_.approximate_find(strWord, nErrorTolerance_);
  if(rgIt.empty()) {
    // we are limiting best_find here since it would be theoretically possible
    // to get the entire dictionary
    rgIt = rgWordList_.best_find(strWord, nBestErrorTolerance_);
  }

  for(itrgIt = rgIt.begin(); itrgIt != rgIt.end(); itrgIt++) {
    rgstrWords.push_back(**itrgIt);
  }

  return rgstrWords;
}
std::vector<const std::string> 
SpellDictionary::GetSuggestionsFromWordUtf8(const std::string& strWord) 
{
    std::vector<const std::basic_string<gunichar>> suggestions;
    suggestions = GetSuggestionsFromWord(Convert::ToUcs4(strWord));

    std::vector<const std::string>        result;

    for(std::vector<const std::basic_string<gunichar>>::iterator it = suggestions.begin();
        it != suggestions.end(); 
        ++it) 
    {
        result.push_back(Convert::ToUtf8(*it));
    }

    return result;
}

std::vector<const std::basic_string<gunichar2>> 
SpellDictionary::GetSuggestionsFromWordUtf16(const std::basic_string<gunichar2>& strWord) 
{
    std::vector<const std::basic_string<gunichar>> suggestions;
    suggestions = GetSuggestionsFromWord(Convert::ToUcs4(strWord));

    std::vector<const std::basic_string<gunichar2>> result;

    for(std::vector<const std::basic_string<gunichar>>::iterator it = suggestions.begin();
        it != suggestions.end(); 
        ++it) 
    {
        result.push_back(Convert::ToUtf16(*it));
    }

    return result;
}

void 
SpellDictionary::AddWord(const std::basic_string<gunichar>& strWord) 
{
  Load();
  // only bother saving if an insertion actually occured.
  if(rgWordList_.find(strWord) != rgWordList_.end()){
    return;
  }

  rgWordList_.insert(strWord);
  Save();
}

void 
SpellDictionary::AddWordUtf8(const std::string& strWord) 
{
    AddWord(Convert::ToUcs4(strWord));
}

void 
SpellDictionary::AddWordUtf16(const std::basic_string<gunichar2>& strWord) 
{
    AddWord(Convert::ToUcs4(strWord));
}

void 
SpellDictionary::RemoveWord(const std::basic_string<gunichar>& word) 
{
  Load();
  std::basic_string<gunichar> strWord = Normalize::ToNFD(word);

  // only bother saving if a removal actually occured.
  if (rgWordList_.erase(strWord) != 0){
    Save();
  }
}

void 
SpellDictionary::RemoveWordUtf8(const std::string& strWord) 
{
    RemoveWord(Convert::ToUcs4(strWord));
}

void 
SpellDictionary::RemoveWordUtf16(const std::basic_string<gunichar2>& strWord) 
{
    RemoveWord(Convert::ToUcs4(strWord));
}

void 
SpellDictionary::RemoveAllWords() 
{
  Load();
  // only bother saving if a removal actually occured.
  if(!rgWordList_.empty()){
    rgWordList_.clear();
    Save();
  }
}


size_t 
SpellDictionary::GetEntryCount() 
{
  Load();
  return rgWordList_.size();
}

void 
SpellDictionary::Load()  
{
    if(!pDictionaryFile_->HasFileChanged())
    {
        return;
    }

    rgWordList_.clear();

    pDictionaryFile_->GetWordsFromFile(rgWordList_);
}

void 
SpellDictionary::Save() 
{
    if(pDictionaryFile_->IsReadOnly()) {
        return;
    }

    assert(!pDictionaryFile_->HasFileChanged());

    pDictionaryFile_->SaveWordsToFile(rgWordList_);
}
