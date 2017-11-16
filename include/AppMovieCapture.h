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
#include "MovieWriter.h"

#ifndef AppMovieCapture_h
#define AppMovieCapture_h

class AppMovieCapture;
using AppMovieCaptureRef = std::shared_ptr<AppMovieCapture>;

class AppMovieCapture
{
    using MovieWriter = ::jp::MovieWriter;
    using MovieWriterRef = ::jp::MovieWriterRef;
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
    
    AppMovieCapture(App* app): mMov(nullptr), mApp(app), mAutoFinish(-1), mFrame(0), mRenewPath(true)
    {
        mPath = ensureTmpPath();
    }
    
    // Generates a temp path that automatically is used.
    static AppMovieCaptureRef create(App* app)
    {
        return std::make_shared<AppMovieCapture>(app);
    }
    
    path ensureTmpPath(const std::string ext = "mkv") const
    {
        auto fold = mApp->getAssetPath("");
        auto outd = fold / "output";
        auto o = outd / (boost::format("capture_0.%1%") % ext).str();
        if (!::ci::fs::exists(outd))
        {
            ::ci::fs::create_directory(outd);
        }
        else if (::ci::fs::is_directory(outd))
        {
            int i = 0;
            while (::ci::fs::exists(o))
            {
                o = outd / (boost::format("capture_%1%.%2%") % i++ % ext).str();
            }
        }
        else
        {
            throw std::exception();
        }
        
        return o;
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
        mFrame = 0;
        if (mRenewPath)
            mPath = ensureTmpPath();
        
        mFormat.width = mApp->getWindowWidth();
        mFormat.height = mApp->getWindowHeight();
        mMov = MovieWriter::create(mPath, mFormat);
    }
    
    void captureFrame()
    {
        if (mMov)
        {
            mMov->addFrame(mApp->copyWindowSurface());
            
            mFrame++;
            if (mFrame >= mAutoFinish)
                finish();
        }
    }
    
    void finish()
    {
        mMov->close();
        mMov = nullptr;
    }
};

#endif /* AppMovieCapture_h */
