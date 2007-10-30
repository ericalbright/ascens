#include "SpellDictionary.h"

SpellDictionary::SpellDictionary(void)
{
}

void SpellDictionary::Load (const std::wstring& strDictionaryFilePath){
  ftLastKnownWrite_.dwHighDateTime = 0;
  ftLastKnownWrite_.dwLowDateTime = 0; // will force a load. 
  strDictionaryFilePath_ = strDictionaryFilePath;
  Load();
}


bool SpellDictionary::IsWordPresent(const std::wstring& strWord) {
  Load();
  return rgWordList_.find(strWord) != rgWordList_.end(); 
}
  
std::vector<std::wstring> SpellDictionary::GetSuggestionsFromWord(const std::wstring& strWord, USHORT nErrorTolerance, USHORT nBestErrorTolerance) {
  std::vector<std::wstring>                       rgstrWords;
  std::vector<wordlist_type::iterator>            rgIt;
  std::vector<wordlist_type::iterator>::iterator  itrgIt;
  
  Load();
  
  // break word and check if two newly created words are both words.
  // If so, add this to suggestions
  std::wstring::const_iterator it = (std::wstring::const_iterator) strWord.begin();
  for(++it; it != strWord.end(); ++it) {
    if( (rgWordList_.find(strWord.begin(), it) != rgWordList_.end()) &&
      (rgWordList_.find(it, strWord.end()) != rgWordList_.end())) {
      rgstrWords.push_back(strWord);
      (*rgstrWords.rbegin()).insert((it-strWord.begin()), 1, L' ');
    }
  }

  rgIt = rgWordList_.approximate_find(strWord, nErrorTolerance);
  if(rgIt.empty()) {
    // we are limiting best_find here since it would be theoretically possible
    // to get the entire dictionary
    rgIt = rgWordList_.best_find(strWord, nBestErrorTolerance);
  }

  for(itrgIt = rgIt.begin(); itrgIt != rgIt.end(); itrgIt++) {
    rgstrWords.push_back(**itrgIt);
  }

  return rgstrWords;
}
      
void SpellDictionary::AddWord(const std::wstring& strWord) {
  Load();
  // only bother saving if an insertion actually occured.
  if(rgWordList_.find(strWord) != rgWordList_.end()){
    return;
  }

  rgWordList_.insert(strWord);
  Save();
}

void SpellDictionary::RemoveWord(const std::wstring& strWord) {
  Load();
  
  // only bother saving if a removal actually occured.
  if (rgWordList_.erase(strWord) != 0){
    Save();
  }
}
  
void SpellDictionary::RemoveAllWords() {
  Load();
  // only bother saving if a removal actually occured.
  if(!rgWordList_.empty()){
    rgWordList_.clear();
    Save();
  }
}


size_t SpellDictionary::GetEntryCount() {
  Load();
  return rgWordList_.size();
}

void SpellDictionary::Load()  {
  std::wstring   strWord;
   
  HANDLE hFile = CreateFileW(strDictionaryFilePath_.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  if(hFile == INVALID_HANDLE_VALUE) {
    throw HRESULT_FROM_WIN32(GetLastError());
  }

  try {

    FILETIME ftLastWriteTime;
    if(!GetFileTime(hFile, NULL, NULL, &ftLastWriteTime)) {
      throw HRESULT_FROM_WIN32(GetLastError());
    }
    
    //has file been modified?
    // returns 0 when first time is equal to second time;
    if(!CompareFileTime(&ftLastKnownWrite_, &ftLastWriteTime)) {
      //if has not been changed since loaded, no need to load again
      CloseHandle(hFile);
      return;
    }

    // on FAT volumes, write time has a resolution of 2 seconds so it is possible that
    // changes could sneak in.
    ftLastKnownWrite_ = ftLastWriteTime;
//      fReadonly_ = FileInformation.dwFileAttributes & FILE_ATTRIBUTE_READONLY;
    
    // peak at the first two bytes to see which encoding we are using.
    wchar_t wcEncoding;
    DWORD cBytes;
    if(ReadFile(hFile, &wcEncoding, sizeof(wcEncoding), &cBytes, NULL)==0) {
      throw HRESULT_FROM_WIN32(GetLastError());
    }
    fIsUtf8_ = false;
    switch(wcEncoding) {
      case 0xFEFF: // Byte Order Mark (UTF-16)
        LoadUTF16File(hFile, LittleEndian);
        break;
      case 0xFFFE: // byte-reversed Byte Order Mark
        LoadUTF16File(hFile, BigEndian);
        break;
      default: // assume UTF-8
        // put the peeked bytes back
        if(SetFilePointer(hFile, -((long)cBytes), NULL, FILE_CURRENT)==-1){
          throw HRESULT_FROM_WIN32(GetLastError());
        }
        fIsUtf8_ = true;
        LoadUTF8File(hFile);
    }
  }
  catch(...){
    rgWordList_.clear();
    CloseHandle(hFile);
    throw;
  }

  if(CloseHandle(hFile) == 0) {
    throw HRESULT_FROM_WIN32(GetLastError());
  }
}

void SpellDictionary::Save() {
  HANDLE hFile = CreateFileW(strDictionaryFilePath_.c_str(), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  if(hFile == INVALID_HANDLE_VALUE) {
    throw HRESULT_FROM_WIN32(GetLastError());
  }

  try{
    if(fIsUtf8_) {
      SaveAsUTF8(hFile);
    }
    else {
      SaveAsUTF16(hFile);
    }
  }
  catch(...){
    CloseHandle(hFile);
    throw;
  }

  if(SetEndOfFile(hFile) == 0) {
    throw HRESULT_FROM_WIN32(GetLastError());
  }

  if(CloseHandle(hFile) == 0) {
    throw HRESULT_FROM_WIN32(GetLastError());
  }

  // update the last known write time
  ftLastKnownWrite_ = GetLastWriteTime();
}

void SpellDictionary::SaveAsUTF16(HANDLE hFile) const {
  wchar_t wcBom = 0xFEFF;
  wchar_t nl[] = {'\r', '\n'};
  DWORD cBytes;
  if(WriteFile(hFile, &wcBom, sizeof(wcBom), &cBytes, NULL)==0) {
    throw HRESULT_FROM_WIN32(GetLastError());
  }

  wordlist_iterator itWordList = rgWordList_.begin();
  wordlist_iterator itEndWordList = rgWordList_.end();
  while (itWordList != itEndWordList) {
    if(WriteFile(hFile, itWordList->c_str(), itWordList->length()*2, &cBytes, NULL)==0) {
      throw HRESULT_FROM_WIN32(GetLastError());
    }
    
    if(WriteFile(hFile, &nl, sizeof(nl), &cBytes, NULL)==0) {
      throw HRESULT_FROM_WIN32(GetLastError());
    }
    itWordList++;
  }
}

void SpellDictionary::SaveAsUTF8(HANDLE hFile) const {
  char nl[] = {'\r', '\n'};
  DWORD cBytes;
  unsigned char buffer[1024];

  wordlist_iterator itWordList = rgWordList_.begin();
  wordlist_iterator itEndWordList = rgWordList_.end();

  ConversionResult r;
  const wchar_t * wsz;
  const wchar_t * wszEnd;
  unsigned char * sz;
  unsigned char * szEnd;
  
  while (itWordList != itEndWordList) {
    wsz = itWordList->c_str();
    wszEnd = wsz+itWordList->length();
    szEnd = &buffer[1024];
    do {
      sz = &buffer[0];
      r = ConvertUTF16toUTF8((const UTF16**)&wsz, (const UTF16*) wszEnd, &sz, szEnd);
      // the pointer sz gets adjusted to the end of the converted span
      if(WriteFile(hFile, &buffer, sz - buffer, &cBytes, NULL)==0) {
        throw HRESULT_FROM_WIN32(GetLastError());
      }
    } while(r == targetExhausted);
    assert(r == ok);
    
    if(WriteFile(hFile, &nl, sizeof(nl), &cBytes, NULL)==0) {
      throw HRESULT_FROM_WIN32(GetLastError());
    }
    itWordList++;
  }
}

FILETIME SpellDictionary::GetLastWriteTime() const {
  FILETIME ftLastWriteTime;
  HANDLE hFile = CreateFileW(strDictionaryFilePath_.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if(hFile == INVALID_HANDLE_VALUE) {
    throw HRESULT_FROM_WIN32(GetLastError());
  }

  // Retrieve the file times for the file.
  if (!GetFileTime(hFile, NULL, NULL, &ftLastWriteTime)){
    HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
    CloseHandle(hFile);
    throw hr;
  }
  if(!CloseHandle(hFile)) {
    throw HRESULT_FROM_WIN32(GetLastError());
  }

  return ftLastWriteTime;
}


void SpellDictionary::LoadUTF8File(HANDLE hFile) {
  assert(hFile != INVALID_HANDLE_VALUE);

  rgWordList_.clear();
  
  DWORD cBytes;
  unsigned long UCS4ch = 0;
  unsigned char c;
  std::wstring str;
  std::wstring strData;
  const unsigned long BOM = 0xfeff;

  unsigned char RemainingBytes[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5};

  unsigned long offsetsFromUTF8[6] =	
        {0x00000000UL, 0x00003080UL, 0x000E2080UL, 
         0x03C82080UL, 0xFA082080UL, 0x82082080UL};

  if(ReadFile(hFile, &c, sizeof(c), &cBytes, NULL)==0) {
    throw HRESULT_FROM_WIN32(GetLastError());
  }

  while(cBytes != 0){ // end of file
    UCS4ch = 0;
	  unsigned char cbRequired = RemainingBytes[(unsigned char)c];

    for(unsigned char i = cbRequired;;--i){
      UCS4ch += c;                  // merge into the UCS4 character
      if(i == 0) {
        break;
      }
      UCS4ch <<= 6;                 // shift and get next character
      if(ReadFile(hFile, &c, sizeof(c), &cBytes, NULL)==0) {
        throw HRESULT_FROM_WIN32(GetLastError());
      }
    }
	  UCS4ch -= offsetsFromUTF8[cbRequired];

    if (UCS4ch <= 0x0000FFFFUL) { // maximum UCS2
      if(UCS4ch == L'\n' || UCS4ch == L'\r') {
        if(!str.empty()) {
          rgWordList_.insert(str);
          str.erase();
        }
      } 
      else {
       if(UCS4ch != BOM){
         str += (wchar_t)UCS4ch;
       }
      }

    } 
    else if (UCS4ch < 0x0010FFFFUL) { // maximum UTF16 (considering surrogates)
		  UCS4ch -= 0x0010000UL;
		  str += (wchar_t)((UCS4ch >> 10) + 0xD800);      // Surrogate High Start
		  str += (wchar_t)((UCS4ch & 0x03FFUL) + 0xDC00); // Surrogate Low Start;
    }
    else {
      str += 0xFFFD;
    }

    if(ReadFile(hFile, &c, sizeof(c), &cBytes, NULL)==0) {
      throw HRESULT_FROM_WIN32(GetLastError());
    }
  }

  if(!str.empty()) {
    rgWordList_.insert(str);
  }
}

void SpellDictionary::LoadUTF16File(HANDLE hFile, ByteOrder byteOrder) {
  assert(hFile != INVALID_HANDLE_VALUE);

  rgWordList_.clear();

  DWORD cBytes;
  wchar_t wc;
  std::wstring str;
  std::wstring strData;

  for(;;) {
    // get a Unicode character
    if(ReadFile(hFile, &wc, sizeof(wc), &cBytes, NULL)==0) {
      throw HRESULT_FROM_WIN32(GetLastError());
    }

    if (cBytes == 0){
      // end of file
      break;
    }

    if(byteOrder == BigEndian) {
      wc = MAKEWORD(HIBYTE(wc), LOBYTE(wc));
    }

    if(wc == L'\n' || wc == L'\r') {
      if(!str.empty()) {
        rgWordList_.insert(str);
        str.erase();
      }
    }
    else {
      str += wc;
    }
  }

  if(!str.empty()) {
    rgWordList_.insert(str);
  }
}


