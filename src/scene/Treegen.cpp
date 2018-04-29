
//
//  {0}.h
//  PlotSpace
//
//  Created by Jacob Peyron on 2017-11-27.
//
//

#include "scene/Treegen.h"
#include "cinder/gl/gl.h"
#include "cinder/audio/audio.h"

using namespace ci;
using namespace cinder;

namespace
{
    audio::MonitorNodeRef xMonitorNode;
    audio::MonitorSpectralNodeRef xSpectralNode;
    audio::InputDeviceNodeRef xInput;
    float xIdxToFreq;
    
    gl::BatchRef xSphere;
}

jp::Treegen::Treegen(ContextRef ctx) :
    Scene(ctx)
{
}
jp::Treegen::~Treegen()
{
}

static void setupAudio()
{
    auto ctx = audio::Context::master();
    
    //xMonitorNode = ctx->makeNode(new audio::MonitorNode);
    
    // By providing an FFT size double that of the window size, we 'zero-pad' the analysis data, which gives
    // an increase in resolution of the resulting spectrum data.
    auto monitorFormat = audio::MonitorSpectralNode::Format().fftSize( 2048 ).windowSize( 1024 );
    xSpectralNode = ctx->makeNode(new audio::MonitorSpectralNode(monitorFormat));
    
    xInput = ctx->createInputDeviceNode();
    
    //xInput >> xMonitorNode;
    xInput >> xSpectralNode;
    
    xIdxToFreq = (float)ctx->getSampleRate() / (float)xSpectralNode->getFftSize();
}

void jp::Treegen::setup()
{
    setupAudio();
    
    auto lambert = gl::ShaderDef().lambert().color();
    gl::GlslProgRef shader = gl::getStockShader( lambert );
    xSphere = gl::Batch::create(geom::Sphere().subdivisions(20), shader);
}
void jp::Treegen::activate()
{
    mCtx->muxFormat.videoOptions["tune"] = "film";
    
    auto ctx = audio::Context::master();
    
    xInput->enable();
    ctx->enable();
}
void jp::Treegen::deactivate()
{
    auto ctx = audio::Context::master();
    ctx->disable();
    xInput->disable();
}
void jp::Treegen::update()
{
    
}
void jp::Treegen::draw()
{
    gl::ScopedColor scpCol;
    gl::ScopedDepth scpDpth(true);

    gl::clearColor(Color(0.3, 0.3, 0.3));
    gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    auto t = mCtx->t;
    
    if (mCtx->animate)
    {
        const auto scl = 0.05f;
        const auto& magSpectrum = xSpectralNode->getMagSpectrum();
        const auto cnt = magSpectrum.size();
        const auto fullScl = scl * cnt;
        const auto maxFreq = (cnt-1) * xIdxToFreq;
        
        const float rad = 2.;
        for (size_t i = 0; i < cnt; i++)
        {
            gl::pushModelView();
            float zo = (i+1) / (float)cnt;
            const auto freq = i * xIdxToFreq;
            const auto& val = magSpectrum[i];

            
            gl::translate(vec3(cos(zo*2.*M_PI)*rad, val*50., sin(zo*2.*M_PI) * rad));
            gl::scale(vec3(scl, scl, scl));
            
            gl::color(Color(ColorModel::CM_HSV, freq/maxFreq, 6., 6.));
            xSphere->draw();
            
            gl::popModelView();
        }
    }
}

void jp::Treegen::reset()
{
    
}

