#include "SearchEngineServer.h"
#include "TcpConnection.h"
#include <functional>
#include <iostream>

#include <nlohmann/json.hpp>
using namespace std;
using namespace std::placeholders;
using json = nlohmann::json;

void SearchEngineServer::warmUpCaches()
{
    // 预热常见搜索建议
    vector<string> commonQueries = {"天气", "新闻", "地图", "翻译"};
    for (auto &query : commonQueries)
    {
        string result = _recommander.doQuery(query);
        _pool.addToMainKeyCache(query, result); // 使用公有方法添加缓存
    }

    // 预热热门网页结果
    vector<string> hotQueries = {"王道", "世界杯", "股票"};
    for (auto &query : hotQueries)
    {
        string result = _searcher.doQuery(query);
        _pool.addToMainPageCache(query, result); // 使用公有方法添加缓存
    }
}

SearchEngineServer::SearchEngineServer(size_t threadNum, size_t queSize, const string &ip, uint16_t port)
    : _server(ip, port), _pool(threadNum, queSize), _recommander(CONFIG[CN_DICT_PATH], CONFIG[CN_INDEX_PATH])
{
}
SearchEngineServer::~SearchEngineServer() {}
void SearchEngineServer::start()
{
    _pool.start();
    warmUpCaches();
    using namespace std::placeholders;
    _server.setAllCallback(std::bind(&SearchEngineServer::onConnection, this, _1),
                           std::bind(&SearchEngineServer::onMessage, this, _1),
                           std::bind(&SearchEngineServer::onClose, this, _1));
    _server.start();
}
void SearchEngineServer::stop()
{
    _pool.stop();
    _server.stop();
}
void SearchEngineServer::onConnection(const TcpConnectionPtr &conn)
{
    cout << conn->toString() << " has connected!" << endl;
}
void SearchEngineServer::onMessage(const TcpConnectionPtr &conn)
{
    string msg = conn->receive();
    cout << "recv msg from client: " << msg << endl;
    _pool.addTask(std::bind(&SearchEngineServer::doTaskThread, this, conn, msg));
}
void SearchEngineServer::onClose(const TcpConnectionPtr &conn)
{
    cout << conn->toString() << " has closed!" << endl;
}
void SearchEngineServer::doTaskThread(const TcpConnectionPtr &conn, string &jsonmsg)
{
    // 添加调试输出，检查线程数据
    cout << "Thread " << std::this_thread::get_id() << " processing request" << endl;

    static thread_local ThreadLocalData *tls = [this]()
    {
        auto threadData = this->_pool.getThreadData();
        cout << "Initializing TLS for thread " << std::this_thread::get_id()
             << ", found " << threadData.size() << " thread data entries" << endl;
        auto currentId = std::this_thread::get_id();
        for (auto *data : threadData)
        {
            if (data->owner_thread_id == currentId)
            {
                return data;
            }
        }
        return static_cast<ThreadLocalData *>(nullptr);
    }();

    json js = json::parse(jsonmsg);
    int query_id = js["query_id"];
    string msg = js["msg"];
    string ret;

    switch (query_id)
    {
    case 1:
        if (tls)
        {
            cout << "Checking cache for key: " << msg << endl;
            if (tls->keycache.get(msg, ret))
            {
                cout << "keyCache hit! Key: " << msg << " Value: " << ret << endl;
                conn->sendInLoop(ret);
                return;
            }
            cout << "Cache miss for key: " << msg << endl;
        }
        cout << "Cache miss!" << endl;
        ret = _recommander.doQuery(msg);
        if (tls)
        {
            tls->keycache.dump();
            cout << "Updating keycache with key: " << msg << " value: " << ret << endl;
            tls->keycache.put(msg, ret);
            std::lock_guard<std::mutex> lock(tls->patch_mutex);
            tls->patch.keypatch[msg] = ret;
            cout << "keyCache updated successfully" << endl;
        }
        break;
    case 2:
        if (tls)
        {
            cout << "Checking cache for key: " << msg << endl;
            if (tls->pagecache.get(msg, ret))
            {
                cout << "pageCache hit! Key: " << msg << " Value: " << ret << endl;
                conn->sendInLoop(ret);
                return;
            }
            cout << "pageCache miss for key: " << msg << endl;
        }
        cout << "pageCache miss!" << endl;
        ret = _searcher.doQuery(msg);
        if (tls)
        {
            tls->pagecache.dump();
            cout << "Updating pagecache with key: " << msg << " value: " << ret << endl;
            tls->pagecache.put(msg, ret);
            std::lock_guard<std::mutex> lock(tls->patch_mutex);
            tls->patch.pagepatch[msg] = ret;
            cout << "pageCache updated successfully" << endl;
        }
        break;
    }

    cout << "result: " << endl;
    cout << ret << endl;
    conn->sendInLoop(ret);
}
