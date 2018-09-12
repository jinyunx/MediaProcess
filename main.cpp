#include "Demuxer.h"
#include "Decoder.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("usage: %s infile\n", argv[0]);
        return 1;
    }

    Demuxer demuxer;
    if (!demuxer.Init(argv[1]))
        av_log(nullptr, AV_LOG_ERROR, "Demuxer init error\n");
    else
        av_log(nullptr, AV_LOG_INFO, "Demuxer init ok\n");

    Decoder decoder;
    if (!decoder.Init(*demuxer.GetFormatContext()))
        av_log(nullptr, AV_LOG_ERROR, "Decoder init error\n");
    else
        av_log(nullptr, AV_LOG_INFO, "Decoder init ok\n");

    AVPacket packet;
    packet.data = nullptr;
    packet.size = 0;
    while(demuxer.ReadPacket(&packet))
    {
        if (demuxer.GetMediaType(packet.stream_index) == AVMEDIA_TYPE_AUDIO)
        {
            const AVCodecParameters *param = demuxer.GetCodecPram(packet.stream_index);
            printf("pts:%d, frame_size:%d, sample_rate:%d\n",
                    packet.pts, param->frame_size, param->sample_rate);
        }
    }

    return 0;
}
