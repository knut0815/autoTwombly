#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    ofEnableAlphaBlending();
    ofSetWindowTitle("Twombly Generator");
    m_drawer.setup();
 }

//--------------------------------------------------------------
void ofApp::update()
{
    m_drawer.update();
}

//--------------------------------------------------------------
void ofApp::draw()
{
    m_drawer.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch (key) {
        case 's':
            //m_drawer.saveScreen();
            break;
        case 'd':
            m_drawer.toggleDisplayControls();
            break;
        case 'c':
            m_drawer.clearAll();
            break;
        case '1':
            m_drawer.toggleScratches();
            break;
        case '2':
            m_drawer.togglePaint();
            break;
        case '3':
            m_drawer.toggleText();
            break;
        case '4':
            m_drawer.toggleCurling();
            break;
        case '5':
            m_drawer.toggleObjects();
            break;
        default:
            break;
    }
}

