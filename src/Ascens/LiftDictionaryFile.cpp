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
#include "LiftDictionaryFile.h"

#if !defined(LIBXML_XPATH_ENABLED)
#error LibXml XPath support is required
#endif
#if !defined(LIBXML_SAX1_ENABLED)
#error LibXml SAX1 support is required
#endif

bool 
LiftDictionaryFile::GetWordsFromFileSetup()
{
    /* Init libxml */     
    xmlInitParser();
    LIBXML_TEST_VERSION

    /* Load XML document */
    doc_ = xmlParseFile(GetDictionaryFilePath().c_str());
    if (doc_ == NULL) {
	    fprintf(stderr, "Error: unable to parse file \"%s\"\n", GetDictionaryFilePath().c_str());
        GetWordsFromFileTeardown();
	    return(false);
    }

    /* Create xpath evaluation context */
    xpathCtx_ = xmlXPathNewContext(doc_);
    if(xpathCtx_ == NULL) {
        fprintf(stderr,"Error: unable to create new XPath context\n");
        GetWordsFromFileTeardown();
        return(false);
    }
    
    /* Evaluate xpath expression */
    xpathObj_ = xmlXPathEvalExpression(
                  reinterpret_cast<const xmlChar*>(sXPathThatSelectsWords_.c_str()),
                  xpathCtx_);
    if(xpathObj_ == NULL) {
        fprintf(stderr,"Error: unable to evaluate xpath expression \"%s\"\n", sXPathThatSelectsWords_.c_str());
        GetWordsFromFileTeardown();
        return(false);
    }

    index_ = 0;

    return true;
}


bool 
LiftDictionaryFile::FileHasAnotherWord() const
{
    xmlNodeSetPtr nodes = xpathObj_->nodesetval;
    int size = (nodes) ? nodes->nodeNr : 0;

    return index_ < size;
}

std::basic_string<gunichar> 
LiftDictionaryFile::GetNextWordFromFile() const
{
    std::basic_string<gunichar> word;

    xmlNodeSetPtr nodes = xpathObj_->nodesetval;

    xmlNodePtr node = nodes->nodeTab[index_];
	
    assert(node != NULL);
    if(node == NULL)
    {
        return word;
    }

    xmlChar* content = xmlNodeGetContent(node);
    if(content != NULL)
    {
        word = Convert::ToUcs4(reinterpret_cast<char*>(content));
        xmlFree(content);
    }
    return word;
}

bool 
LiftDictionaryFile::GetWordsFromFileTeardown()
{

    /* Cleanup */
    if(xpathObj_ != NULL)
    {
        xmlXPathFreeObject(xpathObj_);
        xpathObj_ = NULL;
    }

    if(xpathCtx_ != NULL)
    {
        xmlXPathFreeContext(xpathCtx_); 
        xpathCtx_ = NULL;
    }

    if(doc_ != NULL)
    {
        xmlFreeDoc(doc_); 
        doc_ = NULL;
    }

    /* Shutdown libxml */
    xmlCleanupParser();
    return true;
}
