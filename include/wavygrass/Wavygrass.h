//
//  Wavygrass.h
//  PlotSpace
//
//  Created by Jacob Peyron on 23/11/17.
//
//

#ifndef Wavygrass_h
#define Wavygrass_h

#include "Context.h"
#include <memory>

namespace  jp
{
    class WavygrassImpl;
    
    class Wavygrass;
    using WavygrassRef = std::shared_ptr<Wavygrass>;
    
    class Wavygrass
    {
        WavygrassImpl* mImpl;
        
    public:
        
        Wavygrass(ContextRef ctx);
        ~Wavygrass();
        
        static WavygrassRef create(ContextRef ctx)
        {
            return std::make_shared<Wavygrass>(ctx);
        }
        
        void setup();
        void update();
        void draw();
    };
}

#endif /* Wavygrass_h */
