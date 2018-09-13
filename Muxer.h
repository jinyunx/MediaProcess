//
// Created by onexie on 2018/9/13.
//

#ifndef MEDIAPROCESS_MUXER_H
#define MEDIAPROCESS_MUXER_H


#include <libavformat/avformat.h>
#include <string>
#include <vector>

class Muxer
{
public:
    Muxer()
        : m_formatContext(nullptr)
    {
    }

    ~Muxer()
    {
        if (m_formatContext)
            av_write_trailer(m_formatContext);

        if (m_formatContext && !(m_formatContext->oformat->flags & AVFMT_NOFILE))
            avio_closep(&m_formatContext->pb);
        avformat_free_context(m_formatContext);
    }

    bool Init(AVFormatContext &inFormatContext,
              std::vector<AVCodecContext *> &codecCtx,
              const std::string &fileName)
    {
        return OpenMuxingFile(inFormatContext, codecCtx, fileName);
    }

    bool WriteFrame(AVPacket *packet, const AVRational *timeBase)
    {
        av_packet_rescale_ts(
            packet, *timeBase,
            m_formatContext->streams[packet->stream_index]->time_base);
        return av_interleaved_write_frame(m_formatContext, packet) == 0;
    }

private:
    bool OpenMuxingFile(AVFormatContext &inFormatContext,
                        std::vector<AVCodecContext *> &codecCtx,
                        const std::string &fileName)
    {
        if (inFormatContext.nb_streams != codecCtx.size() &&
            !codecCtx.empty())
        {
            av_log(nullptr, AV_LOG_ERROR,
                   "inFormatContext.nb_streams != codecCtx.size() &&"
                   "!codecCtx.empty()\n");
            return false;
        }

        avformat_alloc_output_context2(&m_formatContext, nullptr,
                                       nullptr, fileName.c_str());
        if (!m_formatContext)
        {
            av_log(nullptr, AV_LOG_ERROR,
                   "Could not create output context\n");
            return false;
        }

        for (int i = 0; i < inFormatContext.nb_streams; ++i)
        {
            AVStream *newStream = avformat_new_stream(m_formatContext, nullptr);
            if (!newStream)
            {
                av_log(nullptr, AV_LOG_ERROR, "Failed allocating output stream\n");
                return false;
            }
            if (!codecCtx.empty())
            {
                if (avcodec_parameters_from_context(
                    newStream->codecpar, codecCtx[i]) < 0)
                {
                    av_log(nullptr, AV_LOG_ERROR,
                           "Failed to copy encoder "
                           "parameters to output stream #%u\n", i);
                    return false;
                }
                newStream->time_base = codecCtx[i]->time_base;
            }
            else
            {
                if (avcodec_parameters_copy(
                    newStream->codecpar,
                    inFormatContext.streams[i]->codecpar) < 0)
                {
                    av_log(nullptr, AV_LOG_ERROR,
                           "Copying parameters for stream #%u failed\n", i);
                    return false;
                }
                newStream->time_base = inFormatContext.streams[i]->time_base;
            }
        }
        av_dump_format(m_formatContext, 0, fileName.c_str(), 1);

        if (!(m_formatContext->oformat->flags & AVFMT_NOFILE))
        {
            if (avio_open(&m_formatContext->pb, fileName.c_str(),
                          AVIO_FLAG_WRITE) < 0)
            {
                av_log(nullptr, AV_LOG_ERROR,
                       "Could not open output file '%s'", fileName.c_str());
                return false;
            }
        }

        if (avformat_write_header(m_formatContext, nullptr) < 0)
        {
            av_log(nullptr, AV_LOG_ERROR,
                   "Error occurred when opening output file\n");
            return false;
        }
        return true;
    }

    AVFormatContext *m_formatContext;
};

#endif //MEDIAPROCESS_MUXER_H
