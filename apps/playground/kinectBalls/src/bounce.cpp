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
	// Setup kinect
	kinect.init();
	//kinect.setVerbose(true);
	kinect.open();
    kinectAngle = 0;
	
	
	// Setup openCV images
	colorImg.allocate(kinect.width, kinect.height);
	liveImg.allocate(kinect.width, kinect.height);
	grayImage.allocate(kinect.width, kinect.height);
	grayThresh.allocate(kinect.width, kinect.height);
	grayThreshFar.allocate(kinect.width, kinect.height);
	
	// Set thresholds
	nearThreshold = 50;
	farThreshold  = 180;
	
	
	// Setup Box2d stuff
	box2d.init();
	box2d.setGravity(0, 10);
    box2d.createGround();
	box2d.checkBounds(true);
	box2d.setFPS(30.0);
    
    doAutoBall = false;
    doBallAtMouse = true;
    
    triangulateSampleSize = 15;
    
	// Listeners
	ofAddListener(ofEvents.mouseMoved, this, &bounce::mouseMoved);
	ofAddListener(ofEvents.keyPressed, this, &bounce::keyPressed);
	
	
	// oF setup
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetVerticalSync(true);
    ofBackgroundHex(0xffffff);
    ofEnableSmoothing();
    
    gui.setFolderName("kinectBalls/");
    gui.setup();
    gui.setDefaultKeys(true);
    setupGUI();
}

//--------------------------------------------------------------

void bounce::setupGUI(string name)
{	
	// New page for kinect stuff
    
	ofxSimpleGuiPage& page = gui.addPage("kinect");
    gui.addSlider("angle", kinectAngle, -27, 27);
	gui.addSlider("near threshold", nearThreshold, 0, 255);
	gui.addSlider("far threshold", farThreshold, 0, 255);
    
    gui.addSlider("min blob", minBlobSize, 0, 200);
    gui.addSlider2d("poly offset", polyOffset, 0, 500, 0, 500);
    gui.addSlider("poly tri sample", triangulateSampleSize, 1, 20);
    
    // balls
    gui.addToggle("auto ball", doAutoBall);
    gui.addToggle("ball at mouse", doBallAtMouse);
    
	
	// Draw Depth, live kinect camera, greyscale and countour images captured from kinect
    gui.addTitle("images").newColumn = true;
    gui.addSlider("grey blur", greyImageBlur, 0, 40);
    gui.addSlider("grey threshold", greyImageThreshold, 0, 255);
	gui.addContent("grey", grayImage, 256);
	gui.addContent("contour", contourFinder, 256);
	//gui.addContent("kinect", kinect, 256);
	gui.addContent("live", liveImg, 256);
	
    
	// Uncomment if post load actions are required
	page.loadFromXML();
	// do post load actions here
    
}

//--------------------------------------------------------------
void bounce::update()
{
    
    if(kinectAngle != kinect.getCurrentCameraTiltAngle()) kinect.setCameraTiltAngle(kinectAngle);
    
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
	grayImage.flagImageChanged();
    
	if (ofGetFrameNum() % 2 == 0) {
        
        
        // find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
        // also, find holes is set to false so we will not get interior contours
        contourFinder.findContours(grayImage, 10, (kinect.width*kinect.height)/2, minBlobSize, true);
        
        // for testing, just grab the first blob
        if(contourFinder.nBlobs > 0)
        {
            poly.destroy();
            vector <ofPoint> points = contourFinder.blobs[0].pts;
            int n = points.size();
            for (int i = 0; i<n; i++) {
                poly.addVertex(polyOffset + points[i]);
            }
            poly.setAsEdge(false);
            poly.triangulate(triangulateSampleSize);
            poly.setPhysics(0, 0, 0);
            poly.create(box2d.getWorld());
        }
        
        if(doAutoBall){
            float x, y;
            if (doBallAtMouse) {
                x = mouseX;
                y = mouseY;
            } else {
                x = ofRandom(0, ofGetWidth());
                y = -10;
            }
            float r2 = ofRandom(5, 12);
            CustomParticle p;
            p.setPhysics(0.1, 0.1, 0.9);
            p.setup(box2d.getWorld(), x, y, r2);
            p.color.r = ofRandom(20, 255);
            p.color.g = ofRandom(20, 255);
            p.color.b = ofRandom(20, 255);
            customParticles.push_back(p);
        }
        
    }
    
	
}


void bounce::updateButtons()
{
}


void bounce::draw()
{
	
	// Draw the live/camera image
	//liveImg.draw(0, 0, ofGetWidth(), ofGetHeight());
	
	
	// Draw Box2d stuff
	// Draw the custom particles
	for(int i=0; i<customParticles.size(); i++)
	{
		customParticles[i].draw();
	}
    ofFill();
    ofSetHexColor(0x666666);
    poly.draw();
    ofNoFill();
    
	// Draw box2d
	box2d.draw();
    
	
	// Debug info
    
	string info = "";
	info += "Total Bodies: "+ofToString(box2d.getBodyCount())+"\n";
	//info += "FPS: "+ofToString(ofGetFrameRate())+"\n";
	ofSetHexColor(0x666666);
	ofDrawBitmapString(info, 30, 30);
    
    
    gui.draw();
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
			//ofLog(OF_LOG_VERBOSE, "DRAW PARTICLE");
            float x, y;
            if (doBallAtMouse) {
                x = mouseX;
                y = mouseY;
            } else {
                x = ofRandom(0, ofGetWidth());
                y = -10;
            }
			float r2 = ofRandom(5, 12);
			CustomParticle p;
			p.setPhysics(0.1, 0.1, 0.9);
			p.setup(box2d.getWorld(), x, y, r2);
			p.color.r = ofRandom(20, 255);
			p.color.g = ofRandom(20, 255);
			p.color.b = ofRandom(20, 255);
			customParticles.push_back(p);
			break;
			
	}
}

//--------------------------------------------------------------
void bounce::mouseMoved(ofMouseEventArgs& e)
{
	//ofLog(OF_LOG_VERBOSE, ofToString(e.x, 1));
	mouseX = e.x;
	mouseY = e.y;
}

//--------------------------------------------------------------
void bounce::mouseDragged(int x, int y, int button)
{}

//--------------------------------------------------------------
void bounce::mousePressed(int x, int y, int button)
{}

//--------------------------------------------------------------
void bounce::mouseReleased(int x, int y, int button)
{}

//--------------------------------------------------------------
void bounce::windowResized(int w, int h)
{}



