//
// Created by onexie on 2018/9/11.
//

#ifndef MEDIAPROCESS_DEMUXER_H
#define MEDIAPROCESS_DEMUXER_H

extern "C"
{
#include <libavformat/avformat.h>
};

#include <string>

class Demuxer
{
public:
    Demuxer()
        : m_formatContext(nullptr)
    {
    }

    ~Demuxer()
    {
        if (m_formatContext)
            avformat_close_input(&m_formatContext);
    }

    bool Init(const std::string &fileName)
    {
        return OpenDemuxingFile(fileName.c_str()) == 0;
    }

    AVFormatContext *GetFormatContext() const
    {
        return m_formatContext;
    }

    bool ReadPacket(AVPacket *packet)
    {
        return av_read_frame(m_formatContext, packet) == 0;
    }

    AVMediaType GetMediaType(int streamIndex) const
    {
        return m_formatContext->streams[streamIndex]->codecpar->codec_type;
    }

    const AVCodecParameters *GetCodecPram(int streamIndex) const
    {
        return m_formatContext->streams[streamIndex]->codecpar;
    }

    const AVRational *GetTimeBase(int streamIndex)
    {
        return &m_formatContext->streams[streamIndex]->time_base;
    }

    Demuxer(const Demuxer &) = delete;
    void operator = (const Demuxer &) = delete;

private:
    int OpenDemuxingFile(const char *fileName)
    {
        int ret = 0;
        if ((ret = avformat_open_input(&m_formatContext, fileName, nullptr, nullptr)) < 0)
        {
            av_log(nullptr, AV_LOG_ERROR, "Cannot open input file, %s\n",
                    fileName);
            return ret;
        }

        if ((ret = avformat_find_stream_info(m_formatContext, nullptr)) < 0)
        {
            av_log(nullptr, AV_LOG_ERROR, "Cannot find stream information\n");
            return ret;
        }

        av_dump_format(m_formatContext, 0, fileName, 0);
        return 0;
    }

    AVFormatContext *m_formatContext;
};

#endif //MEDIAPROCESS_DEMUXER_H
