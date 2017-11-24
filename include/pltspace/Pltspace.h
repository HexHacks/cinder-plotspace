//
//  Pltspace.h
//  PlotSpace
//
//  Created by Jacob Peyron on 23/11/17.
//
//

#ifndef Pltspace_h
#define Pltspace_h

#include "Context.h"
#include <memory>

namespace  jp
{
    class PltspaceImpl;
    
    class Pltspace;
    using PltspaceRef = std::shared_ptr<Pltspace>;
    
    class Pltspace
    {
        PltspaceImpl* mImpl;
        
    public:
        
        Pltspace(ContextRef ctx);
        ~Pltspace();
        
        static PltspaceRef create(ContextRef ctx)
        {
            return std::make_shared<Pltspace>(ctx);
        }
        
        void setup();
        void update();
        void draw();
    };
}

#endif /* Pltspace_h */
