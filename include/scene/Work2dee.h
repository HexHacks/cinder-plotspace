
//
//  {0}.h
//  PlotSpace
//
//  Created by Jacob Peyron on 2017-12-04.
//
//


#ifndef Work2dee_h
#define Work2dee_h

#include "Scene.h"

namespace  jp
{
    class Work2dee;
    using Work2deeRef = std::shared_ptr<Work2dee>;
    
    class Work2dee : public Scene
    {
    public:
        
        Work2dee(ContextRef ctx);
        virtual ~Work2dee();
        
        static Work2deeRef create(ContextRef ctx)
        {
            return std::make_shared<Work2dee>(ctx);
        }
        
        virtual std::string getName() override { return "Work2dee"; }
        virtual void setup() override;
        virtual void activate() override;
        virtual void deactivate() override;
        virtual void update() override;
        virtual void draw() override;
        virtual void reset() override;
    };
}

#endif /* Work2dee_h */

