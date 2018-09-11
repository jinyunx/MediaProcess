#include "Demuxer.h"
#include "Decoder.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("usage: %s infile", argv[0]);
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

    return 0;
}
