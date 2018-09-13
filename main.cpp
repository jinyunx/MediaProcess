#include "Demuxer.h"
#include "Decoder.h"

int64_t ToMs(int64_t ts, const AVRational *timeBase)
{
    return av_rescale_q(ts, *timeBase, (AVRational){ 1, 1000 });
}

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
    int64_t lastPtsMs = 0;
    int64_t totalAudioDuration = 0;
    while (demuxer.ReadPacket(&packet))
    {
        if (demuxer.GetMediaType(packet.stream_index) == AVMEDIA_TYPE_AUDIO)
        {
            AVFrame *frame = av_frame_alloc();
            while (decoder.GetDecodeFrame(packet.stream_index, frame) == 0)
            {
                int64_t ptsMs = ToMs(frame->pts, decoder.GetTimeBase(packet.stream_index));
                printf("pts: %d, duration: %d, channels: %d, sample_rate: %d,"
                       "nb_samples: %d, frame_duration: %0.2f\n", ptsMs, ptsMs - lastPtsMs,
                       frame->channels, frame->sample_rate, frame->nb_samples,
                       1.0/frame->sample_rate*frame->nb_samples*1000);
                totalAudioDuration += ptsMs - lastPtsMs;
                lastPtsMs = ptsMs;
            }

            int64_t packetDuration = ToMs(packet.duration, demuxer.GetTimeBase(packet.stream_index));
            printf("packet_duration: %d\n", packetDuration);
            decoder.DecodePacket(&packet, demuxer.GetTimeBase(packet.stream_index));
        }
    }
    printf("totalAudioDuration: %d\n", totalAudioDuration);
    return 0;
}
