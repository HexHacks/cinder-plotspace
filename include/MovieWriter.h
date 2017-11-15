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
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>


namespace jp
{
    class FMovieWriter;
    using FMovieWriterRef = std::shared_ptr<FMovieWriter>;
    
    class FMovieWriter
    {
        struct OutputStream;
        using OutputStreamRef = std::unique_ptr<OutputStream>;
        
        OutputStreamRef mVideoStream;
        OutputStreamRef mAudioStream;
        AVFormatContext* mContext;
    public:
        static void initialize();
        
        FMovieWriter();
        ~FMovieWriter();
        
        
    };
}

#endif /* FMovieWriter_h */
