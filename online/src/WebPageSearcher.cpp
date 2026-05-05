#include "WebPageSearcher.h"

#include <cstdint>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <memory>
#include <string>
#include <boost/regex.hpp>
#include <boost/locale.hpp>

#include <nlohmann/json.hpp>
using namespace std;
using namespace chrono;
using json = nlohmann::json;

// 验证并过滤无效 UTF-8 字符
std::string filterInvalidUTF8(const std::string &input)
{
    std::string result;
    for (size_t i = 0; i < input.size();)
    {
        uint8_t c = static_cast<uint8_t>(input[i]);
        if (c <= 0x7F)
        { // ASCII 字符 (0x00-0x7F)
            result += input[i];
            ++i;
        }
        else if ((c & 0xE0) == 0xC0)
        { // 2字节字符
            if (i + 1 < input.size() && (static_cast<uint8_t>(input[i + 1]) & 0xC0) == 0x80)
            {
                result.append(input, i, 2);
                i += 2;
            }
            else
            {
                ++i; // 跳过无效字符
            }
        }
        else if ((c & 0xF0) == 0xE0)
        { // 3字节字符
            if (i + 2 < input.size() &&
                (static_cast<uint8_t>(input[i + 1]) & 0xC0) == 0x80 &&
                (static_cast<uint8_t>(input[i + 2]) & 0xC0) == 0x80)
            {
                result.append(input, i, 3);
                i += 3;
            }
            else
            {
                ++i; // 跳过无效字符
            }
        }
        else if ((c & 0xF8) == 0xF0)
        { // 4字节字符
            if (i + 3 < input.size() &&
                (static_cast<uint8_t>(input[i + 1]) & 0xC0) == 0x80 &&
                (static_cast<uint8_t>(input[i + 2]) & 0xC0) == 0x80 &&
                (static_cast<uint8_t>(input[i + 3]) & 0xC0) == 0x80)
            {
                result.append(input, i, 4);
                i += 4;
            }
            else
            {
                ++i; // 跳过无效字符
            }
        }
        else
        {
            ++i; // 跳过无效字符
        }
    }
    return result;
}
// 读取偏移库，获取网页在网页库中的位置
unordered_map<int, pair<size_t, size_t>> readOffsetFile()
{
    unordered_map<int, pair<size_t, size_t>> offsetMap;
    ifstream offsetFile(CONFIG[OFFSET_PATH]);
    if (!offsetFile)
    {
        throw runtime_error("Failed to open web page offset file.");
    }
    string line;
    while (getline(offsetFile, line))
    {
        istringstream iss(line);
        int docid;
        size_t offset, length;
        if (iss >> docid >> offset >> length)
        {
            offsetMap[docid] = {offset, length};
        }
    }
    return offsetMap;
}
// 从网页库中读取网页信息
vector<string> readWebPage(int docid, const unordered_map<int, pair<size_t, size_t>> &offsetMap)
{
    ifstream pageLib(CONFIG[NEW_RIPEPAGE_PATH], ios::binary);
    if (!pageLib)
    {
        throw runtime_error("Failed to open web page lib file.");
    }
    auto it = offsetMap.find(docid);
    if (it == offsetMap.end())
    {
        return {};
    }
    size_t offset = it->second.first;
    size_t length = it->second.second;
    pageLib.seekg(offset);
    string content(length, '\0');
    pageLib.read(&content[0], length);
    istringstream iss(content);
    getline(iss, content); // 跳过<doc>标签
    string id, title, url, text;
    getline(iss, id);
    getline(iss, title);
    getline(iss, url);
    getline(iss, text);
    return {id, url, title, text};
}
// 读取倒排索引库
unordered_map<string, vector<pair<int, double>>> readInvertIndex()
{
    unordered_map<string, vector<pair<int, double>>> invertIndex;
    ifstream indexFile(CONFIG[INDEX_PATH]);
    if (!indexFile)
    {
        throw runtime_error("Failed to open invert index file.");
    }
    string line;
    while (getline(indexFile, line))
    {
        istringstream iss(line);
        string word;
        iss >> word;
        int docid;
        // 读取已归一化的权重
        double weight;
        while (iss >> docid >> weight)
        {
            invertIndex[word].emplace_back(docid, weight);
        }
    }
    return invertIndex;
}

struct SimilarityCompare
{
    SimilarityCompare(vector<double> &base) : _base(base) {}

    /**
     * 重载操作符()以实现文档相似度的比较。
     * @param lhs 左侧文档的ID和向量
     * @param rhs 右侧文档的ID和向量
     * @return 返回true表示左侧文档比右侧文档更相似于基础向量。
     */
    bool operator()(
        const pair<int, vector<double>> &lhs,
        const pair<int, vector<double>> &rhs)
    { // 都与基准向量进行计算
        double lhsCrossProduct = 0;
        double rhsCrossProduct = 0;
        double lhsVectorLength = 0;
        double rhsVectorLength = 0;

        for (int index = 0; index != _base.size(); ++index)
        {
            // 分子
            lhsCrossProduct += (lhs.second)[index] * _base[index];
            rhsCrossProduct += (rhs.second)[index] * _base[index];
            // 分母
            lhsVectorLength += pow((lhs.second)[index], 2);
            rhsVectorLength += pow((rhs.second)[index], 2);
        }

        if (lhsCrossProduct / sqrt(lhsVectorLength) <
            rhsCrossProduct / sqrt(rhsVectorLength))
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    vector<double> _base;
};

WebPageSearcher::WebPageSearcher()
{
}
vector<double> WebPageSearcher::getQueryWordsWeight(vector<string> &queryWords,
                                                    const unordered_map<string, vector<pair<int, double>>> &invertIndex, size_t page_num)
{
    vector<double> weightVec;
    if (queryWords.empty())
    {
        return weightVec;
    }

    // 1. 计算每个查询词的TF (词频)
    unordered_map<string, int> wordCount;
    for (const auto &word : queryWords)
    {
        ++wordCount[word];
    }

    // 2. 计算每个查询词的IDF (逆文档频率)
    unordered_map<string, double> idfMap;
    for (const auto &word : queryWords)
    {
        auto it = invertIndex.find(word);
        if (it != invertIndex.end())
        {
            // IDF = log(总文档数 / (包含该词的文档数 + 1))
            double idf = log(page_num / (it->second.size() + 1.0));
            idfMap[word] = idf;
        }
        else
        {
            idfMap[word] = 0.0;
        }
    }

    // 3. 计算TF-IDF权重
    for (const auto &word : queryWords)
    {
        double tf = wordCount[word] / (double)queryWords.size();
        double idf = idfMap[word];
        weightVec.push_back(tf * idf);
    }

    // 4. 归一化处理
    double sum = 0.0;
    for (auto &weight : weightVec)
    {
        sum += weight * weight;
    }
    sum = sqrt(sum);
    if (sum > 0)
    {
        for (auto &weight : weightVec)
        {
            weight /= sum;
        }
    }

    return weightVec;
}
// 主业务
string WebPageSearcher::doQuery(const string &query_word)
{
    vector<string> queryWords = _jieba.cut(query_word);
    auto invertIndex = readInvertIndex();
    auto offsetMap = readOffsetFile();
    size_t page_num = offsetMap.size();
    // 关键字的权重向量
    vector<double> weightList = getQueryWordsWeight(queryWords, invertIndex, page_num);
    SimilarityCompare similarityCom(weightList);
    // 获取包含所有查询词的网页ID列表
    vector<int> pages = getPages(queryWords, invertIndex);
    string ret;
    if (pages.size() == 0)
    {
        ret = returnNoAnswer();
        return ret;
    }

    vector<pair<int, vector<double>>> resultList; // 存储文档ID和权重向量
    for (auto &docid : pages)
    {

        vector<double> wordWeight;
        for (auto &word : queryWords)
        {
            auto it = invertIndex.find(word);
            if (it != invertIndex.end())
            {
                bool found = false;
                for (const auto &item : it->second)
                {
                    if (item.first == docid)
                    {
                        wordWeight.push_back(item.second);
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    wordWeight.push_back(0.0);
                }
            }
            else
            {
                wordWeight.push_back(0.0);
            }
        }
        resultList.emplace_back(make_pair(docid, wordWeight));
    }
    // 相似度排序
    stable_sort(resultList.begin(), resultList.end(), similarityCom);
    pages.clear();
    for (auto &item : resultList)
    {
        pages.push_back(item.first);
    }
    ret = createJson(pages, queryWords, offsetMap);
    return ret;
}
vector<int> WebPageSearcher::getPages(vector<string> &queryWords, const unordered_map<string, vector<pair<int, double>>> &invertIndex)
{
    if (queryWords.empty())
    {
        return {};
    }

    set<int> pages;
    cout << "查询词数量: " << queryWords.size() << endl;

    for (const auto &word : queryWords)
    {
        set<int> wordPages;
        // 直接从倒排索引文件中获取数据
        auto it = invertIndex.find(word);
        if (it != invertIndex.end())
        {
            for (const auto &item : it->second)
            {
                wordPages.insert(item.first);
            }
        }
        else
        {
            // 没有的词不影响交集
            continue;
        }

        if (pages.empty())
        {
            pages = wordPages;
        }
        else
        {
            // 取交集
            set<int> temp;
            set_intersection(pages.begin(), pages.end(),
                             wordPages.begin(), wordPages.end(),
                             inserter(temp, temp.begin()));
            pages.swap(temp);
        }
    }
    return vector<int>(pages.begin(), pages.end());
}
string WebPageSearcher::createJson(vector<int> &docIdVec, const vector<string> &queryWords, const unordered_map<int, pair<size_t, size_t>> &offsetMap)
{
    json root;
    root["msgID"] = 2;
    cout << "结果网页数量: " << docIdVec.size() << endl;
    for (auto id : docIdVec)
    {
        // 直接从网页库文件中获取网页信息
        vector<string> pageInfo = readWebPage(id, offsetMap);
        if (!pageInfo.empty() && pageInfo.size() >= 4)
        {
            string id = filterInvalidUTF8(pageInfo[0]);
            string title = filterInvalidUTF8(pageInfo[1]);
            string url = filterInvalidUTF8(pageInfo[2]);
            string content = filterInvalidUTF8(pageInfo[3]);

            // 生成摘要：取内容前50个字符
            string summary = content.substr(0, 50);
            // 如果内容不足50字符，则取全部内容
            if (summary.size() < content.size())
            {
                summary += "...";
            }

            json elem;
            elem["id"] = id;
            elem["title"] = title;
            elem["url"] = url;
            elem["abstract"] = filterInvalidUTF8(summary);
            root["files"].push_back(elem);
        }
    }
    return root.dump(4);
}
string WebPageSearcher::returnNoAnswer()
{
    json root;
    json elem;
    root["msgID"] = 2;
    elem["id"] = "404";
    elem["title"] = "404, not found";
    elem["url"] = "";
    elem["abstract"] = "未找到你搜索的内容";
    root["files"].push_back(elem);
    return root.dump(4);
}