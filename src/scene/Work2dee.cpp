
//
//  {0}.h
//  PlotSpace
//
//  Created by Jacob Peyron on 2017-12-04.
//
//

#include "scene/Work2dee.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace cinder;

jp::Work2dee::Work2dee(ContextRef ctx) :
    Scene(ctx)
{
}
jp::Work2dee::~Work2dee()
{
}

void jp::Work2dee::setup()
{
    
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

    if (mCtx->animate)
    {
    }
}
void jp::Work2dee::draw()
{
    gl::ScopedColor scpCol;
    gl::ScopedDepth scpDpth(false);

    gl::clearColor(Color(1.0, 1.0, 1.0));
    gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto scr = mCtx->screenSize;
    auto hscr = scr / 2;
    gl::begin(GL_LINE);
    gl::color(1., 0., 0.);
    gl::vertex(0, hscr.y);
    gl::vertex(scr.x, hscr.y);
    gl::end();
}

