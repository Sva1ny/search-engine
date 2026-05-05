# SearchEngine

## 介绍
这是一个基于C++11实现的搜索引擎项目，支持离线页面处理、关键词提取和在线搜索服务。项目包含完整的离线数据处理流程和高性能的在线搜索服务器。

## 软件架构
项目分为三个主要模块：
1. **offline_page** - 离线页面处理模块
   - XML文档解析
   - 网页去重处理
   - 倒排索引构建
   - 数据持久化存储

2. **offline_keyword** - 关键词处理模块
   - 中英文分词处理
   - 词典构建
   - 索引生成

3. **online** - 在线搜索服务模块
   - 基于epoll的高性能网络服务
   - 多线程任务处理
   - 智能关键词推荐
   - 相关性搜索算法

## 安装教程
1. 安装依赖库
   ```bash
   sudo apt-get install libxml2-dev
   ```

2. 克隆项目
   ```bash
   git clone https://gitee.com/ailhlhlh/search-engine
   ```

3. 构建项目
   ```bash
   cd SearchEngine
   mkdir build && cd build
   cmake ..
   make -j4
   ```

## 使用说明
1. 离线数据处理
   ```bash
   # 处理网页数据
   ./offline_page/offline_page
   
   # 处理关键词数据
   ./offline_keyword/offline_keyword
   ```

2. 启动搜索服务
   ```bash
   ./online/online
   ```

3. 使用客户端测试
   ```bash
   ./online/client
   ```

## 参与贡献
1. Fork项目
2. 创建功能分支
3. 提交代码
4. 创建Pull Request

## 特技
- 支持中英文混合搜索
- 基于Simhash算法的网页去重
- 基于TF-IDF的关键词提取
- 高性能网络服务架构
- 智能关键词补全推荐

## 许可证
本项目采用MIT License，请查看项目根目录下的LICENSE文件获取详细信息。
