
#ifndef _INDEXING_HPP_
#define _INDEXING_HPP_

extern "C" {
    #include <libavformat/avformat.h>
}

#include <string>
#include <list>
#include <cstdio>
#include <cstring>
#include <utility>

struct media_info_error
{
    std::string message;
    media_info_error(std::string &message) : message(message) {}
    media_info_error(std::string &&message) : message(std::move(message)) {}
};

struct stream_not_found : media_info_error { 
    using media_info_error::media_info_error;
};

struct stream_not_supported : media_info_error {
    using media_info_error::media_info_error;
};

class media_info
{
    std::string path;
    AVFormatContext *context = NULL;
    int video_index = -1;
    AVRational time_base = {0, 0};
    int audio_index = -1;
    std::list<int64_t> segments;
    void locate_streams(void);
    void index_video(void);
    void index_audio(void);
    void index_segments(void);
public:
    media_info(std::string path);
    ~media_info(void);
    const std::list<int64_t> &get_segments(void);
};

#endif

