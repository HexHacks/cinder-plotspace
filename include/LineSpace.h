//
//  LineSpace.h
//  PlotSpace
//
//  Created by Jacob Peyron on 10/11/17.
//
//

#include "cinder/CinderGlm.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/VboMesh.h"
#include "cinder/GeomIo.h"
#include <vector>

#ifndef LineSpace_h
#define LineSpace_h

class LineSpace;
using LineSpaceRef = std::shared_ptr<LineSpace>;

class LineSpace
{
    using vec3 = ci::vec3;
    using vec4 = ci::vec4;
    using ivec3 = ci::ivec3;
    using VboMesh = ci::gl::VboMesh;
    using VboMeshRef = ci::gl::VboMeshRef;
    
    vec3 mSize;
    vec3 mHSize;
    ivec3 mCount;
    
    VboMeshRef mVbo;
    
public:
    LineSpace(const vec3& size = vec3(10.), const ivec3& count = ivec3(10)) :
        mSize(size),
        mHSize(size * 0.5f),
        mCount(count)
    {}
    
    static LineSpaceRef create(const vec3& size = vec3(10.), const ivec3& count = ivec3(10))
    {
        auto ret = std::make_shared<LineSpace>(size, count);
        ret->generate();
        
        return ret;
    }
    
    VboMeshRef getVbo() { return mVbo; }
    vec3 getSize() const { return mSize; }
    ivec3 getCount() const { return mCount; }
    
    ivec3 getIdxFromFlat(size_t idx) const
    {
        return ivec3(idx%mCount.x, idx/(mCount.x*mCount.z),(idx/mCount.x)%mCount.z);
    }
    
    size_t getFlatFromIdx(const ivec3& idx) const
    {
        return idx.x + mCount.x * (idx.y + mCount.z * idx.z);
    }
    
    std::vector<GLuint> getNeighbours(const ivec3& idx) const
    {
        static std::vector<ivec3> diffs{
            ivec3(-1, 0, 0),
            ivec3(1, 0, 0),
            ivec3(0, -1, 0),
            ivec3(0, 1, 0),
            ivec3(0, 0, -1),
            ivec3(0, 0, 1)
        };
        
        std::vector<GLuint> out;
        for (const auto& d : diffs)
        {
            auto n = idx + d;
            if (n.x >= 0 && n.x < mCount.x &&
                n.y >= 0 && n.y < mCount.y &&
                n.z >= 0 && n.z < mCount.z)
                out.push_back(getFlatFromIdx(n));
        }
        return out;
    }
    
    std::vector<GLuint> createIndices() const
    {
        std::vector<GLuint> out;
        for (size_t y = 0; y < mCount.y; y++)
        {
            for (size_t z = 0; z < mCount.z; z++)
            {
                for (size_t x = 0; x < mCount.x; x++)
                {
                    ivec3 currIdx(x, y, z);
                    auto flat = getFlatFromIdx(currIdx);
                    auto nei = getNeighbours(currIdx);
                    
                    for (const auto& n : nei)
                    {
                        out.push_back(flat);
                        out.push_back(n);
                    }
                }
            }
        }
        
        return out;
    }
    
    vec3 standardFunc(const ivec3& idx) const
    {
        vec3 zp((idx.x)/float(mCount.x - 1), idx.y/float(mCount.y - 1), idx.z/float(mCount.z - 1));
        return zp * mSize - mHSize;
    }
    
    std::vector<vec3> getStandardPositions() const
    {
        std::vector<vec3> out(mCount.x*mCount.y*mCount.z);
        
        for (size_t i = 0; i < out.size(); i++)
        {
            auto idx = getIdxFromFlat(i);
            out[i] = standardFunc(idx);
        }
        
        return out;
    }
    
    std::vector<vec4> getStandardColors() const
    {
        return std::vector<vec4>(mCount.x*mCount.y*mCount.z, vec4(1.));
    }
    
    VboMeshRef generate()
    {
        const auto cPosAttrib = ci::geom::Attrib::POSITION;
        const auto cColAttrib = ci::geom::Attrib::COLOR;
        std::vector<VboMesh::Layout> bufferLayout = {
            VboMesh::Layout().usage( GL_DYNAMIC_DRAW ).attrib( cPosAttrib, 3 ),
            VboMesh::Layout().usage( GL_DYNAMIC_DRAW ).attrib( cColAttrib, 4 )
        };
        
        size_t vertCount = mCount.x*mCount.y*mCount.z;
        
        auto indices = createIndices();
        mVbo = VboMesh::create(vertCount, GL_LINES, bufferLayout, indices.size(), GL_UNSIGNED_INT);
        
        mVbo->bufferIndices(indices.size() * sizeof(GLuint), indices.data());
        
        auto pos = getStandardPositions();
        auto col = getStandardColors();
        mVbo->bufferAttrib(cPosAttrib, pos.size() * sizeof(vec3), pos.data());
        mVbo->bufferAttrib(cColAttrib, col.size() * sizeof(vec4), col.data());
        
        return mVbo;
    }
    
    void draw()
    {
        if (mVbo)
            ci::gl::draw(mVbo);
    }
};

#endif /* LineSpace_h */
