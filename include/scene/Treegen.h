
//
//  {0}.h
//  PlotSpace
//
//  Created by Jacob Peyron on 2017-11-27.
//
//


#ifndef Treegen_h
#define Treegen_h

#include "Scene.h"

namespace  jp
{
    class Treegen;
    using TreegenRef = std::shared_ptr<Treegen>;
    
    class Treegen : public Scene
    {
    public:
        
        Treegen(ContextRef ctx);
        virtual ~Treegen();
        
        static TreegenRef create(ContextRef ctx)
        {
            return std::make_shared<Treegen>(ctx);
        }
        
        virtual std::string getName() override { return "Treegen"; }
        virtual void setup() override;
        virtual void activate() override;
        virtual void deactivate() override;
        virtual void update() override;
        virtual void draw() override;
        virtual void reset() override;
    };
}

#endif /* Treegen_h */

