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
#include <boost/filesystem.hpp>

namespace jp
{
    using path = boost::filesystem::path;
    
    path ensureTmpOutput(const path& dir, const std::string& ext)
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
}


#endif /* JUtil_h */
