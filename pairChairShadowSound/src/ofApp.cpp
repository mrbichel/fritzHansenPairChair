#include "ofApp.h"

string fragShaderSrc = R"(
    #version 120

    #ifdef GL_ES
    precision mediump float;
    #endif

    //uniform vec2 u_resolution;

    //uniform vec2 u_mouse;
    uniform float u_time;
    uniform float u_width;
    uniform float u_height;

    varying vec2 texCoordVarying;
    uniform sampler2DRect tex0;


    vec2 random2(vec2 st){
        st = vec2( dot(st,vec2(127.1,311.7)),
                  dot(st,vec2(269.5,183.3)) );
        return -1.0 + 2.0*fract(sin(st)*43758.5453123);
    }

    // Value Noise by Inigo Quilez - iq/2013
    // https://www.shadertoy.com/view/lsf3WH
    float noise(vec2 st) {
        vec2 i = floor(st);
        vec2 f = fract(st);
        
        vec2 u = f*f*(3.0-2.0*f);
        
        return mix( mix( dot( random2(i + vec2(0.0,0.0) ), f - vec2(0.0,0.0) ),
                        dot( random2(i + vec2(1.0,0.0) ), f - vec2(1.0,0.0) ), u.x),
                   mix( dot( random2(i + vec2(0.0,1.0) ), f - vec2(0.0,1.0) ),
                       dot( random2(i + vec2(1.0,1.0) ), f - vec2(1.0,1.0) ), u.x), u.y);
    }

    void main() {
        //vec2 st = gl_FragCoord.xy/u_resolution.xy;
        vec2 st = vec2(gl_FragCoord.x/u_width, gl_FragCoord.y/u_height);
        //st.x *= u_resolution.x/u_resolution.y;
        st.x *= u_width/u_height;
        vec3 color = vec3(0.0);
        
        
        float t = 1.0;
        // Uncomment to animate
        t = abs(1.0-sin(u_time*.1))*5.;
        // Comment and uncomment the following lines:
        st += noise(st*2.)*t; // Animate the coordinate space
        color = vec3(1.) * smoothstep(.18,.2,noise(st)); // Big black drops
        //color += smoothstep(.15,.2,noise(st*10.)); // Black splatter
        //color -= smoothstep(.35,.4,noise(st*10.)); // Holes on splatter
        
        //gl_FragColor = vec4(1.-color,0.5);
        vec4 texel0 = texture2DRect(tex0, texCoordVarying);

        gl_FragColor =  vec4((1-texel0.rgb) * (1-color.rgb), 1);

    }
)";



string vertextShaderSrc = R"(
    #version 120

    varying vec2 texCoordVarying;

    void main()
    {
        texCoordVarying = gl_MultiTexCoord0.xy;
        gl_Position = ftransform();
    }

)";


//--------------------------------------------------------------
void ofApp::setup(){
    ofEnableAlphaBlending();

    ofSetBackgroundAuto(false);

    ofSetBoxResolution( 100, 100, 30 );
    
    fragShader.setupShaderFromSource( GL_VERTEX_SHADER, vertextShaderSrc);
    fragShader.setupShaderFromSource( GL_FRAGMENT_SHADER, fragShaderSrc);
    //fragShader.bindDefaults();
    fragShader.linkProgram();
    
    cam.setAspectRatio(16./9.);

    //fadeManager = make_shared<ofxParameterFadeManager>();
    
    pairChairModel.loadModel("chair.dae");
    //pairChairMesh = pairChairModel.getMesh(0);
    
    for(auto & n : pairChairModel.getMeshNames()) {
        Part p;
        p.mesh = pairChairModel.getMesh(n);
        
        p.explosionDirection = ofVec3f(ofRandom(-1, 1),ofRandom(0.01,0.5),ofRandom(-1,1));
        
        p.setParent(chairNode);
        
        parts.push_back(p);
    }

    //light.setAreaLight(100, 100);
    //light.enable();
    
    // lets say our units are mm
    floor.set(3325, 2845, 10);
    
    // rotate chair to stand on the floor
    // set the scale of the chair
    
    gui.setup(params);
    
    gui.loadFromFile("settings.xml");
    
    shadeFbo.allocate(1920, 1080);
    
    reflectFbo.allocate(1920, 1080);
    reflectFbo.begin();
    //ofBackground(0,0,0,255);
    reflectFbo.end();
    
    float width = ofGetWidth();
    float height = ofGetHeight();
    borders.addVertex( ofPoint(0,0));
    borders.addVertex( ofPoint(width,0));
    borders.addVertex( ofPoint(width, height));
    borders.addVertex( ofPoint(0,height));
    borders.addVertex( ofPoint(0,0));
    
    borders2 = borders;
    borders2 = borders2.getResampledByCount(ofGetWidth());
    
    soundscape.setup();
    
}

ofPoint ofApp::randomPtForSize(ofRectangle rect, int side){
    
    ofPoint aa = ofPoint(rect.x, rect.y);
    ofPoint bb = ofPoint(rect.x + rect.width, rect.y);
    ofPoint cc = ofPoint(rect.x + rect.width, rect.y + rect.height);
    ofPoint dd = ofPoint(rect.x, rect.y + rect.height);
    
    if (side == 0){
        return aa + ofRandom(0,1) * (bb-aa);
    } else if (side == 1){
        return bb + ofRandom(0,1) * (cc-bb);
    } else if (side == 2){
        return cc + ofRandom(0,1) * (dd-cc);
    } else {
        return dd + ofRandom(0,1) * (aa-dd);
    }
    return ofPoint(0,0);
}

ofPoint  reflect(ofPoint vector, ofPoint normal)
{
    return vector - 2 * normal.dot(vector) * normal;
}

//--------------------------------------------------------------
void ofApp::update(){
    
    // timeline control
    
    
    if(!pause) {
        
        float t = time.get();

        time.set(t + ofGetLastFrameTime() );
        if(t > time.getMax()) { // loop
            time.set(time.getMin());
        }
        
        
        if(t < 36) {

            ofVec3f nv = lightPosition.get();
            
            nv.x = ofxeasing::map_clamp(t, 13, 26, -10, 1376, ofxeasing::quart::easeInOut);
            nv.y = ofxeasing::map_clamp(t, 10, 20, -10, 626, ofxeasing::quart::easeInOut);
            nv.z = ofxeasing::map_clamp(t, 0, 10, -940, 200, ofxeasing::quart::easeInOut);
            
            if( t > 26 ) {
                nv.x = ofxeasing::map_clamp(t, 26, 28, 1376, 834, ofxeasing::quart::easeInOut);
                nv.y = ofxeasing::map_clamp(t, 26, 28, 626, -414, ofxeasing::quart::easeInOut);
                
            }
            
            if(t > 29) {
                nv.x = ofxeasing::map_clamp(t, 29, 33, 834, -2289, ofxeasing::quart::easeInOut);
                nv.y = ofxeasing::map_clamp(t, 30, 32, -414, -997, ofxeasing::quart::easeInOut);
                nv.z = ofxeasing::map_clamp(t, 29, 34, 200, ofMap(sin(t*10), -1, 1, -8, 20), ofxeasing::exp::easeIn);

            }
            
            
            if(t > 33) {
                
                nv.x = ofxeasing::map_clamp(t, 33, 34, -2289, -831, ofxeasing::bounce::easeOut);
                nv.y = ofxeasing::map_clamp(t, 33, 34, -997, 84, ofxeasing::bounce::easeOut);
                
            }
            
            if(t > 34) {
                nv.z = ofxeasing::map_clamp(t, 34, 36, ofMap(sin(t*10), -1, 1, -8, 20), ofMap(sin(t*t), -1, 1, -8, 100), ofxeasing::quart::easeOut);
            }
            
            lightPosition.set(nv);
            
        } else if(t < 46) {
            
            float st = t-36;
            
            ofVec3f nv = lightPosition.get();
            
            //nv.x = ofxeasing::map_clamp(st, 13, 26, -10, 1376, ofxeasing::quart::easeInOut);
            //nv.y = ofxeasing::map_clamp(st, 10, 20, -10, 626, ofxeasing::quart::easeInOut);
            nv.z = ofxeasing::map_clamp(st, 0, 1, ofMap(sin(t*t), -1, 1, -8, 100), 4000, ofxeasing::quart::easeOut);

            lightPosition.set(nv);
        }
    }
    
    
    // to leg // 831, 84
    
    // light pos to
    // ofVec3f lightTo(-10, -10, 1251);
    
    //fadeManager->add(new ParameterFade<int>(p, msg.getArgAsInt32(0), fadeTime, easeFn));

    cam.setFov(camFov); //	benq mh741 throw ratio 1.15-1.49
    cam.setPosition(camPos);
    //cam.lookAt(ofVec3f(0,0,0));
    //cam.setupOffAxisViewPortal(ofVec3f(-1860/2, -3330/2, 0), ofVec3f(-1860/2, 3330/2, 0), ofVec3f(1860/2, 3330/2, 0));
    cam.lookAt(ofVec3f(0,0,0));
    
    
    shadow.setRange( rangeMin, rangeMax );

    shadow.setBias(shadowBias);
    shadow.setIntensity(shadowIntensity);
    //light.setPosition(lightPosition);
    
    shadow.setLightPosition(lightPosition);
    
    shadow.setLightLookAt( ofVec3f(0,0,0), ofVec3f(0,-1,0) );
    
    ofPixels pixels;
    shadeFbo.readToPixels(pixels);
    
    contourFinder.setMinAreaRadius(minArea);
    contourFinder.setMaxAreaRadius(maxArea);
    contourFinder.setThreshold(threshold);
    contourFinder.findContours(pixels);
    contourFinder.setFindHoles(holes);
    
    lines = contourFinder.getPolylines();
    
    for (int i= 0; i < lines.size(); i++){
        for (int j = 0; j < lines[i].size(); j++){
            //lines[i][j] += ofPoint(100, 500);
            //lines[i][j] = lines[i]
        }
        lines[i] = lines[i].getResampledBySpacing(4.0);
        lines[i] = lines[i].getSmoothed(10);
    }
    
    chairNode.setOrientation(chairRotation);
    chairNode.setPosition(chairOffset);
    
    for(auto & p : parts) {
        p.explosionFactor = ofVec3f(explodeAmount,explodeAmount,explodeAmount);
        p.autoRotationOffsetVelocity = autoRotationOffsetVelocity;
        p.autoRotationFactor = autoRotationFactor;
    }
    
    soundscape.update();
}

void ofApp::drawReflections() {
    
    //ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    //ofClear(0,0,0,0.1);
    //ofSetColor(0,0,0,1);
    //ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
    
    ofRectangle rect(0,0,ofGetWidth(), ofGetHeight());
    
    
    ofSetColor(255);
    
    for (int z = 0; z < 100; z ++){
        
            //ofPoint a = borders2[ (mouseX+borders2.size()/2) % borders2.size() ]; // //borders2[ mouseX % borders2.size() ];; //borders2[  ]; //ofPoint(0, mouseY); ///randomPtForSize(rect, side);
            
            ofPoint a = borders2[ (mouseX+borders2.size()/2) % borders2.size() ]; // //borders2[ mouseX % borders2.size() ];; //borders2[  ]; //ofPoint(0, mouseY); ///randomPtForSize(rect, side);
        
            ofPoint b = ofPoint(ofGetWidth(), mouseY); ///randomPtForSize(rect, sideb);

            /*if(lines.size() > 0) {
                b = lines[0].getCentroid2D();
            }*/
            
        
            bool bNoMoreIntersects = false;
            int count = 0;
            while (!bNoMoreIntersects && count < 100){
                
                bool bIntersectsWord = false;
                float minDistance = 10000000;
                ofPoint pos;
                ofPoint newAngle;
                float angleIntersection;
                ofPoint perp;
                
                for (int i= 0; i < lines.size(); i++){
                    for (int j = 0; j < lines[i].size()-1; j++){
                        
                        ofPoint intersection;
                        
                        if (ofLineSegmentIntersection(a, b, lines[i][j], lines[i][j+1], intersection)){
                            float distance = (a - intersection).length();
                            if (distance > 0.01){
                                if (distance < minDistance){
                                    minDistance = distance;
                                    pos = intersection;
                                    bIntersectsWord = true;
                                    
                                    
                                    ofPoint diff = lines[i][j+1] - lines[i][j];
                                    //ofPoint perp;
                                    perp.set(diff.y, -diff.x);
                                    
                                    angleIntersection = atan2(perp.y, perp.x);
                                }
                            }
                        }
                    }
                }
                
                if (bIntersectsWord == false){
                    
                    ofMesh temp;
                    temp.setMode(OF_PRIMITIVE_LINES);
                    temp.addColor(ofColor(0,0,0,10));
                    temp.addVertex(a);
                    temp.addColor(ofColor(0,0,0,10));
                    temp.addVertex(b);
                    temp.draw();
                    
                    //ofLine(a,b);
                    bNoMoreIntersects = true;
                } else {
                    ofMesh temp;
                    temp.setMode(OF_PRIMITIVE_LINES);
                    temp.addColor(ofColor(0,0,0,10));
                    temp.addVertex(a);
                    temp.addColor(ofColor(0,0,0,10));
                    temp.addVertex(pos);
                    temp.draw();
                    
                    //ofLine(a, pos);
                    
                    ofPoint diff = pos - a;
                    float angleMe = atan2(diff.y, diff.x);
                    
                    a = pos;
                    
                    float angleDiff = 2*angleIntersection - angleMe;
                    
                    //R = 2W - P
                    
                    diff.normalize();
                    perp.normalize();
                    
                    ofPoint tempPt = reflect(diff, perp);
                    
                    float angle =  atan2(tempPt.y, tempPt.x);
                    //angle += ofRandom(-0.002, 0.002);
                    b = pos + ofPoint(1000 * cos(angle), 1000 * sin(angle));
                    
                    //cout << count << " " << a << " " << b << endl;
                }
                
                count++;
            }
            
            //        
            //        if (bIntersectsMe){
            //            ofSetColor(0,0,0,10);
            //            ofLine(a,b);
            //        } else {
            //            ofSetColor(255,255,255,10);
            //            ofLine(a,b);
            //        }
        }
    //ofDisableBlendMode();
}

void ofApp::drawTunnel() {
    ofSetColor(0,0,0);
    for(int i=0; i<10; i++) {
        for (auto & c : contourFinder.getPolylines()) {
            ofPushMatrix();
            
            ofTranslate(c.getCentroid2D());
            ofScale(
                    (i+1)/10.0,
                    (i+1)/10.0,
                    (i+1)/10.0);
            ofTranslate(-c.getCentroid2D());
            c.getSmoothed((i+1)*20*ofNoise(ofGetElapsedTimef() + i)).draw();
            ofPopMatrix();
        }
    }
}



//--------------------------------------------------------------
void ofApp::draw(){
    
    ofBackground(bgColor.get());
    
    ofEnableDepthTest();
    
    shadow.beginDepthPass();
    renderFloor();
    
    renderModels();
    shadow.endDepthPass();
    
    shadeFbo.begin();
    ofClear(0,0,0);
    shadow.beginRenderPass( cam );
    cam.begin();

    renderFloor();
    
    cam.end();
    shadow.endRenderPass();
    shadeFbo.end();

    ofDisableDepthTest();

    ofSetColor(255);
    
    if(renderShade) {
        cam.begin();
        
        fragShader.begin();
        fragShader.setUniform1f("u_width", 1920);
        fragShader.setUniform1f("u_height", 1080);
        fragShader.setUniform1f("u_time", time.get());
        fragShader.setUniformTexture("tex0", shadeFbo, 1);

        //shadeFbo.draw(0,0);
        //ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
        floor.draw();
        
        fragShader.end();
        
        cam.end();
    }
    
    //contourFinder.draw();
    
    if(renderTunnel) {
        drawTunnel();
    }
    
   //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
   // glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

    if(renderReflection) {
        reflectFbo.begin();
        drawReflections();
        reflectFbo.end();
    
        ofSetColor(255);
        reflectFbo.draw(0,0);
    }
    
    
    if(renderChair) {
        ofEnableDepthTest();
        shadow.beginRenderPass( cam );
        cam.begin();
        renderModels();
        cam.end();
        shadow.endRenderPass();
        ofDisableDepthTest();
    }
    
    //ofSetColor(255);
    //shadow.getDepthTexture().draw(0,0,192,108);
    
    // move to its own window
    gui.draw();
    soundscape.draw();
}//end ofApp::draw()


void ofApp::renderFloor() {
    ofSetColor(bgColor.get());
    floor.draw();
}

void ofApp::renderModels() {
    ofSetColor(255);
    ofPushMatrix(); {
        
        for(auto & p : parts) {
            p.draw();
        }
        
    } ofPopMatrix();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}






