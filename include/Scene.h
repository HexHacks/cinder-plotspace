//
//  Scene.h
//  PlotSpace
//
//  Created by Jacob Peyron on 24/11/17.
//
//

#ifndef Scene_h
#define Scene_h

#include <memory>

#include "Context.h"

namespace jp
{    class Scene;
    using SceneRef = std::shared_ptr<Scene>;
    
    class Scene
    {
    public:
        Scene(ContextRef ctx) : mCtx(ctx) {}
        virtual ~Scene() = default;
        
        virtual std::string getName() = 0;
        
        // Called once at setup
        virtual void setup() = 0;
        
        // Called when on scene switch
        virtual void activate() = 0;
        virtual void deactivate() = 0;
        
        virtual void update() = 0;
        virtual void draw() = 0;
        
    protected:
        ContextRef mCtx;
    };
}

#endif /* Scene_h */
