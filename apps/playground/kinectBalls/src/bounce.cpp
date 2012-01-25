/*
 *  bounce.cpp
 *  bounceKinect
 *
 *  Created by Chris Mullany on 06/01/2011.
 *  Copyright 2011 AllofUs. All rights reserved.
 *
 */

#include "bounce.h"
#include "ofxSimpleGuiToo.h"


//--------------------------------------------------------------
void bounce::setup()
{
    // oF setup
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetVerticalSync(true);
    ofBackgroundHex(0xffffff);
    //ofEnableSmoothing();
    
    //
	// Setup kinect
	kinect.init();
	//kinect.setVerbose(true);
	kinect.open();
    kinectAngle = 0;
	
	//
	// Setup openCV images
	colorImg.allocate(kinect.width, kinect.height);
	liveImg.allocate(kinect.width, kinect.height);
	grayImage.allocate(kinect.width, kinect.height);
	grayThresh.allocate(kinect.width, kinect.height);
	grayThreshFar.allocate(kinect.width, kinect.height);
	
    //
	// Set defaults
	nearThreshold = 50;
	farThreshold  = 180;
    gravity = ofPoint(0, 10);
    doAutoBall = false;
    doBallAtMouse = true;
    doKillBalls = false;
    doDrawPolys = true;
    triangulateSampleSize = 15;
    ballDensity = .1;
    ballBounce = .1;
    ballFriction = .9;
    polyOffset = ofPoint(0,0);
    minBallRad = 5;
    maxBallRad = 12;
    ballLifeMin = 5;
    ballLifeMax = 10;
	
	//
	// Setup Box2d stuff
	box2d.init();
    isBounds = true;
    box2d.createBounds(0, -150, APP_WIDTH, APP_HEIGHT+150);
	box2d.checkBounds(true);
	box2d.setFPS(30.0);
    
    //
    // images
    images.addFolder("images/flatobjects", OF_IMAGE_COLOR_ALPHA);
    bg.allocate(1280, 720, OF_IMAGE_COLOR);
    bg.loadImage("images/backgrounds/bkg.jpg");
    
    //
	// Listeners
	ofAddListener(ofEvents.mouseMoved, this, &bounce::mouseMoved);
	ofAddListener(ofEvents.keyPressed, this, &bounce::keyPressed);
	
    //
    // gui setup
    gui.setFolderName("kinectBalls/");
    gui.setup();
    gui.setDefaultKeys(true);
    setupGUI();
    gui.loadFromXML();
    
}

//--------------------------------------------------------------

void bounce::setupGUI(string name)
{	
    //
	// New page for kinect stuff
	ofxSimpleGuiPage& page = gui.addPage("kinect");
    gui.addSlider("angle", kinectAngle, -27, 27);
	// images and cv
    gui.addTitle("cv");
	gui.addSlider("near threshold", nearThreshold, 0, 255);
	gui.addSlider("far threshold", farThreshold, 0, 255);
    gui.addSlider("grey blur", greyImageBlur, 0, 40);
    gui.addSlider("grey threshold", greyImageThreshold, 0, 255);
	gui.addContent("grey", grayImage);
    gui.addSlider("min blob", minBlobSize, 0, 200);
	gui.addContent("contour", contourFinder);
	//gui.addContent("kinect", kinect.);
	gui.addContent("live", liveImg);
    
    //
    // box2d
    gui.addPage("box2d");
    gui.addSlider2d("gravity", gravity, -10, 10, -10, 10);
    // poly
    gui.addTitle("poly");
    gui.addToggle("draw polys", doDrawPolys);
    gui.addSlider2d("poly offset", polyOffset, -200, 200, -200, 200);
    gui.addSlider("poly sample size", triangulateSampleSize, 1, 20);
    // balls
    gui.addTitle("balls");
    gui.addButton("kill balls", doKillBalls);
    gui.addToggle("auto ball", doAutoBall);
    gui.addToggle("ball at mouse", doBallAtMouse);
    gui.addSlider("ball frame mod", ballFrameMod, 1, 60);
    gui.addSlider("ball density", ballDensity, 0, 2);
    gui.addSlider("ball bounce", ballBounce, 0, 2);
    gui.addSlider("ball friction", ballFriction, 0, 1);
    gui.addSlider("ball rad min", minBallRad, 1, 50).increment = 1.0;
    gui.addSlider("ball rad max", maxBallRad, 1, 50).increment = 1.0;
    gui.addSlider("ball life min", ballLifeMin, 1, 50);
    gui.addSlider("ball life max", ballLifeMax, 1, 50);
    
    gui.addToggle("draw colours", doColour);
    gui.addToggle("draw images", doDrawImage);
    gui.addToggle("draw circles", doCircle);
    
}

//--------------------------------------------------------------
void bounce::update()
{
    
    if(kinectAngle != kinect.getCurrentCameraTiltAngle()) kinect.setCameraTiltAngle(kinectAngle);
    
    // box2d
    if(box2d.gravity != gravity) box2d.setGravity(gravity);
    
    vector<CustomParticle>::iterator it;
    for ( it = customParticles.begin() ; it < customParticles.end(); it++ ) {
        CustomParticle& p = *it;
        p.doCircle = doCircle;
        p.doColour = doColour;
        p.doDrawImage = doDrawImage;
        //p.setPhysics(ballDensity, ballBounce, ballFriction);
        p.update();
        if (p.life > p.maxLife) {
            p.destroy();
            customParticles.erase(it);
        }
	}
    
	// Update GUI buttons
	updateButtons();
	
	// update Box2d
	box2d.update();
	
	// Update kinect
	kinect.update();
	
	// Update images: grey for depth, live for color image
	grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
	liveImg.setFromPixels(kinect.getPixels(), kinect.width, kinect.height);
	
    grayImage.mirror(false, true);
    liveImg.mirror(false, true);
    
	// Thresholds
	// Do a cvAnd to get the pixels which are a union of the two thresholds.
	grayThreshFar = grayImage;
	grayThresh = grayImage;
	grayThreshFar.threshold(farThreshold, true);
	grayThresh.threshold(nearThreshold);
	cvAnd(grayThresh.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
	
	//grayImage.blurHeavily();
    grayImage.blur(greyImageBlur);
    grayImage.threshold(greyImageThreshold);
	
	//update the cv image
    liveImg.flagImageChanged();
	grayImage.flagImageChanged();
    
	if (ofGetFrameNum() % 4 == 0) {
        
        // find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
        // also, find holes is set to false so we will not get interior contours
        contourFinder.findContours(grayImage, 10, (APP_WIDTH*APP_HEIGHT)/2, minBlobSize, true);
        
        float scale = APP_WIDTH/kinect.width;
        
        // for testing, just grab the first blob
        if(contourFinder.nBlobs > 0)
        {
            poly.destroy();
            vector <ofPoint> points = contourFinder.blobs[0].pts;
            int n = points.size();
            for (int i = 0; i<n; i++) {
                poly.addVertex(polyOffset + (points[i]*scale));
            }
            poly.setAsEdge(false);
            poly.triangulate(triangulateSampleSize);
            poly.setPhysics(0, 0, 0);
            poly.create(box2d.getWorld());
        }
        
    }
    
    if (doAutoBall && ofGetFrameNum() % ballFrameMod == 0) {
        addBall();
    }
    
	
}


void bounce::updateButtons(){
    if (doKillBalls) {
        doKillBalls = false;
        for(int i=0; i<customParticles.size(); i++){
            customParticles[i].destroy();
        }
        customParticles.clear();
    }
}


void bounce::draw(){
	
    //
    // centre
    glPushMatrix();
    glTranslatef(ofGetWidth()*.5 - APP_WIDTH*.5, ofGetHeight()*.5 - APP_HEIGHT*.5, 0);
	// Draw the live/camera image
	//liveImg.draw(0, 0);
    ofSetHexColor(0xffffff);
    bg.draw(0,0);
	// Draw Box2d stuff
    // balls with alpha
    ofEnableAlphaBlending();
    vector<CustomParticle>::iterator it;
    for ( it = customParticles.begin() ; it < customParticles.end(); it++ ) {
        CustomParticle& p = *it;
		p.draw();
	}
    ofDisableAlphaBlending();
    // polys
    if (doDrawPolys) {
        ofSetHexColor(0xdddddd);
        ofFill();
        poly.draw();
        ofNoFill();
    }
	// Draw box2d
	box2d.draw();
    glPopMatrix();
    
    
	//
	// debug
	//string info = "";
	//info += "Total Bodies: "+ofToString(box2d.getBodyCount())+"\n";
	//info += "FPS: "+ofToString(ofGetFrameRate())+"\n";
	//ofSetHexColor(0x666666);
	//ofDrawBitmapString(info, 30, 30);
    
    
    //
    // gui
    gui.draw();
}


void bounce::addBall(){
    float x, y;
    if (doBallAtMouse) {
        x = mouseX;
        y = mouseY;
    } else {
        x = ofRandom(20, APP_WIDTH-40);
        y = -100;
    }
    float r2 = ofRandom(minBallRad, maxBallRad);
    CustomParticle p;
    p.setPhysics(ballDensity, ballBounce, ballFriction);
    p.setup(box2d.getWorld(), x, y, r2);
    p.color.r = ofRandom(20, 255);
    p.color.g = ofRandom(20, 255);
    p.color.b = ofRandom(20, 255);
    p.image = &images[ofRandom(0, images.size()-1)];
    p.maxLife = (int)ofRandom(ballLifeMin*60, ballLifeMax*60);
    customParticles.push_back(p);
}


void bounce::exit(){
	kinect.close();
}



void bounce::keyPressed (ofKeyEventArgs& e)
{
	switch (e.key)
	{	
		case 'w':
			kinect.enableDepthNearValueWhite(!kinect.isDepthNearValueWhite());
			break;
			
		case 'b' :
            if (isBounds) {
                isBounds = false;
                box2d.world->DestroyBody(box2d.ground);
                box2d.ground = NULL;
            } else {
                isBounds = true;
                box2d.createBounds(0, -150, APP_WIDTH, APP_HEIGHT+150);
            }
			break;
            
            
        case 'f' :
            ofToggleFullscreen();
            break;
            
        case OF_KEY_LEFT :
            gravity = ofPoint(-10, 0);
            break;    
        case OF_KEY_RIGHT :
            gravity = ofPoint(10, 0);
            break;    
        case OF_KEY_UP :
            gravity = ofPoint(0, -10);
            break;    
        case OF_KEY_DOWN :
            gravity = ofPoint(0, 10);
            break;
            
			
	}
}


void bounce::mouseMoved(ofMouseEventArgs& e){
	mouseX = e.x;
	mouseY = e.y;
}

void bounce::mouseDragged(int x, int y, int button){
    mouseX = x;
	mouseY = y;
    if (!doAutoBall) {
        addBall();
    }
    
}

void bounce::mousePressed(int x, int y, int button){
}

void bounce::mouseReleased(int x, int y, int button){}

void bounce::windowResized(int w, int h){}



