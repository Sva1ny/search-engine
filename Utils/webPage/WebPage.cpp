#include "WebPage.h"
#include <boost/regex.hpp>
#include <boost/locale.hpp>

WebPage::WebPage(int docid, string title, string link, string content, int size, uint64_t simhashVal)
    : _docid(docid), _title(title), _link(link), _content(content), _size(size), _simhashVal(simhashVal)
{
}

// WebPage空参构造存在的意义是WordQuery空参初始化有需要
WebPage::WebPage() {};

int WebPage::getSize() { return _size; }
int WebPage::getDocId() { return _docid; }
void WebPage::setDocId(int id) { _docid = id; }
string WebPage::getTitle() { return _title; }
string WebPage::getContent() { return _content; }
string WebPage::getUrl() { return _link; }

bool WebPage::isChinese(const std::string &str)
{
    // 中文正则表达式
    boost::wregex chinese_regex(L"^[\u4e00-\u9fa5]+$");

    // 将输入的 UTF-8 字符串转换为宽字符
    std::wstring wstr = boost::locale::conv::utf_to_utf<wchar_t>(str);

    // 使用 boost::regex_match 检查是否匹配中文或英文正则表达式
    if (boost::regex_match(wstr, chinese_regex))
    {
        return true;
    }
    return false;
}

bool WebPage::isEnglish(const std::string &str)
{
    // 英文正则表达式
    boost::wregex english_regex(L"^[A-Za-z]+$");

    // 将输入的 UTF-8 字符串转换为宽字符
    std::wstring wstr = boost::locale::conv::utf_to_utf<wchar_t>(str);

    // 使用boost::regex_match 检查是否匹配中文正则表达式
    if (boost::regex_match(wstr, english_regex))
    {
        return true;
    }
    return false;
}

std::string WebPage::remove(const std::string &input)
{
    // 定义宽字符的正则表达式，匹配除了中英文及常见标点符号以外的字符
    boost::wregex pattern(L"[^a-zA-Z0-9\u4e00-\u9fa5,.，。!！？?;；:：()（）\\[\\]【】{}“”\"'‘’、]+");

    // 将UTF-8字符串转换为宽字符串
    std::wstring wstr = boost::locale::conv::utf_to_utf<wchar_t>(input);

    // 替换匹配到的非中英文及标点符号字符为空字符串
    std::wstring result = boost::regex_replace(wstr, pattern, L"");

    // 将宽字符串转换回UTF-8字符串
    std::string output = boost::locale::conv::utf_to_utf<char>(result);

    return output;
}

std::string WebPage::summary(const std::vector<std::string> &queryWords)
{
    vector<string> summaryVec;
    string period = "。";

    istringstream iss(_content);
    string line;
    while (iss >> line)
    {
        for (auto word : queryWords)
        {
            string result;
            size_t pos = line.find(word);
            if (pos != string::npos)
            {
                if (line.length() >= 100)
                {
                    // 找到 word 之前的距离 word 最近的第一个句号
                    size_t p1 = line.rfind(period, pos);
                    size_t p2 = line.find(period, pos);

                    if (p1 != string::npos && p2 != string::npos)
                    { // word 前后都有句号
                        p1 += getBytes(line[p1]);
                        p2 -= getBytes(line[p2]);
                        result = line.substr(p1, p2 - p1);
                    }
                    else if (p1 != string::npos && p2 == string::npos)
                    { // word 之后没有句号
                        p1 += getBytes(line[p1]);
                        // 取 100 个字长
                        string temp = line.substr(p1);
                        size_t len = length(temp);
                        if (len > 100)
                        {
                            len = 100;
                        }
                        p2 = p1;
                        for (size_t ilen = 0; ilen < len; ++ilen, ++p2)
                        {
                            size_t bytes = getBytes(temp[p2]);
                            p2 += (bytes - 1);
                        }
                        result = line.substr(p1, p2 - p1);
                    }
                    else if (p1 == string::npos && p2 != string::npos)
                    { // word 之前没有句号
                        p2 -= getBytes(line[p2]);
                        result = line.substr(0, p2);
                    }
                }
                else
                {
                    result = line;
                }
                result.append("...");
                summaryVec.push_back(result);
                break;
            }
        }

        if (summaryVec.size() >= 2)
        {
            break;
        }
    }
    string summary;
    for (auto s : summaryVec)
    {
        summary.append(s).append("\n"); // 不添加换行符或其他字符
    }

    summary = remove(summary);
    return summary;
}

size_t WebPage::getBytes(char c)
{
    // 根据 UTF-8 编码规则判断字符所占字节数
    unsigned char uc = static_cast<unsigned char>(c);
    if (uc <= 0x7F)
    {
        return 1; // ASCII字符
    }
    else if (uc >= 0xC0 && uc <= 0xDF)
    {
        return 2; // 两字节字符
    }
    else if (uc >= 0xE0 && uc <= 0xEF)
    {
        return 3; // 三字节字符
    }
    else if (uc >= 0xF0 && uc <= 0xF7)
    {
        return 4; // 四字节字符
    }
    return 1; // 默认返回1字节（处理异常情况）
}

size_t WebPage::length(const string &s)
{
    size_t len = 0;
    for (size_t i = 0; i < s.length();)
    {
        size_t bytes = getBytes(s[i]);
        i += bytes;
        ++len;
    }
    return len;
}

string WebPage::getDoc() const
{
    ostringstream oss;
    oss << "<doc>" << '\n'
        << '\t' << "<docid>" << _docid << "</docid>" << '\n'
        << '\t' << "<title>" << _title << "</title>" << '\n'
        << '\t' << "<link>" << _link << "</link>" << '\n'
        << '\t' << "<content>" << _content << "</content>" << '\n'
        << "</doc>" << '\n';
    return oss.str();
}

uint64_t WebPage::getSimhash() const
{
    return _simhashVal;
}
// 构建单词映射表
void WebPage::buildWordsMap(WordSegmentation &jieba)
{
    unordered_set<string> &stopWords = Configuration::getInstance()->getStopWords();
    vector<string> words = jieba.cut(_content);
    // 这里word直接从网页content中读取,可能会有大小写问题,需要统一转换为小写
    for (auto &word : words)
    {
        // 统一将单词转换为小写
        string lowerWord;
        if (isEnglish(word))
        {
            lowerWord.reserve(word.size());
            for (char c : word)
            {
                if (isalpha(c))
                {
                    lowerWord += tolower(c);
                }
                else
                {
                    lowerWord += c;
                }
            }
        }
        else
        {
            lowerWord = word;
        }

        // 检查小写后的单词是否在停用词表中
        if (stopWords.count(lowerWord))
        {
            continue;
        }

        if (isChinese(word))
        {
            ++_wordsMap[word];
        }
        else if (isEnglish(word))
        {
            // 此处 word 已经在前面处理为小写，可直接使用
            ++_wordsMap[word];
        }
    }
}

unordered_map<string, int> &WebPage::getWordsMap()
{
    return _wordsMap;
}