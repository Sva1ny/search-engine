#ifndef _WORDSEGMENTATION_H_
#define _WORDSEGMENTATION_H_

#include <cppjieba/Jieba.hpp>
#include "Configuration.h"
#include <iostream>
#include <string>
#include <vector>
using std::cout;
using std::endl;
using std::string;
using std::vector;

class WordSegmentation // 使用结巴分词库进行分词
{
public:
    WordSegmentation()
        : _jieba(CONFIG[DICT_PATH], CONFIG[HMM_PATH], CONFIG[USER_DICT_PATH], CONFIG[IDF_PATH], CONFIG[STOP_WORD_PATH]) // 初始化Jieba类对象
    {
        cout << "cppjieba init!" << endl;
    }

    vector<string> cut(const string str) // 返回str的分字结果
    {
        vector<string> words;
        _jieba.Cut(str, words, true);
        // ; // Cut With HMM
        return words;
    }

private:
    cppjieba::Jieba _jieba;
};
#endif
