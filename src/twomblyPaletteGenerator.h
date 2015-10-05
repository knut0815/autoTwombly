#pragma once

#include "ofMain.h"

class twomblyPaletteGenerator
{
    
public:
    
    twomblyPaletteGenerator();
    void setup(string &fileName);
    void draw();
    vector<ofColor>& getPalette();
    
    vector<ofColor>     m_palette;
    
private:
    
    void refinePalette();
    
    const int           SAMPLE_RESOLUTION = 30;
    const int           SAMPLE_ALPHA = 220;
    const int           MIN_SAMPLE_DIFFERENCE = 30;
    
};