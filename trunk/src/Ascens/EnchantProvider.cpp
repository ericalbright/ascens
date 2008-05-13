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
#include <glib.h>

#include <enchant.h>
#include <enchant-provider.h>

#include "SpellDictionary.h"
#include "XmlDictionaryFile.h"
#include "LineDictionaryFile.h"

#pragma warning(suppress: 4100) //unreferenced formal parameter
ENCHANT_PLUGIN_DECLARE("ascens")


static std::vector<const std::string> 
ascens_get_key_file_dirs (void)
{
    std::vector<const std::string> dirs;

    GSList *config_dirs = enchant_get_user_config_dirs ();
	
	for (GSList* iter = config_dirs; iter; iter = iter->next)
	{
        char * dir = g_build_filename((const gchar *)iter->data, "ascens", NULL);
        dirs.push_back(dir);
        g_free(dir);
	}

	g_slist_foreach (config_dirs, (GFunc)g_free, NULL);
	g_slist_free (config_dirs);

	
	/* Look for explicitly set registry values */
	char * ascens_data_dir = enchant_get_registry_value ("ascens", "Data_Dir");
	if (ascens_data_dir != NULL)
    {
        dirs.push_back(ascens_data_dir);
        g_free(ascens_data_dir);
    }

  	char * enchant_prefix = enchant_get_prefix_dir();
	if(enchant_prefix != NULL)
	{
		gchar* ascens_share_dir = g_build_filename(enchant_prefix, "share", "enchant", "ascens", NULL);
		g_free(enchant_prefix);
        dirs.push_back(ascens_share_dir);
        g_free(ascens_share_dir);
	}

	return dirs;
}

static gchar** ascens_stringvector_to_stringlist(const std::vector<const std::string>& stringvector)
{
    gchar** stringlist = NULL;
    if(!stringvector.empty())
    {
        stringlist = g_new0 (char *, stringvector.size() + 1);

        for(size_t i = 0; i != stringvector.size(); ++i)
        { 
            stringlist[i] = g_strdup(stringvector[i].c_str());
        }
    }
    return stringlist;
}

/*
Returns: 0 if the word is correctly spelled, positive if not, negative if error
*/
static int
ascens_dict_check (EnchantDict * me, const char *const word, size_t len)
{
    SpellDictionary* pSpellDictionary = reinterpret_cast<SpellDictionary*>(me->user_data);
    try{
        if(pSpellDictionary->IsWordPresentUtf8(std::string(word, len)))
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
    catch(...){
    }
    return -1;
}

static char **
ascens_dict_suggest (EnchantDict * me, const char *const word,
		     size_t len, size_t * out_n_suggs)
{
    std::vector<const std::string> suggestions;

    SpellDictionary* pDictionary = reinterpret_cast<SpellDictionary*>(me->user_data);

    try
    {
         suggestions = pDictionary->GetSuggestionsFromWordUtf8(std::string(word, len));
    }
    catch(...)
    {
    }

    *out_n_suggs = suggestions.size();
    return ascens_stringvector_to_stringlist(suggestions);
}

static GKeyFile*
ascens_get_settings_file(const char *const language_id, gchar** full_path)
{
    GKeyFile* key_file = g_key_file_new();
    gchar** search_dirs = ascens_stringvector_to_stringlist(ascens_get_key_file_dirs());
    *full_path = NULL;
    if(!g_key_file_load_from_dirs(key_file,
                                  language_id,
                                  const_cast<const gchar**>(search_dirs),
                                  full_path,
                                  G_KEY_FILE_NONE,
                                  NULL))
    {
        g_key_file_free(key_file);
        key_file = NULL;
    }
    g_strfreev (search_dirs);
    return key_file;
}

const char * DICTIONARY_SETTING_FILEPATH = "Path";
const char * DICTIONARY_SETTING_TYPE = "Type";
const char * DICTIONARY_SETTING_XPATH = "XPath";

static std::string
ascens_get_setting_value(GKeyFile* key_file, const gchar* key)
{
    std::string value;
    if(key_file != NULL)
    {
        gchar* s = g_key_file_get_string(key_file,
                                            "Dictionary",
                                            key,
                                            NULL);
        value = std::string(s);
        g_free(s);
    }
    return value;
}

static std::string
ascens_get_absolute_path_relative_to_settings_file(const std::string &settings_file_path,
                          const std::string &relative_path)
{
    std::string absolute_path;

    if(g_path_is_absolute(relative_path.c_str()))
    {
        return relative_path;
    }

    gchar* base_path = g_path_get_dirname(settings_file_path.c_str());
    gchar* filename = g_build_filename(base_path, relative_path.c_str(), NULL);
    g_free(base_path);

    absolute_path = std::string(filename);
    g_free(filename);

    return absolute_path;
}

static gboolean
ascens_settings_file_is_valid(GKeyFile* settings_file, const gchar* const full_path)
{
    std::string dict_path = ascens_get_setting_value(settings_file, DICTIONARY_SETTING_FILEPATH);

    if(!dict_path.empty())
    {
        dict_path = ascens_get_absolute_path_relative_to_settings_file(full_path, dict_path);
        return g_file_test (dict_path.c_str(), G_FILE_TEST_EXISTS);
    }
    return false;
}

static int
ascens_provider_dictionary_exists (EnchantProvider *,
                                   const char *const language_id)
{
    gboolean exists = false;
    gchar* full_path;
    GKeyFile* settings_file = ascens_get_settings_file(language_id, &full_path);
    exists = ascens_settings_file_is_valid(settings_file, full_path);
    g_free(full_path);
    if(settings_file != NULL)
    {
        g_key_file_free(settings_file);
    }
    return exists;
}

static EnchantDict *
ascens_provider_request_dict (EnchantProvider *, const char *const language_id)
{
	EnchantDict *dict;
    gchar* full_path;
    GKeyFile* settings_file = ascens_get_settings_file(language_id, &full_path);

    if(settings_file == NULL)
    {
        return NULL;
    }

    std::string dict_path = ascens_get_setting_value(settings_file, DICTIONARY_SETTING_FILEPATH);
    std::string dict_type = ascens_get_setting_value(settings_file, DICTIONARY_SETTING_TYPE);
    std::string xpath;
    if(dict_type == "xml")
    {
        xpath = ascens_get_setting_value(settings_file, DICTIONARY_SETTING_XPATH);
    }

    if(settings_file != NULL)
    {
        g_key_file_free(settings_file);
    }

    if(dict_path.empty() || dict_type.empty())
    {
        return NULL;
    }

    // make relative dict_path into absolute path
    dict_path = ascens_get_absolute_path_relative_to_settings_file(full_path, dict_path);

    IDictionaryFile* pDictionaryFile;
    if(dict_type == "xml")
    {
        pDictionaryFile = new XmlDictionaryFile(dict_path, xpath);
    }
    else if(dict_type == "line")
    {
        pDictionaryFile = new LineDictionaryFile(dict_path);
    }
    else
    {
        // invalid type
        return NULL;
    }

    SpellDictionary * pSpellDictionary = new SpellDictionary();
    pSpellDictionary->SetSuggestionErrorTolerance(1);
    pSpellDictionary->SetSuggestionBestErrorTolerance(4);
    pSpellDictionary->Load(pDictionaryFile);

    dict = g_new0 (EnchantDict, 1);
	dict->check = ascens_dict_check;
	dict->suggest = ascens_dict_suggest;
    dict->add_to_exclude = NULL;
    dict->add_to_session = NULL;
    dict->add_to_personal = NULL;
    dict->store_replacement = NULL;
    dict->user_data = pSpellDictionary;

	return dict;
}


static char **
ascens_provider_list_dicts (EnchantProvider *, 
			    size_t * out_n_dicts)
{
    std::vector<const std::string> dictionaries;

    std::vector<const std::string> key_file_dirs = ascens_get_key_file_dirs();
    for(std::vector<const std::string>::iterator it = key_file_dirs.begin();
        it != key_file_dirs.end();
        ++it)
    {
        GDir* dir = g_dir_open(it->c_str(), 0, NULL);
        if(dir == NULL)
        {
            continue;
        }
        while(true){
            const gchar* file = g_dir_read_name(dir);
            if(file == NULL)
            {
                break;
            }
            GKeyFile* settings_file = g_key_file_new();

            if(g_key_file_load_from_file(settings_file, file, G_KEY_FILE_NONE, NULL)
               && ascens_settings_file_is_valid(settings_file, file))
            {
                gchar* filename = g_path_get_basename(file);
                dictionaries.push_back(filename);
                g_free(filename);
            }
            g_key_file_free(settings_file);
        }
    }

    *out_n_dicts = dictionaries.size();
    return ascens_stringvector_to_stringlist(dictionaries);
}

static void
ascens_provider_dispose_dict (EnchantProvider *, EnchantDict * dict)
{
    SpellDictionary* pSpellDictionary = reinterpret_cast<SpellDictionary*>(dict->user_data);
    delete pSpellDictionary;
	g_free (dict);
}

static void
ascens_provider_free_string_list (EnchantProvider *, char **str_list)
{
	g_strfreev (str_list);
}

static void
ascens_provider_dispose(EnchantProvider * me)
{
    g_free(me);
}

static const char *
ascens_provider_identify (EnchantProvider *)
{
	return "ascens";
}

static const char *
ascens_provider_describe (EnchantProvider *)
{
	return "Ascens Provider";
}



extern "C" {

ENCHANT_MODULE_EXPORT(EnchantProvider *) 
init_enchant_provider(void)
{
    EnchantProvider *provider;
	
    provider = g_new0(EnchantProvider, 1);
    provider->dispose = ascens_provider_dispose;

    provider->identify = ascens_provider_identify;
    provider->describe = ascens_provider_describe;

    provider->dictionary_exists = ascens_provider_dictionary_exists;

    provider->request_dict = ascens_provider_request_dict;
    provider->dispose_dict = ascens_provider_dispose_dict;

    provider->list_dicts = ascens_provider_list_dicts;
    provider->free_string_list = ascens_provider_free_string_list;

    return provider;
}

}
