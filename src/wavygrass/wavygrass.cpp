#include "wavygrass/Wavygrass.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/TriMesh.h"
#include "cinder/CameraUi.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;
#define PI 3.1415926

namespace
{
    const vec3 cBasisRight(1., 0., 0.);
    const vec3 cBasisUp(0., 1., 0.);
    const vec3 cBasisBack(0., 0., 1.);
    
    inline quat axisToRot(const vec3& axis, float a)
    {
        auto sa = sin(a/2.f);
        auto ca = cos(a/2.f);
        return quat(ca, axis.x * sa, axis.y * sa, axis.z * sa);
    }
    
    // Return and orthogonalize b with respect to a
    // All vectors will have len(v) == 1
    inline vec3 orthoNormalize(vec3& a, vec3& b)
    {
        a = normalize(a);
        auto c = glm::cross(a, b);
        c = normalize(c);
        b = normalize(glm::cross(c, a));
        
        return c;
    }
    
    inline mat4 basisMat(const vec3& right, const vec3& up, const vec3& back, const vec3& pos)
    {
        return mat4(right.x, right.y, right.z, 0.f,
                    up.x, up.y, up.z, 0.f,
                    back.x, back.y, back.z, 0.f,
                    pos.x, pos.y, pos.z, 1.f);
    }
}


class TreeMesh
{
    int gran;
    
    void addRingPts(TriMesh* mesh, const mat4& frame)
    {
        // Local coords
        vec3 dir(0., 0., -1.);
        vec3 tang(1., 0., 0.);
        float rotStep = 2.f * PI / gran;
        for (int i = 0; i < gran; i++)
        {
            quat rot(axisToRot(dir, rotStep*i));
            vec3 t = rot * tang;
            vec4 pt = frame * vec4(t.x, t.y, t.z, 1.);
            
            mesh->appendPosition(vec3(pt.x, pt.y, pt.z));
            mesh->appendNormal(t);
        }
        
    }
    
    void connectLastRing(TriMesh* mesh)
    {
        // The previous ring first index
        int r0First = mesh->getNumVertices() - gran*2;
        
        // There are just one or less rings present (NOP)
        if (r0First < 0)
            return;
        
        // The current ring first index
        int r1First = mesh->getNumVertices() - gran;
        
        for (int i = 0; i < gran-1; i++)
        {
            int v0 = r0First+i;
            int v1 = r0First+i+1;
            int v2 = r1First+i+1;
            int v3 = r1First+i;
            
            mesh->appendTriangle(v0, v1, v2);
            mesh->appendTriangle(v2, v3, v0);
        }
        
        int lst = gran-1;
        int v0 = r0First+lst;
        int v1 = r0First;
        int v2 = r1First;
        int v3 = r1First+lst;
        
        mesh->appendTriangle(v0, v1, v2);
        mesh->appendTriangle(v2, v3, v0);
    }
    
public:
    TreeMesh(int granularity=20): gran(granularity)
    {}
    
    void buildMesh(TriMesh* mesh)
    {
        vec3 eye(0.);
        vec3 at(1., 0., 0.);
        vec3 up(0., 1., 0.);
        addRingPts(mesh, glm::lookAt(eye, at, up));
        for (int i = 0; i < 5; i++)
        {
            eye += up * 0.3f;
            at += up * 0.3f;
            addRingPts(mesh, glm::lookAt(eye, at, up));
            connectLastRing(mesh);
        }
    }
    
    using TLocalTransFunc = std::function<mat4(float)>;
    
    void buildMesh(TriMesh* mesh, const BSpline3f& spl, int iter = 10, TLocalTransFunc localT = nullptr)
    {
        for (int i = 0; i <= iter; i++)
        {
            vec3 eye;
            vec3 at;
            float f = i / float(iter);
            spl.get(f, &eye, &at);
            
            vec3 right = vec3(-eye.x, 0., -eye.z);
            vec3 up = -orthoNormalize(at, right);
            
            mat4 T = basisMat(right, up, -at, eye);
            if (localT)
            {
                T = T * localT(f);
            }
            
            addRingPts(mesh, T);
            connectLastRing(mesh);
        }
    }
};

namespace jp
{
class WavygrassImpl
{
    ContextRef mCtx;
    
    TriMeshRef mTriMesh;
    TreeMesh mTree;
    
    std::vector<std::vector<float>> mScales;
    
    void createTree(float t);
    
  public:
    WavygrassImpl(ContextRef ctx) :
        mCtx(ctx)
    {}
    
	void setup();
	void update();
	void draw();
};
    
    Wavygrass::Wavygrass(ContextRef ctx) :
        Scene(ctx),
        mImpl(new WavygrassImpl(ctx))
    {
    }
    
    Wavygrass::~Wavygrass()
    {
        delete mImpl;
    }
    
    void Wavygrass::setup()
    {
        mImpl->setup();
    }
    
    void Wavygrass::activate()
    {
    }
    
    void Wavygrass::deactivate()
    {
        
    }
    
    void Wavygrass::update()
    {
        mImpl->update();
    }
    void Wavygrass::draw()
    {
        mImpl->draw();
    }
    
}

void jp::WavygrassImpl::createTree(float t)
{
    mTriMesh = TriMesh::create(TriMesh::Format().positions().normals());
    
    std::vector<vec3> pts;
    int cnt = 30;
    for (int i = 0; i <= cnt; i++)
    {
        float f = i / float(cnt);
        
        auto at = PI*sin(t);
        auto bt = PI*cos(t);
        auto a = 2.f*PI*f;
        auto r = f*f*f*f;
        pts.push_back(vec3(r*cos(a+at), f*5., r*sin(a+bt)));
    }
    
    auto localT = [](float f)
    {
        vec3 sides(1., 1., 0);
        return glm::scale(sides * (1.f-f));
    };
    
    BSpline3f ptSpl(pts, 3, false, true);
    
    mTree.buildMesh(mTriMesh.get(), ptSpl, 40, localT);
}

void jp::WavygrassImpl::setup()
{
    Rand rnd(100);
    
    int w = 10;
    int h = 10;
    for (int y = 0; y < h; y++)
    {
        mScales.push_back(std::vector<float>());
        for(int x = 0; x < w; x++)
        {
            mScales[y].push_back(rnd.nextFloat(0.7, 1.));
        }
    }
    
    createTree(0.f);
}

void jp::WavygrassImpl::update()
{
    if (mCtx->animate)
    {
        auto t = mCtx->t;
        auto f = 2.*PI*t*0.01;
        auto r = 19.f;
        mCtx->cam.lookAt(vec3(r*cos(f), 5.f - sin(f*10.), r*sin(f)), vec3(0., 3., 0.));
    }
}

void jp::WavygrassImpl::draw()
{
    gl::ScopedColor scpCol;
    gl::ScopedBlend scpBlnd(GL_SRC_ALPHA, GL_ONE);
    
    gl::setMatrices(mCtx->cam);
    //gl::enableDepth();
    //gl::enableFaceCulling();
    
    //gl::drawCoordinateFrame();
    
    float t = mCtx->t;
    createTree(t);
    
    auto w = mScales[0].size();
    auto h = mScales.size();
    for (int y = 0; y < h; y++)
    {
        for(int x = 0; x < w; x++)
        {
            gl::ScopedModelMatrix scpMM;
            
            auto at = vec3(x-(w-1)/2.f, 0, y-(h-1)/2.f) * 3.f;
            gl::translate(at);
            
            auto scl = mScales[y][x];
            gl::scale(vec3(1., scl, 1.));
            
            auto xx = 0.5*x / float(w);
            auto yy = 0.5*y / float(h);
            gl::color(ColorA(Colorf(ColorModel::CM_HSV, xx+yy, 0.7, 0.7), 0.25f));
            gl::draw(*mTriMesh);
        }
    }
}

