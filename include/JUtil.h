//
//  JUtil.h
//  PlotSpace
//
//  Created by Jacob Peyron on 17/11/17.
//
//

#ifndef JUtil_h
#define JUtil_h

#include <string>
#include <exception>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <cmath>
#include "cinder/gl/GlslProg.h"

namespace jp
{
    using path = boost::filesystem::path;
    
    template<typename T>
    T sigmoid(T x)
    {
        float xp = exp(x);
        return xp / (xp + 1.f);
    }
    
    path ensureTmpOutput(const path& dir, const std::string& ext);
    
    ci::gl::GlslProgRef loadShader(const path& dir, const std::string& name);
}


#endif /* JUtil_h */
