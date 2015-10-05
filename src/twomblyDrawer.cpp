#include "twomblyDrawer.h"

//--------------------------------------------------------------
//              Public functions and constructors
//--------------------------------------------------------------
twomblyDrawer::twomblyDrawer()
{
    
}

void twomblyDrawer::setup()
{
    canvasColor.set(249, 242, 223);
    pencilColor.set(25, 25, 25, 140);
    paintColors.push_back(ofColor(156, 205, 201, 170));
    paintColors.push_back(ofColor(248, 177, 113, 170));
    paintColors.push_back(ofColor(233, 158, 137, 170));
    paintColors.push_back(ofColor(214, 115, 109, 170));
    paintColors.push_back(ofColor(12, 14, 26, 40));
    paintColors.push_back(ofColor(243, 220, 189, 170));
    paintColors.push_back(ofColor(192, 49, 55, 170));
    paintColors.push_back(ofColor(84, 110, 81, 40));
    paintColor =        paintColors[0];
    
    generateWords();
    generateLetterForms();
    generateObjects();

    drawSecondary =     false;
    
    pencilFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA, 8);
    pencilFbo.begin();
    ofClear(255, 255, 255, 0);
    pencilFbo.end();
    
    paintFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA, 4);
    paintFbo.begin();
    ofClear(255, 255, 255, 0);
    paintFbo.end();
    
    noiseFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA, 4);
    noiseFbo.begin();
    ofClear(255, 255, 255, 0);
    noiseFbo.end();
    
    paintFboRock.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA, 4);
    paintFboRock.begin();
    ofClear(255, 255, 255, 0);
    paintFboRock.end();
    
    paintFboNoise.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA, 4);
    paintFboNoise.begin();
    ofClear(255, 255, 255, 0);
    paintFboNoise.end();
    
    edgeDetectFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA, 4);
    edgeDetectFbo.begin();
    ofClear(255, 255, 255, 0);
    edgeDetectFbo.end();
    
    ofSetTextureWrap(GL_REPEAT);
    paintStrokeShader.load("shaders/passthrough.vert", "shaders/paintStroke.frag");
    edgeDetectShader.load("shaders/passthrough.vert", "shaders/edgeDetect.frag");
    displacementShader.load("shaders/passthrough.vert", "shaders/displacement.frag");
    noiseShader.load("shaders/passthrough.vert", "shaders/noise.frag");
    displacer.loadImage("displacement.jpg");
    brush.loadImage("brush.png");
    
    ofBackground(canvasColor);
    ofSetVerticalSync(false);

    isScratch =             true;
    isPaint =               true;
    isText =                true;
    isCurling =             true;
    isObjects =             true;
    isDisplayingControls =  false;
    isSavingScreen =        false;
    
    //Kick off the drawing somewhere in the middle of the canvas
    frameCounter = 0;
    int rX = ofRandom(ofGetWidth()/2 - 100,
                      ofGetWidth()/2 + 100);
    int rY = ofRandom(ofGetHeight()/2 - 100,
                      ofGetHeight()/2 + 100);
    drawPosition.set(rX, rY);
    startNewLine();
}

void twomblyDrawer::update()
{
    //We keep track of how many frames we've drawn thus far and clear the entire canvas every once in a while
    frameCounter++;
    if (frameCounter > DRAW_DURATION)
    {
        frameCounter = 0;
        clearAll();
    }

    if (ofRandom(1.0f) > 0.98f || ofGetMousePressed())
    {
        endLine();
        startNewLine();
    }
    
    float noiseX = ofMap(ofNoise(ofGetElapsedTimef()), 0.0f, 1.0f, -MAX_MOVEMENT_DRIFT, MAX_MOVEMENT_DRIFT);
    float noiseY = ofMap(ofNoise(ofGetElapsedTimef() + 1000.0f), 0.0f, 1.0f, -MAX_MOVEMENT_DRIFT, MAX_MOVEMENT_DRIFT);
    float randOffsetX = ofRandom(-MAX_MOVEMENT_DRIFT, MAX_MOVEMENT_DRIFT);
    float randOffsetY = ofRandom(-MAX_MOVEMENT_DRIFT, MAX_MOVEMENT_DRIFT);
    float x = lastDrawPosition.x + noiseX + randOffsetX;
    float y = lastDrawPosition.y + noiseY + randOffsetY;
    
    //Make sure the brush doesn't leave the rectangle described
    x = ofClamp(x, DIST_FROM_EDGE, ofGetWidth() - DIST_FROM_EDGE);
    y = ofClamp(y, DIST_FROM_EDGE, ofGetHeight() - DIST_FROM_EDGE);
    
    float velocity = ofDist(lastDrawPosition.x, lastDrawPosition.y, x, y);
    
    //Secondary pencil stroke
    if (drawSecondary)
    {
        for(int i = 0; i < secondaryLines.size(); i++)
        {
            //Optionally draw with a random offset
            if(ofRandom(1.0f) > 0.5f)
            {
                secondaryLines[i].addVertex(x + ofRandom(MAX_PENCIL_VARIATION), y + ofRandom(MAX_PENCIL_VARIATION));
            }
            else
            {
                secondaryLines[i].addVertex(x, y);
            }
        }
    }
    
    //Main pencil stroke
    if(ofRandom(1.0f) > 0.6f)
    {
        currentPencilLine.addVertex(x + ofRandom(MAX_PENCIL_VARIATION), y + ofRandom(MAX_PENCIL_VARIATION));
    }
    
    //Pencil curling
    float pctCurly = ofMap(velocity, 0, 50, 0.95, 0.2, true);
    if(ofRandom(1.0f) > pctCurly && isCurling)
    {
        //First, pick a center point for the curl
        ofPoint squigglyCenter(x + ofRandom(MAX_PENCIL_VARIATION * 2), y + ofRandom(MAX_PENCIL_VARIATION * 2));
        
        //Calculate the radius of the curl
        float r = ofDist(x, y, squigglyCenter.x, squigglyCenter.y);
        
        //Determine a circle resolution for the curl
        int resolution = (int)ofRandom(MAX_SQUIGGLY_RESOLUTION);
        
        //From the resolution, calculate an angle step
        float angleOffset = ofDegToRad(360.0f / resolution);
        
        //Construct the curl and add it to the current polyline
        for(int i = 0; i < resolution; i++)
        {
            float squigglyX = r * cos(angleOffset * i) + x + ofRandom(MAX_PENCIL_VARIATION);
            float squigglyY = r * sin(angleOffset * i) + y + ofRandom(MAX_PENCIL_VARIATION);
            currentPencilLine.addVertex(squigglyX, squigglyY);
        }
    }
    else
    {
        currentPencilLine.addVertex(x, y);
    }
    
    paintFbo.begin(); //----------------------------------- PAINT FBO BEGINS
    
    float pctRadius = ofMap(velocity, 0.0f, 40.0f, 5.0f, 70.0f, true); //last val was 50.0f
    float pctVariation = ofMap(velocity, 0.0f, 50.0f, 5.0f, MAX_PAINT_VARIATION, true);
    if (ofRandom(1.0f) > 0.6f && isPaint) addPaintStroke(x, y, pctRadius, pctVariation);
    if (ofRandom(1.0f) > 0.98f && isPaint) addPaintSplatter(x, y);
    
    paintFbo.end(); //----------------------------------- PAINT FBO ENDS
    
    
    pencilFbo.begin(); //----------------------------------- PENCIL FBO BEGINS
    
    if (ofRandom(1.0f) > 0.98f && isScratch) addScratch(x, y);
    if (ofRandom(1.0f) > 0.996f && isText) addTextSnippet(x, y);
    if (ofRandom(1.0f) > 0.996f && isObjects) addObjectForm(x, y);
    
    pencilFbo.end(); //----------------------------------- PENCIL FBO ENDS
    
    lastDrawPosition.set(x, y);
}

void twomblyDrawer::draw()
{
    //-------------------------------------------------------------- RENDER THE NOISE INTO AN FBO
    noiseFbo.begin();
    
    ofClear(255, 255, 255, 0);
    ofSetColor(255, 255, 255);
    noiseShader.begin();
    noiseShader.setUniform1f("time", ofGetElapsedTimef());
    ofRect(0, 0, ofGetWidth(), ofGetHeight());
    noiseShader.end();
    
    noiseFbo.end();
    //--------------------------------------------------------------
    
    
    //-------------------------------------------------------------- DRAW THE PENCIL INTO AN FBO
    pencilFbo.begin();
    
    ofSetColor(pencilColor, 100);
    if(drawSecondary)
    {
        for(int i = 0; i < secondaryLines.size(); i++)
        {
            ofPolyline secondaryLineSmoothed = secondaryLine.getSmoothed(PENCIL_SMOOTHING);
            secondaryLineSmoothed.draw();
        }
    }
    ofPolyline currentPencilLineSmoothed = currentPencilLine.getSmoothed(PENCIL_SMOOTHING);
    currentPencilLineSmoothed.draw();

    for(auto &pl: pencilLines) pl.draw();
    
    //Since the lines and scratches have been drawn into the FBO, clear the vectors
    pencilLines.clear();
    
    pencilFbo.end();
    //--------------------------------------------------------------
    
    
    ofSetColor(255, 255, 255);
    
    
    //-------------------------------------------------------------- DISPLACE THE PAINT WITH THE ROCK TEXTURE
    paintFboRock.begin();
    displacementShader.begin();
    
    displacementShader.setUniform1f("displacementAmountX", 60.0f);
    displacementShader.setUniform1f("displacementAmountY", 20.0f);
    displacementShader.setUniformTexture("tex0",paintFbo, 0);
    displacementShader.setUniformTexture("tex1",displacer.getTextureReference(), 1);
    paintFbo.draw(0, 0);
    
    displacementShader.end();
    paintFboRock.end();
    //--------------------------------------------------------------
    
    
    //-------------------------------------------------------------- DISPLACE THE PAINT WITH THE NOISE TEXTURE
    paintFboNoise.begin();
    displacementShader.begin();
    
    displacementShader.setUniform1f("displacementAmountX", 1.0f);
    displacementShader.setUniform1f("displacementAmountY", 1.0f);
    displacementShader.setUniformTexture("tex0",paintFboRock, 0);
    displacementShader.setUniformTexture("tex1",noiseFbo.getTextureReference(), 1);
    paintFboRock.draw(0, 0);
    
    displacementShader.end();
    paintFboNoise.end();
    //--------------------------------------------------------------
    
    
    //-------------------------------------------------------------- DETECT EDGES ON THE PAINT LAYER
    edgeDetectFbo.begin();
    
    edgeDetectShader.begin();
    edgeDetectShader.setUniformTexture("tex0", paintFboNoise.getTextureReference(), 0);
    paintFboNoise.draw(0, 0);
    edgeDetectShader.end();
    
    edgeDetectFbo.end();
    //--------------------------------------------------------------
    
    
    paintStrokeShader.begin();
    edgeDetectShader.setUniformTexture("tex0", paintFboNoise.getTextureReference(), 0);
    paintFboNoise.draw(0, 0);
    paintStrokeShader.end();
    
    ofSetColor(255, 255, 255, 10);
    edgeDetectFbo.draw(0, 0);
    
    
    //-------------------------------------------------------------- DISPLACE THE PENCIL WITH THE ROCK TEXTURE
    displacementShader.begin();
    
    displacementShader.setUniform1f("displacementAmountX", 1.0f);
    displacementShader.setUniform1f("displacementAmountY", 1.5f);
    displacementShader.setUniformTexture("tex0",pencilFbo, 0);
    displacementShader.setUniformTexture("tex1",displacer.getTextureReference(), 1);
    pencilFbo.draw(0, 0);
    
    displacementShader.end();
    //--------------------------------------------------------------
    
    if (isDisplayingControls)
    {
        ofSetColor(pencilColor);
        ofDrawBitmapString("Press 'C' to clear the canvas", ofPoint(20, 20));
        ofDrawBitmapString("Press 'S' to save and print a screenshot", ofPoint(20, 40));
        
        ofSetColor(boolToColor(isScratch));
        ofDrawBitmapString("Press '1' to toggle pencil scratches. Currently: " + boolToOffOn(isScratch), ofPoint(20, 60));
        
        ofSetColor(boolToColor(isPaint));
        ofDrawBitmapString("Press '2' to toggle paint strokes. Currently: " + boolToOffOn(isPaint), ofPoint(20, 80));
        
        ofSetColor(boolToColor(isText));
        ofDrawBitmapString("Press '3' to toggle text generation. Currently: " + boolToOffOn(isText), ofPoint(20, 100));
        
        ofSetColor(boolToColor(isCurling));
        ofDrawBitmapString("Press '4' to toggle pencil curling. Currently: " + boolToOffOn(isCurling), ofPoint(20, 120));
        
        ofSetColor(boolToColor(isObjects));
        ofDrawBitmapString("Press '5' to toggle Twombly-esque forms. Currently: " + boolToOffOn(isObjects), ofPoint(20, 140));
        
        ofDrawBitmapString("Framerate: " + ofToString(ofGetFrameRate()) + ", frame counter: " + ofToString(frameCounter), ofPoint(20, 160));
    }
}

string twomblyDrawer::boolToOffOn(bool b) 
{
    return b ? "ON" : "OFF";
}

ofColor twomblyDrawer::boolToColor(bool b)
{
    return b ? pencilColor : ofColor::red;
}

//--------------------------------------------------------------
//              Toggle drawing features
//--------------------------------------------------------------
void twomblyDrawer::toggleScratches()
{
    isScratch = !isScratch;
}

void twomblyDrawer::togglePaint()
{
    isPaint = !isPaint;
}

void twomblyDrawer::toggleText()
{
    isText = !isText;
}

void twomblyDrawer::toggleCurling()
{
    isCurling = !isCurling;
}

void twomblyDrawer::toggleObjects()
{
    isObjects = !isObjects;
}

void twomblyDrawer::toggleDisplayControls()
{
    isDisplayingControls = !isDisplayingControls;
}

//--------------------------------------------------------------
//              Drawing setup
//--------------------------------------------------------------
void twomblyDrawer::startNewLine()
{
    //Optionally choose a new area of the canvas to begin paintings
    if (ofRandom(1.0f) > 0.90f)
    {
        drawPosition.set(ofRandom(DIST_FROM_EDGE, ofGetWidth() - DIST_FROM_EDGE),
                         ofRandom(DIST_FROM_EDGE, ofGetHeight() - DIST_FROM_EDGE));
    }
    
    //Here, we optionally add a number of "secondary" pencil lines that will follow the main stroke
    if (ofRandom(1.0f) > 0.5f)
    {
        drawSecondary = true;
        int numLines = (int)ofRandom(MAX_SECONDARY_LINES);
        for(int i = 0; i < numLines; i++)
        {
            secondaryLine.clear();
            secondaryLine.addVertex(drawPosition.x + ofRandom(MAX_PENCIL_VARIATION),
                                    drawPosition.y + ofRandom(MAX_PENCIL_VARIATION));
            secondaryLines.push_back(secondaryLine);
        }
    }
    else
    {
        drawSecondary = false;
    }
    
    //Now we clear and ready the "main" pencil stroke and choose a paint color
    currentPencilLine.clear();
    currentPencilLine.addVertex(drawPosition.x, drawPosition.y);
    lastDrawPosition.set(drawPosition.x, drawPosition.y);
    paintColor = paintColors[(int) ofRandom(paintColors.size())];
}

void twomblyDrawer::endLine()
{
    //If we were drawing any secondary pencil lines, we add them to the vector of polylines so that we can reference them in draw()
    if(drawSecondary)
    {
        for(int i = 0; i < secondaryLines.size(); i++)
        {
            pencilLines.push_back(secondaryLines[i].getSmoothed(PENCIL_SMOOTHING));
            drawSecondary = false;
        }
        secondaryLines.clear();
    }
    pencilLines.push_back(currentPencilLine.getSmoothed(PENCIL_SMOOTHING));
}

//--------------------------------------------------------------
//              Feature generation
//--------------------------------------------------------------
void twomblyDrawer::generateWords() {
    ofBuffer buffer = ofBufferFromFile("Herodiade.txt");
    
    //Check to see if anything was actually loaded
    if(buffer.size())
    {
        //Get each line of text and find words to add to the dictionary of available words
        while(buffer.isLastLine() == false)
        {
            string line = buffer.getNextLine();
            if(!line.empty()) {
                vector<string> words = ofSplitString(line, " ");
                for(int i = 0; i < words.size(); i++) {
                    int numBackwards = (int)ofRandom(ofClamp(i, 0, 4));
                    if (words[i].size() > MIN_WORD_LENGTH)
                    {
                        string working = words[i];
                        for (int j = 1; j < numBackwards; j++)
                        {
                            working += " " + words[i-j];
                        }
                        twomblyDictionary.push_back(working);
                        cout << "Added \"" + working << "\" to the dictionary.\n";
                    }
                }
            }
        }
        
    }
}

void twomblyDrawer::generateLetterForms() {
    //A vector of polylines that will store all of the loaded letter forms
    vector<ofPolyline> lines;
    
    //Load the XML file
    ofxXmlSettings paths;
    if(paths.loadFile("letters.xml")){
        paths.pushTag("paths");
        
        //Get the number of XML elements labeled "path" under "paths"
        int numberOfSavedPaths = paths.getNumTags("path");
        for(int i = 0; i < numberOfSavedPaths; i++) {
            paths.pushTag("path", i);
            
            //Create a new polyline from this XML element
            ofPolyline pl;
            
            //Get the number of XML elements labeled "position" under this "path"
            int numberOfSavedPoints = paths.getNumTags("position");
            for(int j = 0; j < numberOfSavedPoints; j++) {
                paths.pushTag("position", j);
                
                //Create a new point from this XML element
                ofPoint p;
                p.x = paths.getValue("X", 0.0); //A double
                p.y = paths.getValue("Y", 0.0); //A double
                
                //Add this point to the current polyline
                pl.addVertex(p);
                
                paths.popTag();
            }
            paths.popTag();
            
            //Add this polyline to the vector
            lines.push_back(pl);
        }
        
        //Set this vector of polylines to a global variable
        letterForms = lines;
        cout << "Loaded " + ofToString(letterForms.size()) << " letter forms.\n";
    }
}

void twomblyDrawer::generateObjects() {
    //A vector of polylines that will store all of the loaded letter forms
    vector<ofPolyline> lines;
    
    //Load the XML file
    ofxXmlSettings paths;
    if(paths.loadFile("objects.xml")){
        paths.pushTag("paths");
        
        //Get the number of XML elements labeled "path" under "paths"
        int numberOfSavedPaths = paths.getNumTags("path");
        for(int i = 0; i < numberOfSavedPaths; i++) {
            paths.pushTag("path", i);
            
            //Create a new polyline from this XML element
            ofPolyline pl;
            
            //Get the number of XML elements labeled "position" under this "path"
            int numberOfSavedPoints = paths.getNumTags("position");
            for(int j = 0; j < numberOfSavedPoints; j++) {
                paths.pushTag("position", j);
                
                //Create a new point from this XML element
                ofPoint p;
                p.x = paths.getValue("X", 0.0); //A double
                p.y = paths.getValue("Y", 0.0); //A double
                
                //Add this point to the current polyline
                pl.addVertex(p);
                
                paths.popTag();
            }
            paths.popTag();
            
            //Add this polyline to the vector
            lines.push_back(pl);
        }
        
        //Set this vector of polylines to a global variable
        objectForms = lines;
        cout << "Loaded " + ofToString(objectForms.size()) << " object forms.\n";
    }
}

//--------------------------------------------------------------
//              Drawing functions
//--------------------------------------------------------------
void twomblyDrawer::addPaintSplatter(int x, int y)
{
    ofPoint splatterCenter(x + ofRandom(MAX_PAINT_VARIATION),
                           y + ofRandom(MAX_PAINT_VARIATION));
    int numSplatters = (int) ofRandom(MAX_PAINT_SPLATTERS);
    
    ofSetColor(canvasColor);
    for(int i = 0; i < numSplatters; i++)
    {
        float r = ofRandom(10.0f);
        ofPoint center = ofPoint(splatterCenter.x + ofRandom(-4.0f, 4.0f),
                                 splatterCenter.y + ofRandom(-4.0f, 4.0f));
        brush.draw(center - ofPoint(r * 2, r * 2),
                   r * 4,
                   r * 4);
        ofCircle(center, r);
    }
}

void twomblyDrawer::addPaintStroke(int x, int y, int radius, int variation)
{
    ofPoint center = ofPoint(x + ofRandom(-variation, variation),
                             y + ofRandom(-variation, variation));
    ofSetColor(paintColor, PAINT_ALPHA);
    brush.draw(center - ofPoint(radius * 2, radius * 2),
               radius * 4,
               radius * 4);
    
    ofSetColor(paintColor);
    ofCircle(center, radius);
}

void twomblyDrawer::addScratch(int x, int y)
{
    ofPoint center(x + ofRandom(MAX_SCRATCH_VARIATION), y + ofRandom(MAX_SCRATCH_VARIATION));
    float angle = ofDegToRad(ofRandom(360.0f));
    int alpha = ofRandom(140);
    ofSetColor(pencilColor, alpha);

    int numScratchLines = (int)ofRandom(MIN_SCRATCH_LINES, MAX_SCRATCH_LINES);
    
    //Generate the polylines
    for (int i = 0; i < numScratchLines; i++)
    {
        ofPoint centerOffset(center.x + ofRandom(20), center.y + ofRandom(20));
        
        //Create a new scratch line that will be a part of this mark
        ofPolyline scratchLine;
        int numPoints = SCRATCH_RESOLUTION;
        float length = ofRandom(MAX_SCRATCH_LENGTH);
        float distAlongLength = 0.0;
        float step = length / numPoints;
        
        //Walk along the angled line, generating new points with slightly random offsets
        for (int j = 0; j < numPoints; j++)
        {
            float scratchX = distAlongLength * cos(angle) + centerOffset.x + ofRandom(1.0);
            float scratchY = distAlongLength * sin(angle) + centerOffset.y + ofRandom(1.0);
            distAlongLength += step;
            scratchLine.addVertex(scratchX, scratchY);
        }
        scratchLine.draw();
    }
}

void twomblyDrawer::addTextSnippet(int x, int y) {
    //Choose a random word from the our pre-populated set of phrases
    string randWord = twomblyDictionary[(int)ofRandom(twomblyDictionary.size())];

    //Convert each character of the text to indices [0-25]
    char int_to_char[] = {'a', 'b', 'c', 'd', 'e',
                          'f', 'g', 'h', 'i', 'j',
                          'k', 'l', 'm', 'n', 'o',
                          'p', 'q', 'r', 's', 't',
                          'u', 'v', 'w', 'x', 'y', 'z'};
    
    //Set up a few variables for positioning each letter of the string
    int xOffset = 20;
    int spaceOffset = 0;
    bool foundSpace = false;
    
    ofSetColor(pencilColor, 170);
    
    for(int i = 0; i < randWord.size(); i++)
    {
        char c = tolower(randWord[i]);
        
        //If the character is a space, we need to account for this in our x-spacing
        if (c == ' ')
        {
            foundSpace = true;
            continue;
        }
        
        //Loop through the array of indexed characters above to find a numerical value [0-25] corresponding to the current character
        for (int j = 0; j < sizeof(int_to_char); j++)
        {
            if (int_to_char[j] == c)
            {
                ofPushMatrix();
                ofTranslate(x, y);
                
                if (foundSpace)
                {
                    spaceOffset += xOffset / 2;
                    ofTranslate(xOffset*i + spaceOffset, 0);
                    foundSpace = false;
                }
                else
                {
                    ofTranslate(xOffset*i + spaceOffset, 0);
                    ofRotateZ(ofRandom(-5.0f, 5.0f));
                }
                
                //Draw the polyline and (optionally) draw duplicates of the original
                letterForms[j].draw();
                if (ofRandom(1.0f) > 0.9f)
                {
                    ofPolyline duplicate = letterForms[j].getSmoothed(10.0f);
                    duplicate.draw();
                }
                
                ofPopMatrix();
            }
        }
    }
    
}

void twomblyDrawer::addObjectForm(int x, int y) {
    int index = (int)ofRandom(objectForms.size());
    ofPolyline objectForm = objectForms[index];
    
    for (int i = 0; i < objectForm.size(); i++)
    {
        //Jitter the points of the polyline
        if(ofRandom(1.0f) > 0.95f)
        {
            objectForm[i].x += ofRandom(-3.0f, 3.0f);
            objectForm[i].y += ofRandom(-3.0f, 3.0f);
        }
    }
    
    //Randomly rotate and scale the polyline
    ofSetColor(pencilColor, 170);
    float rotation = ofRandom(-10.0f, 10.0f);
    float scale = ofRandom(0.3f, 1.0f);
    
    ofPushMatrix();
    ofTranslate(x, y);
    ofRotate(rotation);
    ofScale(scale, scale);
    objectForm.draw();
    ofPopMatrix();
}

void twomblyDrawer::clearAll() {
    pencilFbo.begin();
    ofClear(255, 255, 255, 0);
    pencilFbo.end();
    
    paintFbo.begin();
    ofClear(255, 255, 255, 0);
    paintFbo.end();
    
    paintFboRock.begin();
    ofClear(255, 255, 255, 0);
    paintFboRock.end();
    
    paintFboNoise.begin();
    ofClear(255, 255, 255, 0);
    paintFboNoise.end();
    
    edgeDetectFbo.begin();
    ofClear(255, 255, 255, 0);
    edgeDetectFbo.end();
    
    pencilLines.clear();
}
