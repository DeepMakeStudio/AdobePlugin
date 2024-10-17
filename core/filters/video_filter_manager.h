#include "video_filter.h"
#include <vector>

class VideoFilterManager
{
public:
    static VideoFilterManager& instance();

    void initialize();
    int getNumOfFilters() const;
    VideoFilterPtr getFilter(size_t index) const;

    bool isInitialized() const {return m_initialized;}
protected:
    bool m_initialized {false};
    std::vector<VideoFilterPtr> m_filters;
};
