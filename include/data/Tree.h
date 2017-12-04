//
//  Tree.h
//  PlotSpace
//
//  Created by Jacob Peyron on 2017-12-01.
//
//

#ifndef Tree_h
#define Tree_h

#include <vector>
#include "cinder/TriMesh.h"
#include "cinder/CinderMath.h"

namespace jp
{
    class Tree;
    using TreeRef = std::shared_ptr<Tree>;
    
    struct Branch;
    using BranchRef = std::shared_ptr<Branch>;
    using BranchVec = std::vector<BranchRef>;
    
    class Tree
    {
    public:
        
        Tree();
        ~Tree();
        
        BranchRef beginBranch(float startHeight, float startLat);
        void endBranch();
        
        static TreeRef create();
        
        BranchRef getRootBranch() { return mRoot; }
        const BranchRef getRootBranch() const { return mRoot; }
        
        
    private:
        BranchRef mRoot;
        BranchRef mCurrentBranch;
    };
        
    struct Branch
    {
        BranchRef parent;
        BranchVec childBranches;
        ci::mat4 coordSys;
    };
    
}


#endif /* Tree_h */
