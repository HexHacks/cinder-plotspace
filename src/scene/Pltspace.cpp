//
//  Pltspace.cpp
//  PlotSpace
//
//  Created by Jacob Peyron on 23/11/17.
//
//

#include "scene/Pltspace.h"
#include "LineSpace.h"
#include "JUtil.h"

using namespace ci;
using namespace glm;

namespace jp
{
    class PltspaceImpl
    {
        ContextRef mCtx;
        
        LineSpaceRef mSpace;
        vec3 mSize;
        vec3 mDivs;
        
        void calcSpace();
        void resetSpace();
        
    public:
        PltspaceImpl(ContextRef ctx) : mCtx(ctx) {}
        
        void setup();
        void update();
        void draw();
    };
    
    Pltspace::Pltspace(ContextRef ctx) :
        Scene(ctx),
        mImpl(new PltspaceImpl(ctx))
    {
    }
    
    Pltspace::~Pltspace()
    {
        delete mImpl;
    }
    
    void Pltspace::setup()
    {
        mImpl->setup();
    }
    void Pltspace::update()
    {
        mImpl->update();
    }
    void Pltspace::activate()
    {
        gl::enable( GL_LINE_SMOOTH );
        glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
        
        mCtx->muxFormat.videoOptions["tune"] = "grain";
    }
    void Pltspace::deactivate()
    {
        
    }
    void Pltspace::draw()
    {
        mImpl->draw();
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
    
    static vec3 func(float t, const ivec3& idx, const vec3& pos)
    {
        auto a = 0.3;
        auto f = 0.1;
        auto T = m4(1., 0., 0., 0.,
                    0., 1., 0., 0.,
                    0., 0., 1., 0.,
                    a*sin(f*pos.x+f*t), a*cos(f*pos.y*pos.z+f*t), a*0.1*sin(f*t), 1.);
        
        
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
    
    inline static vec4 col(float t, const ivec3& idx, const vec3& pos, const vec3& npos)
    {
        float lx = npos.x - pos.x;
        float ly = npos.y - pos.y;
        float lz = npos.z - pos.z;
        float sx = 0.7f * sigmoid(lx * 0.5f) + 0.3;
        float sy = 0.5f * sigmoid(ly * 0.5f) + 0.5;
        float sz = 0.8f * sigmoid(lz * 0.5f) + 0.2;
        
        return vec4(sx, sy, sz, 1.f);
    }
    
    void PltspaceImpl::resetSpace()
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
    
    void PltspaceImpl::calcSpace()
    {
        auto vbo = mSpace->getVbo();
        auto posIter = vbo->mapAttrib3f(geom::Attrib::POSITION);
        auto colIter = vbo->mapAttrib4f(geom::Attrib::COLOR);
        for (size_t i = 0; i < vbo->getNumVertices(); i++)
        {
            auto idx = mSpace->getIdxFromFlat(i);
            auto pos = mSpace->standardFunc(idx);
            
            auto f = func(mCtx->t, idx, pos);
            *posIter = f;
            *colIter = col(mCtx->t, idx, pos, f);
            
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
    
    
    void PltspaceImpl::setup()
    {
        mSize = vec3(10.f);
        mDivs = ivec3(20);
        
        mSpace = LineSpace::create(mSize, vec3(mDivs));
        
        mCtx->params->addButton("Reset", [this](){ resetSpace(); }, "key=c");
        
    }
    
    void PltspaceImpl::update()
    {
        if (mCtx->animate)
        {
            auto t = mCtx->t;
            
            auto eye = cone(t*0.5, 4., 4., 7.);
            auto at = cone(t*0.08-t, 2., 3., 0.);
            mCtx->cam.lookAt(eye, at);
            
            calcSpace();
        }
    }
    
    void PltspaceImpl::draw()
    {
        gl::ScopedColor scpCol;
        gl::ScopedDepth scpDpth(true); // R/W
        gl::clearColor(Color(0.3, 0.3, 0.3));
        gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        mSpace->draw();
    }
}
