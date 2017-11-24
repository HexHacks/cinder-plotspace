#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"
#include "cinder/params/Params.h"
#include "cinder/CinderMath.h"
#include "cinder/Perlin.h"

#include "LineSpace.h"
#include "AppMovieCapture.h"

#include "smaa/SMAA.h"

#include <cmath>

#include "Context.h"
#include "wavygrass/Wavygrass.h"
#include "pltspace/Pltspace.h"

using namespace ci;
using namespace ci::app;
using namespace std;


class PlotSpaceApp : public App {
    using SceneVec = std::vector<jp::SceneRef>;
    
    jp::ContextRef mCtx;
    
    SceneVec mScenes;
    int mPrevScene;
    int mActiveScene;
    
    AppMovieCaptureRef mMov;
    
    bool mShowParams;
    bool mShowFrame;
    bool mUseSmaa;
    unsigned int mRecFrames;
    
    int mStartFrame;
    
    gl::FboRef mFboScene;
    gl::FboRef mFboFinal;
    gl::TextureRef mFrameTex;
    
    jp::MovieWriter::Format mMovFormat;
    
    SMAA mSMAA;
    
  public:
    
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
    void renderScene();
    
    void prepareSettings( Settings *settings )
    {
        settings->setHighDensityDisplayEnabled();
    }
    
    void resize() override
    {
        gl::Texture2d::Format tfmt;
        tfmt.setMinFilter( GL_NEAREST );
        tfmt.setMagFilter( GL_NEAREST );
        tfmt.setInternalFormat( GL_RGBA8 );
        
        gl::Fbo::Format fmt;
        fmt.setColorTextureFormat( tfmt );
        
        mFboScene = gl::Fbo::create( getWindowWidth(), getWindowHeight(), fmt );
        mFboFinal = gl::Fbo::create( getWindowWidth(), getWindowHeight(), fmt );
        
        mCtx->screenSize.x =
        mMovFormat.width = getWindowWidth();
        
        mCtx->screenSize.y =
        mMovFormat.height = getWindowHeight();
        
        mCtx->cam.setAspectRatio(getWindowAspectRatio());
    }
    
    void resetTime()
    {
        mStartFrame = getElapsedFrames();
        mCtx->t = 0.f;
        mCtx->frame = 0;
    }
    
    void startRec()
    {
        resetTime();
        mCtx->animate = true;
        
        mMov->setAutoFinish(mRecFrames);
        mMov->start(mMovFormat);
    }
    
    void setupParams()
    {
        mCtx->params = params::InterfaceGl::create( getWindow(), "Parameters", ivec2( 200, 200 ) );
        /*mCtx->params->addParam("T", &mT)
            .min(0.0)
            .max(10.0)
            .step(0.1)
            .keyIncr("+")
            .keyDecr("-")
            .updateFn([this](){ calcSpace(); });*/
        
        
        auto changeScene = [this]()
        {
            mScenes[mPrevScene]->deactivate();
            mScenes[mActiveScene]->activate();
            mPrevScene = mActiveScene;
        };
        
        mCtx->params->addParam("Scene", &mActiveScene)
        .min(0)
        .max(mScenes.size()-1)
        .keyIncr("+")
        .keyDecr("-")
        .updateFn(changeScene);

        auto timeReset = [this]()
        {
            resetTime();
        };
        
        mCtx->params->addParam("Animate", &mCtx->animate).key("a");
        mCtx->params->addParam("SMAA", &mUseSmaa).key("s");
        mCtx->params->addSeparator();
        mCtx->params->addParam("Rec frames", &mRecFrames).updateFn(timeReset);
        mCtx->params->addButton("Record", [this](){ startRec(); }, "key=r");
        mCtx->params->addSeparator();
        mCtx->params->addParam("S/H Frame", &mShowFrame).key("f");
        mCtx->params->addParam("S/H Params", &mShowParams).key("p");
        mCtx->params->addSeparator();
    }
    
};

void PlotSpaceApp::setup()
{
    gl::enable( GL_LINE_SMOOTH );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    gl::enableDepthRead();
    gl::enableDepthWrite();
    gl::enableVerticalSync();
    //gl::disableAlphaBlending();
    
    setWindowSize(742, 742);
    
    mCtx = std::make_shared<jp::Context>();
    mCtx->animate = false;
    mCtx->cam.lookAt(vec3(0.f, 3.f, 3.f), vec3(0.f));
    mCtx->camUi = CameraUi(&mCtx->cam, getWindow());
    mCtx->screenSize = getWindowSize();
    
    mScenes = {
        jp::Pltspace::create(mCtx),
        jp::Wavygrass::create(mCtx)
    };
    
    resetTime();
    setupParams();
    
    mShowFrame = true;
    mShowParams = true;
    mUseSmaa = false;
    mRecFrames = 430;
    
    // Setup scenes after params
    for (auto s : mScenes)
        s->setup();
    
    mPrevScene = mActiveScene = mScenes.size() - 1;
    mScenes[mActiveScene]->activate();
    
    mMovFormat = jp::MovieWriter::getHighQualityFormat(AV_CODEC_ID_HEVC, 30, getWindowWidth(), getWindowHeight());
    
    mMov = AppMovieCapture::create(this);
}

void PlotSpaceApp::mouseDown( MouseEvent event )
{
}

void PlotSpaceApp::update()
{
    if (mMov->isCapturing())
    {
        auto title = (boost::format("Frame: %1% of %2%") % (mMov->getCurrentFrame() + 1) % mRecFrames).str();
        getWindow()->setTitle(title);
    }
    else
    {
        getWindow()->setTitle(mScenes[mActiveScene]->getName());
    }
    
    mCtx->frame = 0;
    mCtx->t = 0.f;
    
    if (mCtx->animate)
    {
        mCtx->frame = getElapsedFrames() - mStartFrame;
        mCtx->t = getElapsedSeconds();
        if (mMov->isCapturing())
        {
            mCtx->t = mCtx->frame / float(mMov->getFps());
        }
    }
    
    mScenes[mActiveScene]->update();
}

void PlotSpaceApp::draw()
{
    renderScene();
    
    gl::clear();
    gl::color( Color::white() );
    
    gl::FboRef fbo;
    if (mUseSmaa)
    {
        mSMAA.apply( mFboFinal, mFboScene );
        fbo = mFboFinal;
    }
    else
        fbo = mFboScene;
    
    mFrameTex = fbo->getColorTexture();
    
    {
        gl::ScopedMatrices scpMat;
        gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
        gl::draw(mFrameTex, getWindowBounds());
    }
    
    if (mMov->isCapturing())
    {
        auto surf = fbo->readPixels8u(getWindowBounds());
        //auto dir = getAssetPath("output");
        //writeImage(jp::ensureTmpOutput(dir, "png"), surf);
        mMov->addFrame(surf);
    }
}

void PlotSpaceApp::renderScene()
{
    gl::ScopedFramebuffer scpFbo( mFboScene );
    
    // Clear the buffer.
    gl::clear( ColorA(0.3, 0.3, 0.3, 1.0) );
    
    // Render our scene.
    gl::ScopedViewport scpViewport(0, 0, mFboScene->getWidth(), mFboScene->getHeight());
    gl::setMatrices(mCtx->cam);
    
    mScenes[mActiveScene]->draw();
    
    if (!mMov->isCapturing())
    {
        if (mShowFrame)
            gl::drawCoordinateFrame(1.);
        
        if (mShowParams)
            mCtx->params->draw();
    }
}


CINDER_APP( PlotSpaceApp, RendererGl )
