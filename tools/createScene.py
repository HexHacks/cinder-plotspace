import sys
import time

className = sys.argv[1]

comment = """
//
//  {{0}}.h
//  PlotSpace
//
//  Created by Jacob Peyron on {0}.
//
//
"""

comment = comment.format(time.strftime("%Y-%m-%d"))

headerSrc = comment + """

#ifndef {0}_h
#define {0}_h

#include "Scene.h"

namespace  jp
{{
    class {0};
    using {0}Ref = std::shared_ptr<{0}>;
    
    class {0} : public Scene
    {{
    public:
        
        {0}(ContextRef ctx);
        virtual ~{0}();
        
        static {0}Ref create(ContextRef ctx)
        {{
            return std::make_shared<{0}>(ctx);
        }}
        
        virtual std::string getName() override {{ return "{0}"; }}
        virtual void setup() override;
        virtual void activate() override;
        virtual void deactivate() override;
        virtual void reset() override;
        virtual void update() override;
        virtual void draw() override;
    }};
}}

#endif /* {0}_h */

""".format(className)

cppSrc = comment + """
#include "scene/{0}.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace cinder;

jp::{0}::{0}(ContextRef ctx) :
    Scene(ctx)
{{
}}
jp::{0}::~{0}()
{{
}}

void jp::{0}::setup()
{{
    
}}
void jp::{0}::activate()
{{
    mCtx->muxFormat.videoOptions["tune"] = "film";
}}
void jp::{0}::deactivate()
{{
    
}}
void jp::{0}::reset()
{{
    
}}
void jp::{0}::update()
{{
    auto t = mCtx->t;

    if (mCtx->animate)
    {{
    }}
}}
void jp::{0}::draw()
{{
    gl::ScopedColor scpCol;
    gl::ScopedDepth scpDpth(true);

    gl::clearColor(Color(0.3, 0.3, 0.3));
    gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}}

""".format(className)

with open('include/scene/{0}.h'.format(className), 'w') as h:
    h.write(headerSrc)

with open('src/scene/{0}.cpp'.format(className), 'w') as c:
    c.write(cppSrc)