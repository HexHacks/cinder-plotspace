//
//  Wavygrass.h
//  PlotSpace
//
//  Created by Jacob Peyron on 23/11/17.
//
//

#ifndef Wavygrass_h
#define Wavygrass_h

#include "Scene.h"
#include <memory>

namespace  jp
{
    class WavygrassImpl;
    
    class Wavygrass;
    using WavygrassRef = std::shared_ptr<Wavygrass>;
    
    class Wavygrass : public Scene
    {
        WavygrassImpl* mImpl;
        
    public:
        
        Wavygrass(ContextRef ctx);
        virtual ~Wavygrass();
        
        static WavygrassRef create(ContextRef ctx)
        {
            return std::make_shared<Wavygrass>(ctx);
        }
        
        virtual std::string getName() override { return "Wavygrass"; }
        virtual void setup() override;
        virtual void activate() override;
        virtual void deactivate() override;
        virtual void update() override;
        virtual void draw() override;
        virtual void reset() override;
    };
}

#endif /* Wavygrass_h */
