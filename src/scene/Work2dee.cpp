
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
#include <cmath>

using namespace ci;
using namespace cinder;

static gl::GlslProgRef xIqProg;
static gl::GlslProgRef xLineProg;
static gl::VboRef xEndpointVbo;
static gl::VboRef xDataVbo;
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

static vec2 getField(const vec2& p, float t)
{
    auto l = length(p);
    auto v = p / l;
    auto a = atan2(v.y, v.x);
    return vec2(cos(a + M_PI_4), sin(a + M_PI_4)) * 50.f - v * 60.f;
}

static void setLines(jp::ContextRef c)
{
    auto t = c->t;
    auto dt = c->dt;
    auto scr = vec2(float(c->screenSize.x), float(c->screenSize.y));
    auto hscr = vec2(scr.x / 2., scr.y / 2.);
    
    auto endpts = (vec4*)xEndpointVbo->mapWriteOnly();
    auto data = (vec4*)xDataVbo->mapReplace();
    for (int i = 0; i < cNumLines; i++)
    {
        auto& v = endpts[i];
        auto& d = data[i];
        
        auto p0 = vec2(v.x, v.y);
        auto p1 = vec2(v.z, v.w);
        
        auto f = getField(p0 - hscr, t);
        p0 += f * dt;
        
        f = getField(p1 - hscr, t);
        p1 += f * dt;
        
        auto val = 1. - length(p0) / length(scr);
        auto col = Color(ColorModel::CM_HSV, vec3(val, 0.7, val));
        d.x = col.r;
        d.y = col.g;
        d.z = col.b;
        d.w = 1.f;
        
        v.x = p0.x;
        v.y = p0.y;
        v.z = p1.x;
        v.w = p1.y;
    }
    xDataVbo->unmap();
    xEndpointVbo->unmap();
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
    
    const auto siz = sizeof(vec4);
    std::vector<vec4> endpts(cNumLines);
    std::vector<vec4> data(cNumLines);
    
    for (int i = 0; i < cNumLines; i++)
    {
        auto& e = endpts[i];
        e.x = rnd.nextFloat(scr.x);
        e.y = rnd.nextFloat(scr.y);
        
        auto dir = rnd.nextVec2();
        e.z = e.x + dir.x * 20.f;
        e.w = e.y + dir.y * 20.f;
        
        data[i] = vec4(1., 0., 0., 1.);
    }
    
    // create the VBO which will contain per-instance (rather than per-vertex) data
    xEndpointVbo = gl::Vbo::create( GL_ARRAY_BUFFER, endpts.size() * siz, endpts.data(), GL_DYNAMIC_DRAW );
    xDataVbo = gl::Vbo::create( GL_ARRAY_BUFFER, data.size() * siz, data.data(), GL_DYNAMIC_DRAW );
    
    // we need a geom::BufferLayout to describe this data as mapping to the CUSTOM_0 semantic, and the 1 (rather than 0) as the last param indicates per-instance (rather than per-vertex)
    geom::BufferLayout layout1, layout2;
    layout1.append( geom::Attrib::CUSTOM_0, 4, siz, 0, 1 /* per instance */ );
    layout2.append( geom::Attrib::CUSTOM_1, 4, siz, 0, 1 /* per instance */ );
    
    // Add vbo to mesh
    mesh->appendVbo(layout1, xEndpointVbo);
    mesh->appendVbo(layout2, xDataVbo);
    
    // and finally, build our batch, mapping our CUSTOM_0 attribute to the "vInstancePosition" GLSL vertex attribute
    mBatch = gl::Batch::create( mesh, xLineProg, {
        { geom::Attrib::CUSTOM_0, "vInstanceEndpoints" },
        { geom::Attrib::CUSTOM_1, "vInstanceData"}
    } );
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
    gl::setMatricesWindow(scr);

    gl::clearColor(Color(1.0, 1.0, 1.0));
    gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //gl::translate(hscr);
    //gl::scale(10., 10., 10.);
    //gl::color(1., 0, 0.);
    //mBatch->draw();
    mBatch->drawInstanced( cNumLines );
}

