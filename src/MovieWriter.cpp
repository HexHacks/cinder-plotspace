//
//  FMovieWriter.cpp
//  PlotSpace
//
//  Created by Jacob Peyron on 13/11/17.
//
//

#include "FMovieWriter.h"


namespace jp
{
    // A wrapper around a single output AVStream
    struct FMovieWriter::OutputStream
    {
        AVStream *st;
        AVCodecContext *enc;
        
        /* pts of the next frame that will be generated */
        int64_t next_pts;
        int samples_count;
        
        AVFrame *frame;
        AVFrame *tmp_frame;
        
        float t, tincr, tincr2;
        
        struct SwsContext *sws_ctx;
        struct SwrContext *swr_ctx;
    };
    
    FMovieWriter::FMovieWriter()
    {
        
    }
    
    FMovieWriter::~FMovieWriter()
    {
        
    }
    
    void FMovieWriter::initialize()
    {
        static bool initialized = false;
        if (!initialized)
        {
            
            initialized = true;
        }
    }
}
