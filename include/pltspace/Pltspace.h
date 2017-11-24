//
//  Pltspace.h
//  PlotSpace
//
//  Created by Jacob Peyron on 23/11/17.
//
//

#ifndef Pltspace_h
#define Pltspace_h

#include <memory>
#include "Scene.h"

namespace  jp
{
    class PltspaceImpl;
    
    class Pltspace;
    using PltspaceRef = std::shared_ptr<Pltspace>;
    
    class Pltspace : public Scene
    {
        PltspaceImpl* mImpl;
        
    public:
        
        Pltspace(ContextRef ctx);
        virtual ~Pltspace();
        
        static PltspaceRef create(ContextRef ctx)
        {
            return std::make_shared<Pltspace>(ctx);
        }
        
        virtual std::string getName() override { return "Pltspace"; }
        virtual void setup() override;
        virtual void activate() override;
        virtual void deactivate() override;
        virtual void update() override;
        virtual void draw() override;
    };
}

#endif /* Pltspace_h */
