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
    height(523)
{}

void jp::setHighQualityFormat(MuxFormat& out, AVCodecID codec)
{
    out.videoCodec = codec;
    
    // Default stream format
    out.videoPixFmt = AV_PIX_FMT_YUV420P;
    
    out.videoOptions.clear();
    switch (codec)
    {
        case AV_CODEC_ID_H264:
        case AV_CODEC_ID_HEVC:
            out.videoOptions["preset"] = "medium";
            out.videoOptions["crt"] = "20";
            out.videoOptions["tune"] = "grain";
            break;
            
        default:
            break;
    }
}
