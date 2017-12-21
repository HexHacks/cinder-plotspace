
//
//  {0}.h
//  PlotSpace
//
//  Created by Jacob Peyron on 2017-11-27.
//
//

#include "scene/Treegen.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace cinder;

jp::Treegen::Treegen(ContextRef ctx) :
    Scene(ctx)
{
}
jp::Treegen::~Treegen()
{
}

void jp::Treegen::setup()
{
    
}
void jp::Treegen::activate()
{
    mCtx->muxFormat.videoOptions["tune"] = "film";
}
void jp::Treegen::deactivate()
{
    
}
void jp::Treegen::update()
{
    auto t = mCtx->t;

    if (mCtx->animate)
    {
    }
}
void jp::Treegen::draw()
{
    gl::ScopedColor scpCol;
    gl::ScopedDepth scpDpth(true);

    gl::clearColor(Color(0.3, 0.3, 0.3));
    gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void jp::Treegen::reset()
{
    
}

