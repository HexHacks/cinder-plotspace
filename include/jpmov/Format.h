//
//  Format.h
//  PlotSpace
//
//  Created by Jacob Peyron on 24/11/17.
//
//

#ifndef Format_h
#define Format_h

#include <map>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
    
#ifdef __cplusplus
}
#endif

namespace jp
{
    using Options = std::map<std::string, std::string>;
    
    struct MuxFormat
    {
        MuxFormat();
        void setHighQualityVideoOptions(AVCodecID codec);
        void setBitrateMB(int frameCount, int fileSizeMB);
        
        AVCodecID videoCodec;
        AVPixelFormat videoPixFmt;
        //AVCodecID audioCodec; // Not yet implemented
        
        int framesPerSecond;
        int width, height;
        int bitRate;
        Options videoOptions;
    };
}

#endif /* Format_h */
