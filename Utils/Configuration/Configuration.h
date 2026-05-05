
#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <iostream>
#include <sstream>

#define XML_PATH "xmlDir"
#define EN_ART_PATH "en_art"
#define CN_ART_PATH "cn_art"
#define EN_DICT_PATH "en_dict"
#define CN_DICT_PATH "cn_dict"
#define EN_INDEX_PATH "en_index"
#define CN_INDEX_PATH "cn_index"

#define RIPEPAGE_PATH "ripepageLib"
#define OFFSET_PATH "offsetLib"
#define NEW_RIPEPAGE_PATH "newRipepageLib"
#define NEW_OFFSET_PATH "newOffsetLib"
#define INDEX_PATH "invertIndex"

#define DICT_PATH "dict"
#define HMM_PATH "hmm_model"
#define IDF_PATH "idf"
#define STOP_WORD_PATH "stop_words"
#define USER_DICT_PATH "user_dict"

#define IP "ip"
#define PORT "port"
#define THREAD_NUM "threadNum"
#define QUE_SIZE "queSize"

#define CONFIG_INIT(config_path) Configuration::getInstance(config_path)
#define CONFIG Configuration::getInstance()->getConfigMap()

class Configuration
{
public:
    static Configuration *getInstance();
    static Configuration *getInstance(const std::string &filepath);
    std::unordered_map<std::string, std::string> &getConfigMap();
    std::unordered_set<std::string> &getStopWords();

private:
    Configuration(const std::string &filepath);
    ~Configuration();
    static void destroy();

private:
    std::unordered_map<std::string, std::string> _configs;
    std::unordered_set<std::string> _stopWords;
    static Configuration *_pInstance;
};

#endif //_CONFIGURATION_H