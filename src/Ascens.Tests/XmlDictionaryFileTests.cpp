#include <UnitTest++.h>
#include "XmlDictionaryFile.h"
#include <fstream>
#include <io.h>
#include <algorithm>
#include "Convert.h"

struct XmlDictionaryTestFixture{
  //Setup
  XmlDictionaryTestFixture()
  {
      gchar * szDictionaryFileName;
      gint handle = g_file_open_tmp(NULL, &szDictionaryFileName, NULL);
      sDictionaryFileName_ = std::string(szDictionaryFileName);
      g_free(szDictionaryFileName);
      _close(handle);

    sWords_.push_back(std::wstring(L"cat"));
    sWords_.push_back(std::wstring(L"hat"));
    sWords_.push_back(std::wstring(L"that"));
    sWords_.push_back(std::wstring(L"bat"));
    sWords_.push_back(std::wstring(L"tot"));
    WriteDictionary();

    pXmlDictionaryFile_ = new XmlDictionaryFile(sDictionaryFileName_,
                                "//entry/lexical-unit");
  }

  //Teardown
  ~XmlDictionaryTestFixture()
  {
      g_remove(sDictionaryFileName_.c_str());

      if(pXmlDictionaryFile_ != NULL)
      {
          delete pXmlDictionaryFile_;
      }
  }

  void WriteDictionary()
  {
//    Sleep(2000); // for FAT systems have a 2 second resolution
               // NTFS is appreciably faster but no specs on what it is exactly
               // this seems to work
    std::wofstream ofs;

    wchar_t* utf16 = (wchar_t*)g_utf8_to_utf16(sDictionaryFileName_.c_str(), 
            static_cast<glong>(sDictionaryFileName_.length()), NULL, NULL, NULL);

    ofs.open(utf16, std::ios_base::trunc | std::ios_base::out);
    g_free(utf16);
    ofs << "<?xml version='1.0'?>" <<std::endl;
    ofs << "<dictionary>" << std::endl;

    for(std::vector<const std::wstring>::const_iterator itWord = sWords_.begin();
      itWord != sWords_.end();
      ++itWord)
    {
        std::wstring s(*itWord);

        ofs << "<entry>" << std::endl;
        ofs << "<lexical-unit>" << s << "</lexical-unit>" << std::endl;
        ofs << "<definition>Insert a definition here</definition>" << std::endl;
        std::reverse(s.begin(), s.end());
        ofs << "<reversal>" << s << "</reversal>" << std::endl;
        ofs << "</entry>" << std::endl;
    }

    ofs << "</dictionary>" << std::endl;
    ofs.close();
  }

  XmlDictionaryFile* pXmlDictionaryFile_;
  std::string sDictionaryFileName_;
  SpellDictionary::wordlist_type rgWordList_;
  std::vector<const std::wstring> sWords_;

};

TEST_FIXTURE(XmlDictionaryTestFixture, HasFileChanged_Initially_True)
{
  CHECK(pXmlDictionaryFile_->HasFileChanged());
}

TEST_FIXTURE(XmlDictionaryTestFixture, HasFileChanged_AfterGetWordsFromFile_False)
{
  pXmlDictionaryFile_->GetWordsFromFile(rgWordList_);
  CHECK(!pXmlDictionaryFile_->HasFileChanged());
}

TEST_FIXTURE(XmlDictionaryTestFixture, IsReadOnly_True)
{
  CHECK(pXmlDictionaryFile_->IsReadOnly());
}

TEST_FIXTURE(XmlDictionaryTestFixture, SaveWordsToFile_NotSuccessful)
{
  CHECK(!pXmlDictionaryFile_->SaveWordsToFile(rgWordList_));

}

TEST_FIXTURE(XmlDictionaryTestFixture, GetWordsFromFile_Successful)
{
  CHECK(pXmlDictionaryFile_->GetWordsFromFile(rgWordList_));
}

TEST_FIXTURE(XmlDictionaryTestFixture, GetWordsFromFile_RetrievedAllWords)
{
  pXmlDictionaryFile_->GetWordsFromFile(rgWordList_);
  CHECK_EQUAL(sWords_.size(), rgWordList_.size());
  for(std::vector<const std::wstring>::iterator it = sWords_.begin();
      it != sWords_.end();
      ++it)
  {
      CHECK(rgWordList_.find(Convert::ToUcs4(*it)) != rgWordList_.end());
  }
}
