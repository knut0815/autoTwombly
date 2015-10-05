#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( )
{
    ofSetCurrentRenderer(ofGLProgrammableRenderer::TYPE);
    ofSetupOpenGL(1920, 1080, OF_FULLSCREEN);
	ofRunApp(new ofApp());
}
