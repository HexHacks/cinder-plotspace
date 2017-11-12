#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"
#include "cinder/params/Params.h"
#include "cinder/CinderMath.h"

#include "LineSpace.h"
#include "AppMovieCapture.h"

#include "smaa/SMAA.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class PlotSpaceApp : public App {
    CameraPersp mCam;
    CameraUi mCamUi;
    params::InterfaceGlRef mParams;
    
    LineSpaceRef mSpace;
    AppMovieCaptureRef mMov;
    
    float mT;
    bool mAnimate;
    bool mShowParams;
    bool mShowFrame;
    bool mUseSmaa;
    unsigned int mRecFrames;
    
    gl::FboRef mFboScene;
    gl::FboRef mFboFinal;
    gl::TextureRef mFrameTex;
    
    SMAA mSMAA;
    
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
    void renderScene();
    
    void resetSpace();
    void calcSpace();
    vec3 func(const ivec3& idx, const vec3& pos);
    vec4 col(const ivec3& idx, const vec3& pos);
    
    void prepareSettings( Settings *settings )
    {
        settings->setHighDensityDisplayEnabled();
        settings->setWindowSize( 700, 700 );
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
        
        mCam.setAspectRatio(getWindowAspectRatio());
    }
    
    void startRec()
    {
        // 5 secs
        mMov->setAutoFinish(mRecFrames);
        mMov->start();
    }
    
    void setupParams()
    {
        mParams = params::InterfaceGl::create( getWindow(), "Parameters", ivec2( 200, 200 ) );
        mParams->addParam("T", &mT)
            .min(0.0)
            .max(10.0)
            .step(0.1)
            .keyIncr("+")
            .keyDecr("-")
            .updateFn([this](){ calcSpace(); });

        mParams->addParam("Animate", &mAnimate).key("a");
        mParams->addParam("SMAA", &mUseSmaa).key("s");
        mParams->addSeparator();
        mParams->addParam("Rec frames", &mRecFrames);
        mParams->addButton("Record", [this](){ startRec(); }, "key=r");
        mParams->addSeparator();
        mParams->addParam("S/H Frame", &mShowFrame).key("f");
        mParams->addParam("S/H Params", &mShowParams).key("p");
        mParams->addButton("Reset", [this](){ resetSpace(); }, "key=c");
    }
    
};

void PlotSpaceApp::setup()
{
    gl::enable( GL_LINE_SMOOTH );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    gl::enableDepthRead();
    gl::enableDepthWrite();
    gl::enableVerticalSync();
    gl::disableAlphaBlending();
    
    mT = 0.;
    mAnimate = false;
    mShowFrame = true;
    mShowParams = true;
    mUseSmaa = false;
    mRecFrames = 30 * 5; // 5 secs
    
    mMov = AppMovieCapture::create(this);
    mSpace = LineSpace::create(vec3(10.), ivec3(10));
    
    mCam.lookAt(vec3(0.f, 3.f, 3.f), vec3(0.f));
    mCamUi = CameraUi(&mCam, getWindow());
    
    
    setupParams();
}

void PlotSpaceApp::mouseDown( MouseEvent event )
{
}

static vec4 v4(const vec3& v)
{
    return vec4(v.x, v.y, v.z, 1.);
}

static vec3 v3(const vec4& v)
{
    return vec3(v.x / v.w, v.y / v.w, v.z / v.w);
}

static mat4 m4(float aa, float ab, float ac, float ad,
               float ba, float bb, float bc, float bd,
               float ca, float cb, float cc, float cd,
               float da, float db, float dc, float dd)
{
    return mat4(aa, ba, ca, da,
                ab, bb, cb, db,
                ac, bc, cc, dc,
                ad, bd, cd, dd);
}

vec3 PlotSpaceApp::func(const ivec3& idx, const vec3& pos)
{
    auto T = m4(1., 0., 0., 0.,
                0., 1., 0., 0.,
                0., 0., 1., 0.,
                0.1*sin(pos.x+mT), 0.1*cos(pos.y*pos.z+mT), 0., 1.);

    
    return v3(T * v4(pos));
}

vec4 PlotSpaceApp::col(const ivec3& idx, const vec3& pos)
{
    vec4 col(1.);
    float outer = length(mSpace->getSize()) * 0.5f;
    float rad = length(pos) / outer;
    
    col.r = 1.f-rad;
    col.g = math<float>::clamp(1.f/(rad*rad+1.f), 0.f, 1.f);
    col.w = rad;
    
    return col;
}

void PlotSpaceApp::resetSpace()
{
    auto vbo = mSpace->getVbo();
    auto posIter = vbo->mapAttrib3f(geom::Attrib::POSITION);
    auto colIter = vbo->mapAttrib4f(geom::Attrib::COLOR);
    for (size_t i = 0; i < vbo->getNumVertices(); i++)
    {
        auto idx = mSpace->getIdxFromFlat(i);
        auto pos = mSpace->standardFunc(idx);
        
        *posIter = pos;
        *colIter = vec4(1.);
        
        posIter++;
        colIter++;
    }
    
    colIter.unmap();
    posIter.unmap();
}
                           
void PlotSpaceApp::calcSpace()
{
    auto vbo = mSpace->getVbo();
    auto posIter = vbo->mapAttrib3f(geom::Attrib::POSITION);
    auto colIter = vbo->mapAttrib4f(geom::Attrib::COLOR);
    for (size_t i = 0; i < vbo->getNumVertices(); i++)
    {
        auto idx = mSpace->getIdxFromFlat(i);
        auto pos = mSpace->standardFunc(idx);
        
        *posIter = func(idx, pos);
        *colIter = col(idx, pos);
        
        posIter++;
        colIter++;
    }
    
    colIter.unmap();
    posIter.unmap();
}

void PlotSpaceApp::update()
{
    if (mAnimate)
    {
        mT = 5.f*(sin(getElapsedSeconds())+1.f);
        calcSpace();
    }
}

void PlotSpaceApp::draw()
{
    renderScene();
    
    gl::clear();
    gl::color( Color::white() );
    
    if (mUseSmaa)
    {
        mSMAA.apply( mFboFinal, mFboScene );
        mFrameTex = mFboFinal->getColorTexture();
    }
    else
        mFrameTex = mFboScene->getColorTexture();
    
    {
        gl::ScopedMatrices scpMat;
        gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
        gl::draw(mFrameTex, getWindowBounds());
    }
    
    if (mMov->isCapturing())
    {
        mMov->captureFrame();
    }
}

void PlotSpaceApp::renderScene()
{
    gl::ScopedFramebuffer scpFbo( mFboScene );
    
    // Clear the buffer.
    gl::clear( ColorA(0.3, 0.3, 0.3, 1.0) );
    //gl::color( Color::gray(0.3) );
    
    // Render our scene.
    gl::ScopedViewport scpViewport(0, 0, mFboScene->getWidth(), mFboScene->getHeight());
    gl::setMatrices(mCam);
    
    mSpace->draw();
    
    if (!mMov->isCapturing())
    {
        if (mShowFrame)
            gl::drawCoordinateFrame(1.);
        
        if (mShowParams)
            mParams->draw();
    }
}


CINDER_APP( PlotSpaceApp, RendererGl )
