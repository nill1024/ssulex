
#include <stdio.h>
#include <stdlib.h>
#include <libavformat/avformat.h>

int find_video_stream(AVFormatContext *context)
{
    for (int i=0; i<context->nb_streams; i++)
        if (context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            return i;
    return -1;
}
 
int index_key_frames(FILE *output, const char *input)
{
    AVFormatContext *context = NULL;

    int ret;
    if ((ret = avformat_open_input(&context, input, NULL, NULL)) < 0) {
        fprintf(stderr, "avformat_open_input() failed -> %s.\n", input);
        return -1;
    }

    if ((ret = avformat_find_stream_info(context, NULL)) < 0) {
        fprintf(stderr, "avformat_find_stream_info() failed -> %s.\n", input);
        goto error;
    }

    int video_index = find_video_stream(context);
    if (video_index < 0) {
        fprintf(stderr, "Failed to find a video stream -> %s.\n", input);
        goto error;
    }

    while (1) {
        AVPacket packet;

        if ((ret = av_read_frame(context, &packet)) < 0)
            break;

        if (packet.stream_index != video_index)
            continue;

        if (packet.flags & AV_PKT_FLAG_KEY > 0)
            fwrite(&packet.dts, sizeof(int64_t), 1, output);

        av_packet_unref(&packet);
    }

    avformat_close_input(&context);
    return 0;

error:

    avformat_close_input(&context);
    return -1;
}

