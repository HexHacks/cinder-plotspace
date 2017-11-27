//
//  JUtil.cpp
//  PlotSpace
//
//  Created by Jacob Peyron on 25/11/17.
//
//

#include "JUtil.h"
#include "cinder/DataSource.h"

boost::filesystem::path jp::ensureTmpOutput(const path& dir, const std::string& ext)
{
    auto o = dir / (boost::format("out_0.%1%") % ext).str();
    if (!boost::filesystem::exists(dir))
    {
        boost::filesystem::create_directory(dir);
    }
    else if (boost::filesystem::is_directory(dir))
    {
        int i = 0;
        while (boost::filesystem::exists(o))
        {
            o = dir / (boost::format("out_%1%.%2%") % i++ % ext).str();
        }
    }
    else
    {
        throw std::runtime_error("Bad input directory");
    }
    
    return o;
}

ci::gl::GlslProgRef jp::loadShader(const path& dir, const std::string& name)
{
    using namespace ci;
    
    auto vs = dir / (name + ".vert");
    auto fs = dir / (name + ".frag");
    return gl::GlslProg::create(loadFile(vs), loadFile(fs));
}
