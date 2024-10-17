#include "video_filter_manager.h"
#include "ai_renderer_video_filter.h"
#include "logger.h"

VideoFilterManager& VideoFilterManager::instance()
{
    static VideoFilterManager instance;
    if (!instance.isInitialized())
        instance.initialize();
    return instance;
}

void VideoFilterManager::initialize()
{
    VideoFilterPtr ai_renderer_filter = std::make_shared<AIRendererFilter>();
    ai_renderer_filter->initialize();
    m_filters.push_back(ai_renderer_filter);
}

int VideoFilterManager::getNumOfFilters() const
{
    return static_cast<int>(m_filters.size());
}

VideoFilterPtr VideoFilterManager::getFilter(size_t index) const
{ 
    if (index < m_filters.size())
        return m_filters[index]; 

//    LogError("Filter Index not found: " + std::to_string(index) + " filter count: " + std::to_string(int(m_filters.size())));
    return nullptr;
}
