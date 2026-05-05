#include "Configuration.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

Configuration *Configuration::_pInstance = nullptr;

Configuration *Configuration::getInstance()
{
    return _pInstance;
}

Configuration *Configuration::getInstance(const std::string &filepath)
{
    if (nullptr == _pInstance)
    {
        atexit(destroy);
        _pInstance = new Configuration(filepath);
    }
    return _pInstance;
}

void Configuration::destroy()
{
    if (_pInstance)
    {
        delete _pInstance;
    }
}

Configuration::Configuration(const std::string &filepath)
{
    std::cout << filepath << std::endl;
    std::ifstream ifsConf(filepath);
    if (!ifsConf.is_open())
    {
        std::cout << "open config file failed" << std::endl;
        return;
    }
    _configs.reserve(10);
    std::cout << "read config file success" << std::endl;
    std::string line, key, value;
    while (std::getline(ifsConf, line))
    {
        std::istringstream iss(line);
        iss >> key >> value;
        // emplace避免创建临时对象
        _configs.emplace(key, value);
    }

    ifsConf.close();
}

Configuration::~Configuration()
{
    std::cout << "~Configuration()" << std::endl;
}

std::unordered_set<std::string> &Configuration::getStopWords()
{
    if (_stopWords.size() > 0)
    {
        return _stopWords;
    }

    std::string line;
    std::ifstream ifsStopWords(CONFIG[STOP_WORD_PATH]);
    if (!ifsStopWords.is_open())
    {
        perror("open");
    }
    _stopWords.reserve(1024);
    while (std::getline(ifsStopWords, line))
    {
        istringstream iss(line);
        string word;
        iss >> word;
        _stopWords.emplace(word);
    }
    cout << "read stop words success" << endl;
    ifsStopWords.close();
    return _stopWords;
}
std::unordered_map<std::string, std::string> &Configuration::getConfigMap()
{
    return _configs;
}