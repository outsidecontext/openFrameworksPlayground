/*
 *  bounce.h
 *  bounceKinect
 *
 *  Created by Chris Mullany on 06/01/2011.
 *  Copyright 2011 AllofUs. All rights reserved.
 *
 */

#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"
#include "ofxBox2d.h"

// ------------------------------------------------- a simple extended box2d circle
class CustomParticle : public ofxBox2dCircle {
	
public:
	CustomParticle() {
	}
	ofColor color;
	void draw() {
		float radius = getRadius();
		
		glPushMatrix();
		glTranslatef(getPosition().x, getPosition().y, 0);
		
		ofSetColor(color.r, color.g, color.b);
		ofFill();
		ofCircle(0, 0, radius);	
		
		glPopMatrix();
		
	}
};

// -------------------------------------------------

class bounce : public ofBaseApp
{
	
public:
	
	
	
	// Methods //
	
	// Base methods
	void setup();
	void update();
	void draw();
	void exit();
	
	// Base events
	void keyPressed  (ofKeyEventArgs& e);
	void mouseMoved(ofMouseEventArgs& e);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	
	// GUI methods
	void setupGUI(string name="");
	void updateButtons();
	
	
	
	// Fields //
	
	// Mouse point
	float mouseX;
	float mouseY;
	
	// The kinect object
	ofxKinect kinect;
    float kinectAngle;
	
	// Open CV images for tracking depth and storing a clean plate
	ofxCvColorImage	colorImg;
	ofxCvColorImage	liveImg;
	ofxCvGrayscaleImage grayImage;
	ofxCvGrayscaleImage grayThresh;
	ofxCvGrayscaleImage grayThreshFar;
    int minBlobSize;
	
	
	// For Contour analysis
	ofxCvContourFinder contourFinder;
	
	// For threshold analysis
	bool bThreshWithOpenCV;
	int nearThreshold;
	int farThreshold;
    int greyImageBlur, greyImageThreshold;
	
	// Box2d
	ofxBox2d box2d;
	vector<ofxBox2dCircle> circles;
	vector<ofxBox2dPolygon>	polygons;
	vector<ofxBox2dRect> boxes;
	vector<CustomParticle> customParticles;
	
    ofPoint polyOffset;
	ofxBox2dPolygon poly;
    int triangulateSampleSize;
    
    bool doAutoBall, doBallAtMouse;
	
	
};
