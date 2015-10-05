#pragma once

#include "ofMain.h"
#include "twomblyDrawer.h"

class ofApp : public ofBaseApp
{
    
public:
    
    void setup();
    void update();
    void draw();
    void keyPressed(int key);
    
    twomblyDrawer           m_drawer;
};
