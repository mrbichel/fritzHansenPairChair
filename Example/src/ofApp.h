#pragma once

#include "ofMain.h"
#include "ofxShadowSimple.h"
#include "ofxAssimpModelLoader.h"
#include "ofxGui.h"

class MyChairPart {
public:
    ofVec3f offset;
    ofMesh mesh;
    
    MyChairPart(ofMesh m) {
        mesh = m;
    }

    void draw(){
        ofPushMatrix();
        ofTranslate(offset);
        mesh.draw();
        ofPopMatrix();
    };
    
};

class ofApp : public ofBaseApp{
public:
    void setup();
    void update();
    void draw();
    
    void renderScene(bool isDepthPass);
    void prepareExplodedParts();
    
    void explodeButtonPressed();
    void exit();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    
    ofShader initFragShader();
    ofShader beginFragShader(ofShader shader);
    ofShader fragShader;
    
    ofEasyCam cam;
    
    ofxShadowSimple shadow;
    
    
    ofFbo outFbo;
    
    ofxAssimpModelLoader chairModel;
    ofMesh chairMesh;

    
    //move or refactor into a seperate class
    
    ofxAssimpModelLoader chairBack;
    ofxAssimpModelLoader chairSeat;
    ofxAssimpModelLoader chairBase;
    ofxAssimpModelLoader chairLegs;
    ofxAssimpModelLoader chairFeet;
    int nParts = 5; //number of chair parts
    
    //gah, how to make nParts a static constant?
    //ofxAssimpModelLoader parts[5];
    vector<MyChairPart> parts;
    
    ofPoint partsPos[5]; //current postions of the parts
    ofVec3f partsVec[5];
    
    ofPoint partsPosExplode[5];
    ofVec3f partsVecExplode[5];
    
    ofPoint partsPosInit[5];
    ofVec3f partsVecInit[5];
    
    ofxButton goExplode;
    
    //endmove
    
    
    ofxPanel gui;
    ofParameter<ofVec3f> guiChairRotation {"Rotation", ofVec3f(20,0,0), ofVec3f(0,0,0), ofVec3f(360,360,360)};
    
    
    ofParameter<ofVec3f> guiChairOffset {"Offset",
        ofVec3f(0,0,0),
        ofVec3f(-400,-400,-400),
        ofVec3f(400,400,400)
    };

    ofParameter<ofVec3f> guiChairScale {"Scale",
        ofVec3f(0.015,0.015,0.015),
        ofVec3f(-0.5,-0.5,-0.5),
        ofVec3f(0.5,0.5,0.5)
    };

    
    ofParameter<ofVec3f> lightOffset {"Offset",
        ofVec3f(0,0,0),
        ofVec3f(-400,-400,-400),
        ofVec3f(400,400,400)
    };
    
    ofxFloatSlider explosionRadius;
    ofxFloatSlider explosionSpeed;
    
    
    ofParameterGroup chairParams {"Chair",
            guiChairRotation,
            guiChairOffset,
            guiChairScale
        };
    
    ofParameterGroup lightingParams {"Lighting",
        lightOffset
    };
    
    ofxToggle displayModel, displayParts, explodeParts ;

    ofParameterGroup params {"params",
        chairParams,
        lightingParams,
    };
    
    
    



};
