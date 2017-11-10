#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"
#include "cinder/params/Params.h"

#include "LineSpace.h"

using namespace ci;
using namespace ci::app;
using namespace std;

static float clamp(float v, float min, float max)
{
    if (v < min)
        return min;
    else if (v > max)
        return max;
    else
        return v;
}

class PlotSpaceApp : public App {
    CameraPersp mCam;
    CameraUi mCamUi;
    params::InterfaceGlRef mParams;
    
    LineSpace mSpace;
    
    float mT;
    bool mAnimate;
    
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
    void calcSpace();
    vec3 func(const ivec3& idx, const vec3& pos);
    vec4 col(const ivec3& idx, const vec3& pos);
    
    void prepareSettings( Settings *settings )
    {
        settings->setHighDensityDisplayEnabled();
    }
    
    void resize() override
    {
        mCam.setAspectRatio(getWindowAspectRatio());
    }
    
    void setupSpace()
    {
        mSpace.generateVbo();
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
    }
    
};

void PlotSpaceApp::setup()
{
    setWindowSize(500, 500);
    mT = 0.;
    mAnimate = false;
    
    mCam.lookAt(vec3(0.f, 3.f, 3.f), vec3(0.f));
    mCamUi = CameraUi(&mCam, getWindow());
    
    setupSpace();
    setupParams();
    
    gl::enableDepthRead();
    gl::enableDepthWrite();
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
    float outer = length(mSpace.getSize()) * 0.5f;
    float rad = length(pos) / outer;
    
    col.r = 1.f-rad;
    col.g = clamp(1.f/(rad*rad+1.f), 0.f, 1.f);
    col.w = rad;
    
    return col;
}

void PlotSpaceApp::calcSpace()
{
    auto vbo = mSpace.getVbo();
    auto posIter = vbo->mapAttrib3f(geom::Attrib::POSITION);
    auto colIter = vbo->mapAttrib4f(geom::Attrib::COLOR);
    for (size_t i = 0; i < vbo->getNumVertices(); i++)
    {
        auto idx = mSpace.getIdxFromFlat(i);
        auto pos = mSpace.standardFunc(idx);
        
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
    gl::clear( Color::gray(0.3) );
    gl::setMatrices(mCam);
    
    
    mSpace.draw();
    gl::drawCoordinateFrame(1.);
    
    mParams->draw();
}

CINDER_APP( PlotSpaceApp, RendererGl )
