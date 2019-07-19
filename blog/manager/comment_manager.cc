#include "comment_manager.h"
#include "sylar/log.h"
#include "sylar/util.h"
#include "blog/util.h"

namespace blog {

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

bool CommentManager::loadAll() {
    auto db = GetDB();
    if(!db) {
        SYLAR_LOG_ERROR(g_logger) << "Get SQLite3 connection fail";
        return false;
    }
    std::vector<data::CommentInfo::ptr> results;
    if(blog::data::CommentInfoDao::QueryAll(results, db)) {
        SYLAR_LOG_ERROR(g_logger) << "CommentManager loadAll fail";
        return false;
    }

    std::unordered_map<int64_t, blog::data::CommentInfo::ptr> datas;
    std::unordered_map<int64_t, blog::data::CommentInfo::ptr> users;

    for(auto& i : results) {
        datas[i->getId()] = i;
        users[i->getUserId()] = i;
    }

    sylar::RWMutex::WriteLock lock(m_mutex);
    m_datas.swap(datas);
    m_articles.swap(users);
    return true;
}

void CommentManager::add(blog::data::CommentInfo::ptr info) {
    sylar::RWMutex::WriteLock lock(m_mutex);
    m_datas[info->getId()] = info;
    m_articles[info->getUserId()] = info;
}

#define XX(map, key) \
    sylar::RWMutex::ReadLock lock(m_mutex); \
    auto it = map.find(key); \
    return it == map.end() ? nullptr : it->second;

blog::data::CommentInfo::ptr CommentManager::get(int64_t id) {
    XX(m_datas, id);
}

blog::data::CommentInfo::ptr CommentManager::getByArticleId(int64_t id) {
    XX(m_articles, id);
}

#undef XX

}