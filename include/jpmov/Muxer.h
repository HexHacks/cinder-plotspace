//
//  Muxer.h
//  PlotSpace
//
//  Created by Jacob Peyron on 13/11/17.
//
//

#ifndef Muxer_h
#define Muxer_h

#include <memory>
#include <cstdint>
#include <map>
#include <string>
#include <boost/filesystem.hpp>
#include "Format.h"

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
    
    class Muxer;
    using MuxerRef = std::shared_ptr<Muxer>;
    
    class Muxer
    {
    public:
        // Typedefs
        using path = boost::filesystem::path;
        
        static void initialize();
        
        Muxer();
        ~Muxer();
        
        static MuxerRef create(const path& path, const MuxFormat& format);
        
        //void addFrame(void* data, size_t bytes, AVPixelFormat pixelFormat);
        
#ifdef USE_CINDER
        void addFrame(const cinder::Surface& surface);
#endif
        
        void open(const path& path, const MuxFormat& format);
        void close();
        
    private:
        
        MuxFormat mFormat;
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

#endif /* Muxer_h */
