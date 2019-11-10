
#ifndef _INDEXING_HPP_
#define _INDEXING_HPP_

extern "C" {
    #include <libavformat/avformat.h>
}

#include <cstdio>
#include <string>
#include <vector>
#include <utility>
#include <map>

struct media_error
{
    std::string message;
    template<typename... Args>
    media_error(const char *format, Args ...args) {
        int length = snprintf(NULL, 0, format, args...);
        message.resize(length);
        sprintf(message.data(), format, args...);
    }
    media_error(const media_error &e) : message(e.message) {}
    media_error(media_error &&e) : message(std::move(e.message)) {}
};

class output_media;

class media
{
    std::string path;
    AVFormatContext *context = NULL;
    AVCodecContext *decoder_context = NULL;
    int video_index = -1;
    int audio_index = -1;

    // Identifies the indices of video & audio streams.
    void locate_streams(void);

    // The list of I frame timestamps from which to start encoding.
    // Currently, the gap between those is at least 2 seconds.
    std::vector<int64_t> segments;
    void index_segments(int stream_index);

    // For all AVPackets in index-th segment, call handler.
    void for_each_AVPacket(int index, output_media *om,
        void (media::*handler)(AVPacket *packet, output_media *om));

    void remux_packet(AVPacket *packet, output_media *om);
    void decode_packet(AVPacket *packet, output_media *om);

    void begin_decode(int index);
    void end_decode(void);

public:

    media(const char *path);
    ~media(void);

    // Exports or loads 'segments' to remember what we've already indexed.
    const std::vector<int64_t> &get_segments(void);
    template<typename T>
    void load_segments(T &&segments) {
        this->segments = std::forward<T>(segments);
    }

    void index(void);

    void remux(int index);
    void transcode(int index);
};

#define VIDEO_ENCODER  "libx264"

class output_media
{
    std::string path;
    AVFormatContext *context = NULL;
    AVCodecContext *encoder_context = NULL;

    double start = 0.;
    std::map<int, int> index;
    std::map<int, int64_t> delay;

public:

    template<typename... Args>
    output_media(const char *format, Args ...args) {
        int length = snprintf(NULL, 0, format, args...);
        path.resize(length);
        sprintf(path.data(), format, args...);
        
        if (avformat_alloc_output_context2(&context, NULL, NULL, this->path.c_str()) < 0)
            throw media_error("No format available for %s.", this->path.c_str());
    }
    ~output_media(void);

    void map_stream(AVStream *from, AVCodecContext *decoder_context = NULL);
    void init(AVStream *stream, AVPacket *packet);
    void push_prologue(void);
    void push_packet(AVStream *stream, AVPacket *packet);
    void push_frame(AVStream *stream, AVFrame *frame);
    void push_epilogue(void);
};

#endif

