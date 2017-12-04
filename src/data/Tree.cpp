//
//  Tree.cpp
//  PlotSpace
//
//  Created by Jacob Peyron on 04/12/17.
//
//

#include "data/Tree.h"


jp::Tree::Tree()
{
}

jp::Tree::~Tree()
{
}

jp::BranchRef jp::Tree::beginBranch(float startHeight, float startLat)
{
    auto newBranch = std::make_shared<Branch>();
    newBranch->parent = mCurrentBranch;
    //TODO
    
    return newBranch;
}

void jp::Tree::endBranch()
{
    if (mCurrentBranch->parent)
        mCurrentBranch = mCurrentBranch->parent;
}

jp::TreeRef jp::Tree::create()
{
    return std::make_shared<Tree>();
}
