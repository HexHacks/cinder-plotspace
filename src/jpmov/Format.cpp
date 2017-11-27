//
//  Format.cpp
//  PlotSpace
//
//  Created by Jacob Peyron on 24/11/17.
//
//

#include "jpmov/Format.h"

jp::MuxFormat::MuxFormat() :
    videoCodec(AV_CODEC_ID_HEVC),
    videoPixFmt(AV_PIX_FMT_YUV444P),
    framesPerSecond(30),
    width(512),
    height(523),
    bitRate(24960000)
{}

void jp::MuxFormat::setHighQualityVideoOptions(AVCodecID codec)
{
    videoCodec = codec;
    
    // Default stream format
    videoPixFmt = AV_PIX_FMT_YUV420P;
    
    videoOptions.clear();
    switch (codec)
    {
        case AV_CODEC_ID_H264:
            // qmin 50 an qmax 51 gives the lowest quality
            // qmin 0 -qmax 1 gives the highest quality
            
            // 16 - 26 (120mb)
            // 24 - 30 (30Mbish)
            
            videoOptions["profile"] = "high";
        case AV_CODEC_ID_HEVC:
            videoOptions["tune"] = "film";
            videoOptions["preset"] = "veryslow";
            videoOptions["qmin"] = "22";
            videoOptions["qmax"] = "28";
            //videoOptions["crt"] = "20";
            //videoOptions["tune"] = "grain";
            break;
            
        default:
            break;
    }
}

void jp::MuxFormat::setBitrateMB(int frameCount, int fileSizeMB)
{
    int duration = frameCount / framesPerSecond;
    bitRate = (fileSizeMB * 8192/*to_kBit*/) / duration;
}
