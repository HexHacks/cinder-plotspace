//
//  Context.h
//  PlotSpace
//
//  Created by Jacob Peyron on 23/11/17.
//
//

#ifndef Context_h
#define Context_h

#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/params/Params.h"

namespace jp
{
    struct Context;
    using ContextRef = std::shared_ptr<Context>;
    
    struct Context
    {
        cinder::CameraPersp cam;
        cinder::CameraUi camUi;
        cinder::params::InterfaceGlRef params;
        
        bool animate;
        float t;
        int frame;
        cinder::ivec2 screenSize;
    };
}

#endif /* Context_h */
