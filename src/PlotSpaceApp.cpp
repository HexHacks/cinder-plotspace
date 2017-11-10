#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class PlotSpaceApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void PlotSpaceApp::setup()
{
}

void PlotSpaceApp::mouseDown( MouseEvent event )
{
}

void PlotSpaceApp::update()
{
}

void PlotSpaceApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( PlotSpaceApp, RendererGl )
