//
//  FMovieWriter.h
//  PlotSpace
//
//  Created by Jacob Peyron on 13/11/17.
//
//

#ifndef FMovieWriter_h
#define FMovieWriter_h

#include <memory>
#include <cstdint>
#include <boost/filesystem.hpp>

#ifdef __cplusplus
extern "C" {
#endif
    
#include <libavcodec/avcodec.h>
    
#ifdef __cplusplus
}
#endif

struct AVFormatContext;

namespace jp
{
    struct OutputStream;
    using OutputStreamRef = std::shared_ptr<OutputStream>;
    
    class MovieWriter;
    using MovieWriterRef = std::shared_ptr<MovieWriter>;
    
    class MovieWriter
    {
    public:
        // Typedefs
        using path = boost::filesystem::path;
        
        struct Format
        {
            AVCodecID videoCodec;
            AVPixelFormat pixelFormat;
            //AVCodecID audioCodec; // Not yet implemented
            
            int framesPerSecond;
            int width, height;
            
            Format() :
                videoCodec(AV_CODEC_ID_H264),
                pixelFormat(AV_PIX_FMT_BGR24),
                framesPerSecond(30),
                width(512),
                height(523)
            {}
        };
        
        static void initialize();
        
        //
        MovieWriter();
        ~MovieWriter();
        
        static MovieWriterRef create(const path& path, const Format& format);
        
        void addFrame(void* data, size_t bytes, AVPixelFormat pixelFormat);
        
        void open(const path& path, const Format& format);
        void close();
        
    private:
        
        Format mFormat;
        path mPath;
        
        AVFormatContext* mContext;
        OutputStreamRef mVideoStream;
        
        void assertOpen();
        void allocContext();
        void allocStreamCommon(OutputStreamRef stream, AVCodecID codecId);
        void allocVideoStream();
        void openVideoStream();
        void writeHeader();
        
        void cpyToFrame(void* data, size_t bytes, AVPixelFormat pixelFormat);
        void encodeFrame();
    };
}

#endif /* FMovieWriter_h */
