//
//  FMovieWriter.cpp
//  PlotSpace
//
//  Created by Jacob Peyron on 13/11/17.
//
//

#include "MovieWriter.h"
#include <cstring>
#include <boost/format.hpp>

#ifdef __cplusplus
extern "C" {
#endif
    
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavutil/pixdesc.h>
    
#ifdef __cplusplus
}
#endif

static void throwOnCondition(bool cond, const char* msg)
{
    if (cond)
        throw std::runtime_error(msg);
}

#define THROW_ON_NULL(a, msg) throwOnCondition(a == nullptr, msg)

#define STREAM_PIX_FMT AV_PIX_FMT_YUV420P
#define TMP_PIX_FMT AV_PIX_FMT_RGB24
#define TMP_R 0
#define TMP_G 1
#define TMP_B 2
#define TMP_PIX_SIZE 3

namespace jp
{
    struct OutputStream
    {
        AVStream* st;
        AVCodecContext* enc;
        
        /* pts of the next frame that will be generated */
        int64_t next_pts;
        int samples_count;
        
        AVFrame* frame;
        AVFrame* tmp_frame;
        
        AVPacket* pkt;
        
        float t, tincr, tincr2;
        
        struct SwsContext *sws_ctx;
        struct SwrContext *swr_ctx;
    };
    
    void MovieWriter::initialize()
    {
        static bool initialized = false;
        if (!initialized)
        {
            // Initialize libavcodec, and register all codecs and formats.
            av_register_all();
            
            initialized = true;
        }
    }
    
    MovieWriter::MovieWriter() :
        mFormat(),
        mPath(),
        mContext(nullptr),
        mVideoStream(std::make_shared<OutputStream>())
    {
        initialize();
    }
    
    MovieWriter::~MovieWriter()
    {
        close();
    }
    
    MovieWriterRef MovieWriter::create(const path& path, const Format& format)
    {
        auto ret = std::make_shared<MovieWriter>();
        ret->open(path, format);
        return ret;
    }
    
    MovieWriter::Format MovieWriter::getHighQualityHEVCFormat()
    {
        Format out;
        out.videoCodec = AV_CODEC_ID_HEVC;
        out.videoOptions["crf"] = "0";
        return out;
    }
    
    void MovieWriter::encodeFrame()
    {
        auto ret = avcodec_send_frame(mVideoStream->enc, mVideoStream->frame);
        throwOnCondition(ret < 0, "Error encoding video frame");
        
        auto encoder = mVideoStream->enc;
        auto pkt = mVideoStream->pkt;
        auto stream = mVideoStream->st;
        while (ret >= 0)
        {
            ret = avcodec_receive_packet(encoder, pkt);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                return;
            else if (ret < 0)
                throw std::runtime_error("Error while encoding");
            
            // Rescale output packet timestamp values from codec to stream timebase
            av_packet_rescale_ts(pkt, encoder->time_base, stream->time_base);
            pkt->stream_index = stream->index;
            
            // Write to disk
            ret = av_interleaved_write_frame(mContext, pkt);
            throwOnCondition(ret < 0 , "Error writing frame");
            av_packet_unref(pkt);
        }
    }
    
#ifdef USE_CINDER
    
    void cpySurfaceToFrame(AVFrame* frame, const cinder::Surface& surface)
    {
        // TODO: Size checks
        auto iter = surface.getIter();
        
        int y = 0;
        while (iter.line())
        {
            int x = 0;
            while (iter.pixel())
            {
                auto x0 = x*TMP_PIX_SIZE;
                frame->data[0][y * frame->linesize[0] + x0 + TMP_R] = iter.r();
                frame->data[0][y * frame->linesize[0] + x0 + TMP_G] = iter.g();
                frame->data[0][y * frame->linesize[0] + x0 + TMP_B] = iter.b();
                
                x++;
            }
            y++;
        }
    }
    
    void MovieWriter::cpyToFrame(const cinder::Surface& surface)
    {
        auto enc = mVideoStream->enc;
        auto dstFrame = mVideoStream->frame;
        if (mVideoStream->enc->pix_fmt != TMP_PIX_FMT)
        {
            if (!mVideoStream->sws_ctx)
            {
                int okInp = sws_isSupportedInput(TMP_PIX_FMT);
                throwOnCondition(okInp < 0, "Bad conversion input pixel formet");
                
                int okOut = sws_isSupportedOutput(mVideoStream->enc->pix_fmt);
                throwOnCondition(okOut < 0, "Bad conversion output pixel format");
                
                mVideoStream->sws_ctx = sws_getContext(mFormat.width, mFormat.height, TMP_PIX_FMT,
                                                       enc->width , enc->height, enc->pix_fmt,
                                                       SWS_BICUBIC, nullptr, nullptr, nullptr);
                
                THROW_ON_NULL(mVideoStream->sws_ctx, "Could not create scaling context");
            }
            
            // Copy the surface to the temp frame
            auto srcFrame = mVideoStream->tmp_frame;
            cpySurfaceToFrame(srcFrame, surface);
            
            // Put them into the output frame
            auto ret = sws_scale(mVideoStream->sws_ctx, (const uint8_t * const *) srcFrame->data,
                      srcFrame->linesize, 0, enc->height, dstFrame->data, dstFrame->linesize);
            
            throwOnCondition(ret < 0, "Could not convert frame");
        }
        else
        {
            cpySurfaceToFrame(dstFrame, surface);
        }
        
        dstFrame->pts = mVideoStream->next_pts++;
    }
    
    void MovieWriter::addFrame(const cinder::Surface& surface)
    {
        assertOpen();
        
        auto ret = av_frame_make_writable(mVideoStream->frame);
        throwOnCondition(ret < 0, "Could not make frame writable");
        
        cpyToFrame(surface);
        encodeFrame();
    }
#endif
    
    static void closeStream(OutputStreamRef ost)
    {
        avcodec_free_context(&ost->enc);
        av_frame_free(&ost->frame);
        av_frame_free(&ost->tmp_frame);
        sws_freeContext(ost->sws_ctx);
        swr_free(&ost->swr_ctx);
        av_packet_unref(ost->pkt);
    }
    
    void MovieWriter::close()
    {
        if (!mContext)
            return;
        
        // Done before closing streams.
        av_write_trailer(mContext);
        
        closeStream(mVideoStream);
        mVideoStream = nullptr;
        
        if (!(mContext->oformat->flags & AVFMT_NOFILE))
            avio_close(mContext->pb);
        
        avformat_free_context(mContext);
        
        mContext = nullptr;
    }
    
    void MovieWriter::open(const path& path, const Format& format)
    {
        close();
        
        throwOnCondition(boost::filesystem::exists(path), "Path already exists");
        throwOnCondition(format.width % 2 != 0, "Width must be multiple of 2");
        throwOnCondition(format.height % 2 != 0, "Height must be multiple of 2");
        
        mPath = path;
        mFormat = format;
        
        allocContext();
        allocVideoStream();
        
        //av_dump_format(mContext, 0, mPath.c_str(), 1);
        
        // auto err = av_err2str(ret);
        auto ret = avio_open(&mContext->pb, mPath.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0)
            throw std::runtime_error("Could not open file.");

        writeHeader();
    }
    
    void MovieWriter::allocContext()
    {
        throwOnCondition(mContext != nullptr, "Internal error: Context in use.");
        
        // allocate the output media context
        avformat_alloc_output_context2(&mContext, nullptr, nullptr, mPath.c_str());
        if (!mContext)
        {
            // Could not deduce output format from file extension: using MPEG.
            avformat_alloc_output_context2(&mContext, nullptr, "mpeg", mPath.c_str());
        }
        
        THROW_ON_NULL(mContext, "Could not create context");
    }
    
    void MovieWriter::allocStreamCommon(OutputStreamRef stream, AVCodecID codecId)
    {
        // Find the encoder
        auto codec = avcodec_find_encoder(codecId);
        THROW_ON_NULL(codec, "Could not find encoder for id");
        
        stream->st = avformat_new_stream(mContext, nullptr);
        THROW_ON_NULL(stream->st, "Could not allocate stream");
        
        stream->st->id = mContext->nb_streams-1;
        
        auto c = avcodec_alloc_context3(codec);
        THROW_ON_NULL(c, "Could not alloc an encoding context");

        stream->enc = c;
        
        switch (codec->type)
        {
            case AVMEDIA_TYPE_AUDIO:
                c->sample_fmt  = codec->sample_fmts ? codec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
                c->bit_rate    = 64000;
                c->sample_rate = 44100;
                if (codec->supported_samplerates)
                {
                    c->sample_rate = codec->supported_samplerates[0];
                    for (int i = 0; codec->supported_samplerates[i]; i++)
                    {
                        if (codec->supported_samplerates[i] == 44100)
                            c->sample_rate = 44100;
                    }
                }
                
                c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
                c->channel_layout = AV_CH_LAYOUT_STEREO;
                if (codec->channel_layouts)
                {
                    c->channel_layout = codec->channel_layouts[0];
                    for (int i = 0; codec->channel_layouts[i]; i++)
                    {
                        if (codec->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
                            c->channel_layout = AV_CH_LAYOUT_STEREO;
                    }
                }
                c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
                stream->st->time_base = (AVRational){ 1, c->sample_rate };
                break;
                
            case AVMEDIA_TYPE_VIDEO:
                c->codec_id = codecId;
                
                c->bit_rate = 400000;
                // Resolution must be a multiple of two.
                c->width    = mFormat.width;
                c->height   = mFormat.height;
                
                /* timebase: This is the fundamental unit of time (in seconds) in terms
                 * of which frame timestamps are represented. For fixed-fps content,
                 * timebase should be 1/framerate and timestamp increments should be
                 * identical to 1. */
                stream->st->time_base = (AVRational){ 1, mFormat.framesPerSecond };
                c->time_base = stream->st->time_base;
                
                c->gop_size = 12; // emit one intra frame every twelve frames at most
                c->pix_fmt = STREAM_PIX_FMT;
                if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO)
                {
                    // Just for testing, we also add B-frames
                    c->max_b_frames = 2;
                }
                if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO)
                {
                    /* Needed to avoid using macroblocks in which some coeffs overflow.
                     * This does not happen with normal video, it just happens here as
                     * the motion of the chroma plane does not match the luma plane. */
                    c->mb_decision = 2;
                }
                break;
                
            default:
                break;
        }
        
        // Some formats want stream headers to be separate.
        if (mContext->oformat->flags & AVFMT_GLOBALHEADER)
            c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    
    static AVFrame* allocFrame(enum AVPixelFormat pix_fmt, int width, int height)
    {
        auto picture = av_frame_alloc();
        if (!picture)
            return NULL;
        
        picture->format = pix_fmt;
        picture->width  = width;
        picture->height = height;

        // Allocate the buffers for the frame data
        auto ret = av_frame_get_buffer(picture, 32);
        throwOnCondition(ret < 0, "Could not allocate frame data");

        return picture;
    }
    
    void MovieWriter::allocVideoStream()
    {
        allocStreamCommon(mVideoStream, mFormat.videoCodec);
        
        auto c = mVideoStream->enc;
        auto codec = c->codec;
        
        AVDictionary* opt = nullptr;
        for (const auto& kv : mFormat.videoOptions)
        {
            av_dict_set(&opt, kv.first.c_str(), kv.second.c_str(), 0);
        }
        
        auto ret = avcodec_open2(c, codec, &opt);
        throwOnCondition(ret < 0, "Could not open video codec");
        
        // Allocate and init a re-usable frame
        mVideoStream->frame = allocFrame(c->pix_fmt, c->width, c->height);
        THROW_ON_NULL(mVideoStream->frame, "Could not allocate video frame");
        
        /* If the output format is not the same as the user supplied, then a temporary
         * picture is needed too. It is then converted to the required
         * output format. */
        mVideoStream->tmp_frame = NULL;
        if (c->pix_fmt != TMP_PIX_FMT)
        {
            mVideoStream->tmp_frame = allocFrame(TMP_PIX_FMT, c->width, c->height);
            THROW_ON_NULL(mVideoStream->tmp_frame, "Could not allocate temporary image");
        }
        
        // Copy the stream parameters to the muxer
        ret = avcodec_parameters_from_context(mVideoStream->st->codecpar, c);
        throwOnCondition(ret < 0, "Could not copy the stream parameters");
        
        // Alloc packet
        mVideoStream->pkt = av_packet_alloc();
        THROW_ON_NULL(mVideoStream->pkt, "Could not create packet");
        
        mVideoStream->next_pts = 0;
    }
    
    void MovieWriter::writeHeader()
    {
        auto ret = avformat_write_header(mContext, nullptr);
        if (ret < 0)
            throw std::runtime_error("Internal error: could not write header.");
    }

    void MovieWriter::assertOpen()
    {
        if (!mContext)
            throw std::runtime_error("File not open");
    }
}
