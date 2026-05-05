#include <unordered_map>
#include <list>
#include <mutex>
#include <vector>
#include <thread>
#include <iostream>
using namespace std;

// LRU缓存实现
template <typename Key, typename Value>
class LRUCache
{
public:
    LRUCache(size_t capacity) : _capacity(capacity) {}

    bool get(const Key &key, Value &value)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _cache.find(key);
        if (it == _cache.end())
        {
            return false;
        }

        // 移动节点到链表头部
        _lru.splice(_lru.begin(), _lru, it->second);
        value = it->second->second;
        return true;
    }

    void put(const Key &key, const Value &value)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _cache.find(key);
        if (it != _cache.end())
        {
            // 更新值并移动节点
            it->second->second = value;
            _lru.splice(_lru.begin(), _lru, it->second);
            return;
        }

        if (_cache.size() >= _capacity)
        {
            _cache.erase(_lru.back().first);
            _lru.pop_back();
        }

        // 添加新节点
        _lru.emplace_front(key, value);
        _cache[key] = _lru.begin();
    }

    size_t capacity() const { return _capacity; }

    void dump() const
    {
        cout << "LRUCache content (size=" << _cache.size()
             << "/capacity=" << _capacity << "):" << endl;
        for (const auto &item : _lru)
        {
            cout << "  " << item.first << " => " << item.second << endl;
        }
    }

    // 新增方法：获取所有缓存项
    std::vector<std::pair<Key, Value>> getAll() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        std::vector<std::pair<Key, Value>> result;
        result.reserve(_lru.size());
        for (const auto &item : _lru)
        {
            result.emplace_back(item.first, item.second);
        }
        return result;
    }

private:
    size_t _capacity;
    std::list<std::pair<Key, Value>> _lru;
    std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator> _cache;
    mutable std::mutex _mutex;
};

// 线程本地存储
struct ThreadLocalData
{
    LRUCache<std::string, std::string> keycache;
    LRUCache<std::string, std::string> pagecache;
    std::mutex cache_mutex;

    struct Patch
    {
        std::unordered_map<std::string, std::string> keypatch;
        std::unordered_map<std::string, std::string> pagepatch;
    } patch;
    std::mutex patch_mutex;
    std::thread::id owner_thread_id; // 添加线程ID成员

    ThreadLocalData(size_t cache_size)
        : owner_thread_id(std::this_thread::get_id()),
          keycache(cache_size),
          pagecache(cache_size) {}
};