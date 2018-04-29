//
//  Tree.cpp
//  PlotSpace
//
//  Created by Jacob Peyron on 04/12/17.
//
//

#include "data/Tree.h"


jp::Tree::Tree() :
    mNextIndex(0)
{
}

jp::Tree::~Tree()
{
}

jp::BranchRef jp::Tree::beginRoot()
{
    return beginBranch(nullptr, -1, -1);
}

jp::BranchRef jp::Tree::beginBranch(BranchRef parent, int ring, int pt)
{
    auto newBranch = std::make_shared<Branch>();
    if (parent == nullptr)
    {
        mRoot = newBranch;
        mRoot->parent = nullptr;
    }
    else
    {
        parent->childBranches.push_back(newBranch);
        newBranch->parent = parent;
    }
    
    newBranch->startIndex = mNextIndex;
    
    if (ring > -1)
    {
        
    }
    //TODO
    
    return newBranch;
}

void jp::Tree::commitBranch(BranchRef branch)
{
    mNextIndex += branch->getPointCount();
}

jp::TreeRef jp::Tree::create()
{
    return std::make_shared<Tree>();
}

void jp::Branch::populateStandard()
{
    // DOTTOOTO
}

int jp::Branch::getPointCount() const
{
    int pts = 0;
    for (const auto& ring : rings)
        pts += ring.points.size();
    return pts;
}
int jp::Branch::getPointCount(int ring) const
{
    return (int)rings[ring].points.size();
}
int jp::Branch::getRelativeIndex(int ring) const
{
    int rel = 0;
    for (int i = 0; i < ring; i++)
    {
        rel += getPointCount(i);
    }
    
    return rel;
}
int jp::Branch::getAbsoluteIndex(int ring) const
{
    return startIndex + getRelativeIndex(ring);
}

jp::Points jp::Branch::buildAllPoints() const
{
    Points pts;
    for (const auto& ring : rings)
    {
        pts.insert(pts.end(), ring.points.begin(), ring.points.end());
    }
    return pts;
}
jp::Indices jp::Branch::buildAllIndices() const
{
    Indices out;
    if (!initialIndexLayer.empty())
        out.insert(out.begin(), initialIndexLayer.begin(), initialIndexLayer.end());
    
    // TODOOTOODOO
    
    return out;
}
