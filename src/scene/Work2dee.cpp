
//
//  {0}.h
//  PlotSpace
//
//  Created by Jacob Peyron on 2017-12-04.
//
//

#include "scene/Work2dee.h"
#include "cinder/app/AppBase.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "JUtil.h"

using namespace ci;
using namespace cinder;

static gl::GlslProgRef xIqProg;
static gl::GlslProgRef xLineProg;
static gl::VboRef mInstanceDataVbo;
static gl::BatchRef mBatch;

const auto cNumLines = 100;
const auto cLineThickness = 3.f;

jp::Work2dee::Work2dee(ContextRef ctx) :
    Scene(ctx)
{
}
jp::Work2dee::~Work2dee()
{
}

static void setLines(jp::ContextRef c)
{
    auto t = c->t;
    auto scr = c->screenSize;
    auto hscr = scr / 2;
    
    /*auto p = (vec4*)mInstanceDataVbo->mapReplace();
    for (int i = 0; i < cNumLines; i++)
    {
        auto& v = p[i];
        
        auto c = 50.f*cos(t);
        auto s = 50.f*sin(t);
        v.x = hscr.x;
        v.y = hscr.y;
        
        v.z = hscr.x + c;
        v.w = hscr.y + s;
    }
    mInstanceDataVbo->unmap();*/
}

void jp::Work2dee::setup()
{
    //xIqProg = loadShader(app::getAssetPath("work2dee"), "iq");
    xLineProg = loadShader(app::getAssetPath("work2dee"), "line");
    
    Rectf rect(0.f, -cLineThickness, 1.f, cLineThickness);
    vec2 ul(0., 0.);
    vec2 ur(1., 0.);
    vec2 ll(0., 1.);
    vec2 lr(1., 1.);
    auto geomRect = geom::Rect().rect(rect).texCoords(ul, ur, lr, ll);
    gl::VboMeshRef mesh = gl::VboMesh::create(geomRect);
    
    const auto scr = mCtx->screenSize;
    ci::Rand rnd;
    
    const auto dims = 4;
    const auto siz = sizeof(vec4);
    std::vector<vec4> endpts(cNumLines);
    
    for (int i = 0; i < cNumLines; i++)
    {
        auto& e = endpts[i];
        e.x = rnd.nextFloat(scr.x);
        e.y = rnd.nextFloat(scr.y);
        
        auto dir = rnd.nextVec2();
        e.z = e.x + dir.x * 20.f;
        e.w = e.y + dir.y * 20.f;
    }
    
    // create the VBO which will contain per-instance (rather than per-vertex) data
    mInstanceDataVbo = gl::Vbo::create( GL_ARRAY_BUFFER, endpts.size() * siz, endpts.data(), GL_DYNAMIC_DRAW );
    
    // we need a geom::BufferLayout to describe this data as mapping to the CUSTOM_0 semantic, and the 1 (rather than 0) as the last param indicates per-instance (rather than per-vertex)
    geom::BufferLayout instanceDataLayout;
    instanceDataLayout.append( geom::Attrib::CUSTOM_0, dims, siz, 0, 1 /* per instance */ );
    
    // now add it to the VboMesh we already made of the Teapot
    mesh->appendVbo( instanceDataLayout, mInstanceDataVbo );
    
    // and finally, build our batch, mapping our CUSTOM_0 attribute to the "vInstancePosition" GLSL vertex attribute
    mBatch = gl::Batch::create( mesh, xLineProg, { { geom::Attrib::CUSTOM_0, "vInstanceEndpoints" } } );
    //auto lambert = gl::ShaderDef().lambert().color();
    //gl::GlslProgRef shader = gl::getStockShader( lambert );
    //mBatch = gl::Batch::create(geom::Rect(), shader);
}
void jp::Work2dee::activate()
{
    mCtx->muxFormat.videoOptions["tune"] = "film";
}
void jp::Work2dee::deactivate()
{
    
}

void jp::Work2dee::update()
{
    auto t = mCtx->t;
    
    setLines(mCtx);

    if (mCtx->animate)
    {
    }
}
void jp::Work2dee::draw()
{
    gl::ScopedMatrices scpMat;
    gl::ScopedColor scpCol;
    gl::ScopedDepth scpDpth(false);
    
    auto scr = mCtx->screenSize;
    auto hscr = scr / 2;
    
    gl::setMatricesWindow(scr);

    gl::clearColor(Color(1.0, 1.0, 1.0));
    gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //gl::translate(hscr);
    //gl::scale(10., 10., 10.);
    //gl::color(1., 0, 0.);
    //mBatch->draw();
    mBatch->drawInstanced( cNumLines );
}

