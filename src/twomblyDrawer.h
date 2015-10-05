#ifndef __drawingTool_03__twomblyDrawer__
#define __drawingTool_03__twomblyDrawer__

#include "ofMain.h"
#include "ofxXmlSettings.h"
//#define DEBUG

class twomblyDrawer
{
    
public:
    
    twomblyDrawer();
    void setup();
    void update();
    void draw();
    void toggleScratches();
    void togglePaint();
    void toggleText();
    void toggleCurling();
    void toggleObjects();
    void toggleDisplayControls();
    void clearAll();
    
private:
    
    string boolToOffOn(bool b);
    ofColor boolToColor(bool b);
    void startNewLine();
    void endLine();
    void generateWords();
    void generateLetterForms();
    void generateObjects();
    void addPaintSplatter(int x, int y);
    void addPaintStroke(int x, int y, int radius, int variation);
    void addScratch(int x, int y);
    void addTextSnippet(int x, int y);
    void addObjectForm(int x, int y);
    
    //-------------------------------------------------------------- Constants
    const int               DRAW_DURATION = 5000;
    const int               MAX_MOVEMENT_DRIFT = 10;
    const int               DIST_FROM_EDGE = 300;
    const int               MAX_SECONDARY_LINES = 6;
    const int               MAX_PENCIL_VARIATION = 10;
    const int               MAX_SQUIGGLY_RESOLUTION = 40;
    const int               PENCIL_SMOOTHING = 8;
    const int               MIN_SCRATCH_LINES = 10;
    const int               MAX_SCRATCH_LINES = 40;
    const int               MAX_SCRATCH_LENGTH = 60;
    const int               MAX_SCRATCH_VARIATION = 20;
    const int               SCRATCH_RESOLUTION = 10;
    const int               MAX_PAINT_VARIATION = 50;
    const int               MAX_PAINT_SPLATTERS = 20;
    const int               PAINT_ALPHA = 20;
    const int               MIN_WORD_LENGTH = 4;
    
    //-------------------------------------------------------------- Colors
    ofColor                 canvasColor;
    ofColor                 pencilColor;
    ofColor                 paintColor;
    vector<ofColor>         paintColors;
    
    //-------------------------------------------------------------- Drawing elements
    vector<ofPolyline>      objectForms;
    vector<ofPolyline>      letterForms;
    vector<string>          twomblyDictionary;
    ofPolyline              currentPencilLine;
    ofPolyline              secondaryLine;
    vector<ofPolyline>      secondaryLines;
    vector<ofPolyline>      pencilLines;
    bool                    drawSecondary;
    ofImage                 brush;
    ofPoint                 drawPosition;
    ofPoint                 lastDrawPosition;
    int                     frameCounter;
    
    //-------------------------------------------------------------- FBOs, shaders, and textures
    ofFbo                   pencilFbo;
    ofFbo                   paintFbo;
    ofFbo                   paintFboRock;
    ofFbo                   paintFboNoise;
    ofFbo                   noiseFbo;
    ofFbo                   edgeDetectFbo;
    ofShader                paintStrokeShader;
    ofShader                edgeDetectShader;
    ofShader                displacementShader;
    ofShader                noiseShader;
    ofImage                 displacer;
    ofImage                 paperTexture;
    
    //-------------------------------------------------------------- Control toggles
    bool                    isScratch;
    bool                    isPaint;
    bool                    isText;
    bool                    isCurling;
    bool                    isObjects;
    bool                    isDisplayingControls;
    bool                    isSavingScreen;
    
};

#endif
