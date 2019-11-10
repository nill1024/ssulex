
extern "C" {
    #include <libavformat/avformat.h>
    #include <libavutil/opt.h>
}

#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <indexing.hpp>

media::media(const char *path) : path(path)
{
    if (avformat_open_input(&context, path, NULL, NULL) < 0)
        throw media_error("avformat_open_input failed -> %s.", path);

    if (avformat_find_stream_info(context, NULL) < 0)
        throw media_error("avformat_find_stream_info failed -> %s.", path);

    locate_streams();
}

media::~media(void)
{
    if (context)
        avformat_close_input(&context);
    if (decoder_context)
        avcodec_free_context(&decoder_context);
}

void media::locate_streams(void)
{
    for (int i=0; i<context->nb_streams; i++)
        if (video_index < 0
            && context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            video_index = i;
        else if (audio_index < 0
            && context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            audio_index = i;

    if (video_index < 0 && audio_index < 0)
        throw media_error("Not a media file.");
}

void media::index_segments(int stream_index)
{
    double time_base = av_q2d(context->streams[stream_index]->time_base);
    double segment_duration = .0;

    AVPacket packet;
    while (av_read_frame(context, &packet) == 0) {

        if (packet.stream_index != stream_index)
            continue;

        // 새로운 I 프레임을 발견한 경우,
        if (packet.flags & AV_PKT_FLAG_KEY) {
            // 첫 조각이 아직 없으면,
            if (segments.size() == 0)
                segments.push_back(packet.dts); //  이 프레임을 첫 조각의 시작으로!
            // 현재 조각이 2초를 넘었다면,
            else if (segment_duration > 2.) {
                segment_duration = .0;
                segments.push_back(packet.dts); // 새로운 조각을 시작한다!
            }
        }

        // 첫 조각이 정해진 경우에만,
        if (segments.size())
            segment_duration += time_base * packet.duration; 

        av_packet_unref(&packet);
    }
}

const std::vector<int64_t> &media::get_segments(void)
{
    if (segments.size() == 0)
        throw media_error("None indexed.");
    return segments;
}

void media::index(void)
{
    if (video_index >= 0)
        index_segments(video_index);
    else
        throw media_error("No stream to index.");
}

void media::for_each_AVPacket(int index, output_media *om,
    void (media::*handler)(AVPacket *packet, output_media *om))
{
    if (av_seek_frame(context, video_index, index ? segments[index-1] : segments[index], AVSEEK_FLAG_ANY) < 0)
        throw media_error("Failed to seek to an I frame.");

    // [vbegin, vend) == [segments[index], segments[index+1])
    int64_t vbegin = segments[index];
    int64_t vend = index+1 == segments.size() ? -1 : segments[index+1];

    // Convert [vbegin, vend) to match audio stream's time_base!
    int64_t abegin = av_rescale_q(vbegin, context->streams[video_index]->time_base, context->streams[audio_index]->time_base);
    int64_t aend = vend == -1 ? -1 : av_rescale_q(vend, context->streams[video_index]->time_base, context->streams[audio_index]->time_base);

    bool init = true;
    bool vdone = false;
    bool adone = false;

    while (true) {
        if (vdone && adone)
            break;

        AVPacket packet;
        if (av_read_frame(context, &packet) != 0)
            break;

        if (packet.stream_index == video_index) {
            if (vend != -1 && packet.dts >= vend)
                vdone = true;
            else if (packet.dts >= vbegin) {
                if (init) {
                    om->init(context->streams[video_index], &packet);
                    init = false;
                }
                (this->*handler)(&packet, om);
            }
        }

        if (packet.stream_index == audio_index) {
            if (aend != -1 && packet.dts >= aend)
                adone = true;
            else if (packet.dts >= abegin)
                (this->*handler)(&packet, om);
        }

        av_packet_unref(&packet);
    }
    
}

void media::remux_packet(AVPacket *packet, output_media *om)
{
    om->push_packet(context->streams[packet->stream_index], packet);
}

void media::decode_packet(AVPacket *packet, output_media *om)
{
    if (packet == NULL || packet->stream_index == video_index) {
        AVFrame *frame = av_frame_alloc();
        if (!frame)
            throw media_error("Failed to allocate an AVFrame.");
    
        int ret = avcodec_send_packet(decoder_context, packet);
        if (ret < 0)
            throw media_error("Failed to send a packet for decoding.");
    
        while (ret >= 0) {
            ret = avcodec_receive_frame(decoder_context, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                break;
            else if (ret < 0)
                throw media_error("Failed to encode.");
    
            om->push_frame(context->streams[video_index], frame);
        }
        
        av_frame_free(&frame);
    }
    else
        om->push_packet(context->streams[packet->stream_index], packet);
}

void media::begin_decode(int index)
{
    // 해당 코덱의 디코더 찾기
    AVCodec *decoder = avcodec_find_decoder(context->streams[index]->codecpar->codec_id);
    if (!decoder)
        throw media_error("Failed to find the decoder.");

    // 디코딩에 쓸 컨텍스트 할당
    decoder_context = avcodec_alloc_context3(decoder);
    if (!decoder_context)
        throw media_error("Failed to allocate the decoder context.");

    if (avcodec_parameters_to_context(decoder_context, context->streams[index]->codecpar) < 0)
        throw media_error("Failed to copy decoder parameters.");

    // 디코더를 열고, 이를 컨텍스트에 반영
    if (avcodec_open2(decoder_context, decoder, NULL) < 0)
        throw media_error("Failed to open the decoder.");
}

void media::end_decode(void)
{
    avcodec_free_context(&decoder_context);
}

void media::remux(int index)
{
    output_media om("%02d.mp4", index);
    om.map_stream(context->streams[video_index]);
    om.map_stream(context->streams[audio_index]);
    om.push_prologue();
    for_each_AVPacket(index, &om, &media::remux_packet);
    om.push_epilogue();
}

void media::transcode(int index)
{
    begin_decode(video_index);
    output_media om("%02d.mp4", index);
    om.map_stream(context->streams[video_index], decoder_context);
    om.map_stream(context->streams[audio_index]);
    om.push_prologue();
    for_each_AVPacket(index, &om, &media::decode_packet);
    decode_packet(NULL, &om);
    om.push_frame(context->streams[video_index], NULL);
    om.push_epilogue();
    end_decode();
}

void output_media::init(AVStream *stream, AVPacket *packet)
{
    start = packet->dts * av_q2d(stream->time_base);
    delay.emplace(stream->index, packet->dts);
}

output_media::~output_media(void)
{
    if (!(context->oformat->flags & AVFMT_NOFILE))
        avio_closep(&context->pb);
    avformat_free_context(context);
    avcodec_free_context(&encoder_context);
}

void output_media::map_stream(AVStream *from, AVCodecContext *decoder_context)
{
    AVStream *to = avformat_new_stream(context, NULL);
    if (to == NULL)
        throw media_error("Failed to allocate a new stream.");

    if (decoder_context) {

        AVCodec *encoder;

        // 미리 설정한 인코더를 코덱 타입에 맞게 찾는다.
        if (decoder_context->codec_type == AVMEDIA_TYPE_VIDEO)
            encoder = avcodec_find_encoder_by_name(VIDEO_ENCODER);
        else
            throw media_error("Audio encoding is currently not supported.");

        // 시스템에 인코더가 없으면,
        if (!encoder)
            throw media_error("Failed to find the encoder.");

        // 인코딩에 사용할 컨텍스트를 할당한다.
        encoder_context = avcodec_alloc_context3(encoder);
        if (!encoder_context)
            throw media_error("Failed to allocate an encoder context.");

        encoder_context->height = decoder_context->height;
        encoder_context->width = decoder_context->width;
        encoder_context->sample_aspect_ratio = decoder_context->sample_aspect_ratio;
        encoder_context->pix_fmt = decoder_context->pix_fmt;
        encoder_context->time_base = from->time_base; // ???

        if (avcodec_open2(encoder_context, encoder, NULL) < 0)
            throw media_error("Failed to open the encoder.");

        if (avcodec_parameters_from_context(to->codecpar, encoder_context) < 0)
            throw media_error("Failed to copy codec parameters.");
    }
    else
        if (avcodec_parameters_copy(to->codecpar, from->codecpar) < 0)
            throw media_error("Failed to copy codec parameters.");
    
    to->codecpar->codec_tag = 0;
    index.emplace(from->index, to->index);
}

void output_media::push_prologue(void)
{
    if (!(context->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&context->pb, path.c_str(), AVIO_FLAG_WRITE) < 0)
            throw media_error("Failed to open %s.", path.c_str());
    }

    if (avformat_write_header(context, NULL) < 0)
        throw media_error("Failed to write a header for %s.", path.c_str());
}

void output_media::push_packet(AVStream *stream, AVPacket *packet)
{
    // Reject if the stream was not mapped.
    if (index.find(stream->index) == index.end())
        throw media_error("");

/*
    if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO
        && delay.find(stream->index) == delay.end()) {
        start = packet->dts * av_q2d(stream->time_base);
        delay.emplace(stream->index, packet->dts);
    }
*/
    if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO
        && delay.find(stream->index) == delay.end()) {
        double start = packet->dts * av_q2d(stream->time_base);
        int64_t gap = static_cast<int64_t>((start - this->start) / av_q2d(stream->time_base));
        delay.emplace(stream->index, packet->dts - gap);
    }

    packet->pos = -1;
    packet->pts = av_rescale_q(packet->pts - delay[stream->index], stream->time_base, context->streams[index[stream->index]]->time_base);
    packet->dts = av_rescale_q(packet->dts - delay[stream->index], stream->time_base, context->streams[index[stream->index]]->time_base);
    packet->duration = av_rescale_q(packet->duration, stream->time_base, context->streams[index[stream->index]]->time_base);
    packet->stream_index = index[stream->index];

    if (av_interleaved_write_frame(context, packet) < 0)
        throw media_error("Failed to write packet.");
}

void output_media::push_frame(AVStream *stream, AVFrame *frame)
{
    AVPacket *packet = av_packet_alloc();
    if (!packet)
        throw media_error("Failed to allocate a AVPacket.");

    int ret = avcodec_send_frame(encoder_context, frame);
    if (ret < 0)
        throw media_error("Failed to send a frame for encoding.");

    while (ret >= 0) {
        ret = avcodec_receive_packet(encoder_context, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        else if (ret < 0)
            throw media_error("Failed to encode.");

        push_packet(stream, packet);
        av_packet_unref(packet);
    }
    
    av_packet_free(&packet);
}

void output_media::push_epilogue(void)
{
    av_write_trailer(context);
}

