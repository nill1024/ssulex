
extern "C" {
    #include <libavformat/avformat.h>
    #include <libavutil/opt.h>
}

#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <media.hpp>

/////////////////////////////////////////////////////////

AVBufferRef *hw_device_ctx = NULL;

enum AVPixelFormat get_vaapi_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts) {
    const enum AVPixelFormat *p;
    for (p = pix_fmts; *p != AV_PIX_FMT_NONE; p++) {
        if (*p == AV_PIX_FMT_VAAPI)
            return *p;
    }
    return AV_PIX_FMT_NONE;
}

/////////////////////////////////////////////////////////

media::media(std::string path) : path(std::move(path))
{
    if (avformat_open_input(&context, this->path.c_str(), NULL, NULL) < 0)
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
                duration.push_back(segment_duration);
                segments.push_back(packet.dts); // 새로운 조각을 시작한다!
                segment_duration = .0;
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

const std::vector<double> &media::get_duration(void)
{
    if (duration.size() == 0)
        throw media_error("None indexed.");
    return duration;
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
                throw media_error("Failed to decode.");
    
            om->push_frame(context->streams[video_index], frame);
        }

        av_frame_free(&frame);
    }
    else
        om->push_packet(context->streams[packet->stream_index], packet);
}

void media::begin_decode(void)
{
    begin_video_decode();
//    begin_audio_decode();
}

void media::begin_video_decode(void)
{
    // FIXME: 요고 해제할 곳이 마땅치 않음...
    if (hw_device_ctx == NULL) {
        if (av_hwdevice_ctx_create(&hw_device_ctx, AV_HWDEVICE_TYPE_VAAPI, NULL, NULL, 0) < 0)
            throw media_error("Failed to create a VAAPI device.");
    }
    
    // 해당 코덱의 디코더 찾기
    AVCodec *decoder = avcodec_find_decoder(context->streams[video_index]->codecpar->codec_id);
    if (!decoder)
        throw media_error("Failed to find the decoder.");

    // 디코딩에 쓸 컨텍스트 할당
    decoder_context = avcodec_alloc_context3(decoder);
    if (!decoder_context)
        throw media_error("Failed to allocate the decoder context.");

    if (avcodec_parameters_to_context(decoder_context, context->streams[video_index]->codecpar) < 0)
        throw media_error("Failed to copy decoder parameters.");

    // VAAPI 장치 연결
    decoder_context->hw_device_ctx = av_buffer_ref(hw_device_ctx);
    if (!decoder_context->hw_device_ctx)
        throw media_error("Failed to ref a VAAPI device.");
    decoder_context->get_format = get_vaapi_format;

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
    for_each_AVPacket(index, &om, &media::remux_packet);
    om.push_epilogue();
}

void media::transcode(int index, uint64_t bit_rate)
{
    begin_decode();
    output_media om("%02d.mp4", index);
    om.set_bit_rate(bit_rate);
    om.map_stream(context->streams[video_index], decoder_context);
    om.map_stream(context->streams[audio_index]);
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

    // FIXME: from <-> decoder_context가 전혀 관련 없을 수 있음.
    if (decoder_context) {
        if (from->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            this->decoder_context = decoder_context;
        else
            throw media_error("Audio decoding is not yet supported.");
    }

    // Defaults to stream copy.
    if (avcodec_parameters_copy(to->codecpar, from->codecpar) < 0)
        throw media_error("Failed to copy codec parameters.");

    to->codecpar->codec_tag = 0;
    index.emplace(from->index, to->index);
}

void output_media::push_packet(AVStream *stream, AVPacket *packet)
{
    // Reject if the stream was not mapped.
    if (index.find(stream->index) == index.end())
        throw media_error("%d is an invalid stream index.", stream->index);

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

    // VAAPI 인코더가 아직 초기화 안 됐으면,
    if (encoder_context == NULL) {
        // 요 패킷은 잠시 cache에 모셔두자.
        AVPacket *dup = (AVPacket *)malloc(sizeof(AVPacket));
        av_init_packet(dup);
        av_packet_ref(dup, packet);
        cache.emplace(dup);
    }
    else {
        // 처음이라고?
        if (!initialized) {

            // 파일 열고,
            if (!(context->oformat->flags & AVFMT_NOFILE)) {
                if (avio_open(&context->pb, path.c_str(), AVIO_FLAG_WRITE) < 0)
                    throw media_error("Failed to open %s.", path.c_str());
            }
        
            // 헤더 쓰자...
            if (avformat_write_header(context, NULL) < 0)
                throw media_error("Failed to write a header for %s.", path.c_str());

            // 밀린 패킷을 몽땅 써줍니다.
            while (cache.size()) {
                AVPacket *dup = cache.front();
                cache.pop();
                if (av_interleaved_write_frame(context, dup) < 0)
                    throw media_error("Failed to flush `AVPacket`s.");
                av_packet_unref(dup);
                free(dup);
            } 

            initialized = true;
        }

        if (av_interleaved_write_frame(context, packet) < 0)
            throw media_error("Failed to write packet.");
    }

}

void output_media::push_frame(AVStream *stream, AVFrame *frame)
{
    if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && encoder_context == NULL) {

        AVCodec *encoder;
        if (!(encoder = avcodec_find_encoder_by_name("h264_vaapi")))
            throw media_error("Failed to find `h264_vaapi`.");

        encoder_context = avcodec_alloc_context3(encoder);
        if (!encoder_context)
            throw media_error("Failed to allocate an encoder context.");
        
        encoder_context->hw_frames_ctx = av_buffer_ref(decoder_context->hw_frames_ctx);
        if (!encoder_context->hw_frames_ctx)
            throw media_error("Failed to ref `decoder_context->hw_frames_ctx`.");

        encoder_context->time_base = av_inv_q(decoder_context->framerate);
        encoder_context->pix_fmt = AV_PIX_FMT_VAAPI;
        encoder_context->width = decoder_context->width;
        encoder_context->height = decoder_context->height;
        encoder_context->bit_rate = bit_rate;
        av_opt_set(encoder_context->priv_data, "rc_mode", "CBR", 0);

        if (avcodec_open2(encoder_context, encoder, NULL) < 0)
            throw media_error("Failed to open `h264_vaapi`.");

        context->streams[index[stream->index]]->time_base = encoder_context->time_base;
        if (avcodec_parameters_from_context(context->streams[index[stream->index]]->codecpar, encoder_context))
            throw media_error("Failed to copy the stream parameters.");
    } 

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

