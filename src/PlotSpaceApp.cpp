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
    vec3 mSize;
    vec3 mDivs;
    
    Perlin mPerlin;
    
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
    
    void resetSpace();
    void calcSpace();
    vec3 func(const ivec3& idx, const vec3& pos);
    vec4 col(const ivec3& idx, const vec3& pos, const vec3& npos);
    
    void prepareSettings( Settings *settings )
    {
        settings->setHighDensityDisplayEnabled();
        settings->setWindowSize( 1024, 1024 );
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
        
        mMovFormat.width = getWindowWidth();
        mMovFormat.height = getWindowHeight();
        
        mCam.setAspectRatio(getWindowAspectRatio());
    }
    
    void startRec()
    {
        mMov->setAutoFinish(mRecFrames);
        mMov->start(mMovFormat);
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
    gl::enableAlphaBlending();
    //gl::disableAlphaBlending();
    
    setWindowSize(1080, 712);
    
    mT = 0.;
    mAnimate = false;
    mShowFrame = true;
    mShowParams = true;
    mUseSmaa = false;
    mRecFrames = 50;
    mSize = vec3(10.f);
    mDivs = ivec3(20);
    
    mMovFormat = jp::MovieWriter::getHighQualityFormat(AV_CODEC_ID_HEVC, 30, getWindowWidth(), getWindowHeight());
    
    mMov = AppMovieCapture::create(this);
    
    mSpace = LineSpace::create(mSize, vec3(mDivs));
    
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
    auto a = 0.3;
    auto f = 0.1;
    auto T = m4(1., 0., 0., 0.,
                0., 1., 0., 0.,
                0., 0., 1., 0.,
                a*sin(f*pos.x+f*mT), a*cos(f*pos.y*pos.z+f*mT), a*0.1*sin(f*mT), 1.);

    
    return v3(T * v4(pos));
}

inline float sn(float x)
{
    return sin(x) * 0.5 + 0.5;
}

inline float d(float x)
{
    auto s = sn(x);
    return 1. - s*s;
}

inline float dd(float x)
{
    auto s = sn(x);
    return s*s;
}

inline float sigmoid(float x)
{
    float xp = exp(x);
    return xp / (xp + 1.f);
}

vec4 PlotSpaceApp::col(const ivec3& idx, const vec3& pos, const vec3& npos)
{
    float lx = npos.x - pos.x;
    float ly = npos.y - pos.y;
    float lz = npos.z - pos.z;
    float sx = 0.7f * sigmoid(lx * 0.5f) + 0.3;
    float sy = 0.5f * sigmoid(ly * 0.5f) + 0.5;
    float sz = 0.8f * sigmoid(lz * 0.5f) + 0.2;
    
    return vec4(sx, sy, sz, 1.f);
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
        
        auto f = func(idx, pos);
        *posIter = f;
        *colIter = col(idx, pos, f);
        
        posIter++;
        colIter++;
    }
    
    colIter.unmap();
    posIter.unmap();
}

static vec3 cone(float t, float diam, float length, float x)
{
    auto at = (sin(t) * 0.5 + 0.5);
    auto al = at * length;
    auto amp = diam * (1.f-at) / length;
    return vec3(x + al, amp*cos(t), amp*sin(t));
}

void PlotSpaceApp::update()
{
    if (mMov->isCapturing())
    {
        auto title = (boost::format("Frame: %1% of %2%") % (mMov->getCurrentFrame() + 1) % mRecFrames).str();
        getWindow()->setTitle(title);
    }
    
    if (mAnimate)
    {
        auto s = getElapsedFrames() / float(mMov->getFps());
        mT = 5.f*(sin(s)+1.f);
        
        auto eye = cone(s*0.5, 4., 4., 7.);
        auto at = cone(s*0.08-s, 2., 3., 0.);
        mCam.lookAt(eye, at);
        
        calcSpace();
    }
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
