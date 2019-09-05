
extern "C" {
    #include <libavformat/avformat.h>
}

#include <string>
#include <list>
#include <utility>
#include <indexing.hpp>

media_info::media_info(std::string path) : path(std::move(path))
{
    if (avformat_open_input(&context, this->path.c_str(), NULL, NULL) < 0)
        throw media_info_error("avformat_open_input failed -> " + this->path + ".");

    if (avformat_find_stream_info(context, NULL) < 0)
        throw media_info_error("avformat_find_stream_info failed -> " + this->path + ".");

    locate_streams();

    if (video_index >= 0)
        index_video();
    else
        index_audio();
}

media_info::~media_info(void)
{
    if (context)
        avformat_close_input(&context);
}

void media_info::locate_streams(void)
{
    for (int i=0; i<context->nb_streams; i++)
        if (video_index < 0
            && context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            video_index = i;
        else if (audio_index < 0
            && context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            audio_index = i;

    if (video_index < 0 && audio_index < 0)
        throw stream_not_found("Not a media file.");
}

void media_info::index_video(void)
{
    index_segments();
}

void media_info::index_audio(void)
{
    throw stream_not_supported("An audio file is not yet supported.");
}

void media_info::index_segments(void)
{
    double time_base = av_q2d(context->streams[video_index]->time_base);
    double segment_duration = .0;
    segments.push_back(0);

    AVPacket packet;
    while (av_read_frame(context, &packet) == 0)
    {
        if (packet.stream_index != video_index)
            continue;
        if ((packet.flags & AV_PKT_FLAG_KEY) > 0
            && segment_duration > 2.)
        {
            segment_duration = .0;
            segments.push_back(packet.dts);
        }
        if (packet.duration == 0)
            throw stream_not_supported("The video stream lacks a timestamp.");
        segment_duration += time_base * packet.duration;
        av_packet_unref(&packet);
    }
}

const std::list<int64_t> &media_info::get_segments(void)
{
    return segments;
}

