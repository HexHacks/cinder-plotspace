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
        
        AVCodecID videoCodec;
        AVPixelFormat videoPixFmt;
        //AVCodecID audioCodec; // Not yet implemented
        
        int framesPerSecond;
        int width, height;
        Options videoOptions;
    };
    
    void setHighQualityFormat(MuxFormat& out, AVCodecID codec);
}

#endif /* Format_h */
