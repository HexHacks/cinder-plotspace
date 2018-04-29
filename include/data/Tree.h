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
#include "cinder/Vector.h"

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
        
        static TreeRef create();
        
        BranchRef beginRoot();
        BranchRef beginBranch(BranchRef parent, int ring, int pt);
        void commitBranch(BranchRef branch);
        
        BranchRef getRootBranch() { return mRoot; }
        const BranchRef getRootBranch() const { return mRoot; }
        
        
    private:
        BranchRef mRoot;
        int mNextIndex;
    };
    
    using BRingGenerator = std::function<ci::vec3(float)>;
    using Points = std::vector<ci::vec3>;
    using Indices = std::vector<int>;
    
    struct BRing
    {
        Points points;
    };
    
    using BRingVec = std::vector<BRing>;
        
    struct Branch
    {
        BranchRef parent;
        BranchVec childBranches;
        
        int startIndex;
        Indices initialIndexLayer;
        ci::mat4 coordSys;
        BRingVec rings;
        
        // Populate rings based on simple mechanism
        void populateStandard();
        
        int getPointCount() const;
        int getPointCount(int ring) const;
        int getRelativeIndex(int ring) const; // Based on 0
        int getAbsoluteIndex(int ring) const; // Based on startIndex
        
        Points buildAllPoints() const;
        Indices buildAllIndices() const;
    };
    
}


#endif /* Tree_h */
