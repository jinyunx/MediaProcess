//
// Created by onexie on 2018/9/11.
//

#ifndef MEDIAPROCESS_DECODER_H
#define MEDIAPROCESS_DECODER_H

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
};

#include <vector>

class Decoder
{
public:
    Decoder() = default;

    ~Decoder()
    {
        for (auto &ctx : m_decCtxVec)
            avcodec_free_context(&ctx);
    }

    bool Init(AVFormatContext &formatContext)
    {
        return PrepareDecodeStreamCtx(formatContext) == 0;
    }

    Decoder(const Decoder &) = delete;
    void operator = (const Decoder &) = delete;

private:
    int PrepareDecodeStreamCtx(AVFormatContext &formatContext)
    {
        // Per stream per decoder
        for (int i = 0; i < formatContext.nb_streams; ++i)
        {
            AVStream *stream = formatContext.streams[i];

            // Find decoder
            AVCodec *dec = avcodec_find_decoder(stream->codecpar->codec_id);
            if (!dec)
            {
                av_log(nullptr, AV_LOG_ERROR,
                       "Failed to find decoder for stream #%u\n", i);
                return AVERROR_DECODER_NOT_FOUND;
            }

            // Alloc decode context
            AVCodecContext *codecCtx = avcodec_alloc_context3(dec);
            if (!codecCtx)
            {
                av_log(nullptr, AV_LOG_ERROR, "Failed to allocate the "
                       "decoder context for stream #%u\n", i);
                return AVERROR(ENOMEM);
            }

            // Copy decoder param
            int ret = avcodec_parameters_to_context(codecCtx, stream->codecpar);
            if (ret < 0)
            {
                av_log(nullptr, AV_LOG_ERROR, "Failed to copy decoder parameters"
                       " to input decoder context for stream #%u\n", i);
                return ret;
            }

            // Reencode video & audio and remux subtitles etc
            if (codecCtx->codec_type == AVMEDIA_TYPE_VIDEO ||
                codecCtx->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                if (codecCtx->codec_type == AVMEDIA_TYPE_VIDEO)
                {
                    codecCtx->framerate = av_guess_frame_rate(
                            &formatContext, stream, nullptr);
                }

                // Open decoder
                ret = avcodec_open2(codecCtx, dec, nullptr);
                if (ret < 0)
                {
                    av_log(nullptr, AV_LOG_ERROR, "Failed to open"
                           " decoder for stream #%u\n", i);
                    return ret;
                }
            }

            m_decCtxVec.push_back(codecCtx);
        }
        return 0;
    }

    std::vector<AVCodecContext *> m_decCtxVec;
};

#endif //MEDIAPROCESS_DECODER_H
