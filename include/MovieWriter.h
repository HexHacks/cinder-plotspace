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
#include <map>
#include <string>
#include <boost/filesystem.hpp>

#ifdef __cplusplus
extern "C" {
#endif
    
#include <libavcodec/avcodec.h>
    
#ifdef __cplusplus
}
#endif

// Remove this if cinder is not available
#define USE_CINDER

#ifdef USE_CINDER
#include <cinder/Surface.h>
#endif

struct AVFormatContext;
struct AVFrame;

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
        using Options = std::map<std::string, std::string>;
        
        struct Format
        {
            AVCodecID videoCodec;
            //AVCodecID audioCodec; // Not yet implemented
            
            int framesPerSecond;
            int width, height;
            Options videoOptions;
            
            Format() :
                videoCodec(AV_CODEC_ID_HEVC),
                framesPerSecond(30),
                width(512),
                height(523)
            {}
        };
        
        static void initialize();
        
        //
        MovieWriter();
        ~MovieWriter();
        
        static Format getHighQualityFormat(AVCodecID codec, int fps = 30, int width = 512, int height = 512);
        
        static MovieWriterRef create(const path& path, const Format& format);
        
        //void addFrame(void* data, size_t bytes, AVPixelFormat pixelFormat);
        
#ifdef USE_CINDER
        void addFrame(const cinder::Surface& surface);
#endif
        
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
        AVFrame* allocFrame(enum AVPixelFormat pix_fmt, int width, int height);
        void openVideoStream();
        void writeHeader();
        
#ifdef USE_CINDER
        void cpyToFrame(const cinder::Surface& surface);
#endif
        void encodeFrame(OutputStreamRef stream);
        void flushEncoder(OutputStreamRef stream);
        
        void writeFrame(OutputStreamRef stream);
    };
}

#endif /* FMovieWriter_h */
