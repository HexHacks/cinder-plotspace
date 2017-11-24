//
//  AppMovieCapture.h
//  PlotSpace
//
//  Created by Jacob Peyron on 11/11/17.
//
//

#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <exception>
#include "cinder/app/App.h"
#include "jpmov/Muxer.h"
#include "JUtil.h"

#ifndef AppMovieCapture_h
#define AppMovieCapture_h

class AppMovieCapture;
using AppMovieCaptureRef = std::shared_ptr<AppMovieCapture>;

class AppMovieCapture
{
    using MovieWriter = ::jp::Muxer;
    using MovieWriterRef = ::jp::MuxerRef;
    using App = ::ci::app::App;
    using path = ::ci::fs::path;
    
    MovieWriter::Format mFormat;
    MovieWriterRef mMov;
    path mPath;
    App* mApp;
    int mAutoFinish;
    int mFrame;
    bool mRenewPath;
    
public:
    
    AppMovieCapture(App* app):
        mMov(nullptr),
        mApp(app),
        mAutoFinish(-1),
        mFrame(0),
        mRenewPath(true),
        mFormat()
    {
        mPath = ensureTmpPath();
    }
    
    // Generates a temp path that automatically is used.
    static AppMovieCaptureRef create(App* app)
    {
        return std::make_shared<AppMovieCapture>(app);
    }
    
    path ensureTmpPath(const std::string& ext = "mkv") const
    {
        auto fold = mApp->getAssetPath("");
        auto outd = fold / "output";
        
        return jp::ensureTmpOutput(outd, ext);
    }
    
    /////// The internal state only effects next start
    
    void setPath(const path& path)
    {
        mPath = path;
    }
    
    path getPath() const
    {
        return mPath;
    }
    
    void setFps(int fps)
    {
        mFormat.framesPerSecond = fps;
    }
    
    int getFps() const
    {
        return mFormat.framesPerSecond;
    }
    
    void setAutoFinish(int af)
    {
        mAutoFinish = af;
    }
    
    int getAutoFinish() const
    {
        return mAutoFinish;
    }
    
    int getCurrentFrame() const
    {
        return mFrame;
    }
    
    void setRenewPath(bool renew)
    {
        mRenewPath = renew;
    }
    
    bool getRenewPath() const
    {
        return mRenewPath;
    }
    
    bool isCapturing() const
    {
        return mMov != nullptr;
    }
    
    void start()
    {
        mFormat.width = mApp->getWindowWidth();
        mFormat.height = mApp->getWindowHeight();
        start(mFormat);
    }
    
    void start(const MovieWriter::Format& format)
    {
        mFrame = 0;
        if (mRenewPath)
            mPath = ensureTmpPath();
        
        mMov = MovieWriter::create(mPath, format);
    }
    
    void addFrame(const ::ci::Surface& surface)
    {
        if (mMov)
        {
            mMov->addFrame(surface);
            
            mFrame++;
            if (mFrame >= mAutoFinish)
                finish();
        }
    }
    
    void captureFrame()
    {
        if (mMov)
        {
            addFrame(mApp->copyWindowSurface());
        }
    }
    
    void finish()
    {
        mMov->close();
        mMov = nullptr;
    }
};

#endif /* AppMovieCapture_h */
