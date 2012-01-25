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
#include "ImageList.h"


#define APP_WIDTH 1280
#define APP_HEIGHT 720


class CustomParticle : public ofxBox2dCircle {
	
public:
    
    bool doColour, doDrawImage, doCircle;
    ofColor color;
    ofImage* image;
    int life, maxLife;
    
	CustomParticle() {
        image = NULL;
        doColour = true;
        doDrawImage = true;
        doCircle = false;
        life = 0;
        maxLife = 240;
	}
    
    void update() {
        life++;
    }
    
	void draw() {
        
		float radius = getRadius();
		
		glPushMatrix();
		glTranslatef(getPosition().x, getPosition().y, 0);
        glRotatef(getRotation(), 0, 0, 1);
		
        if (doColour) {
            ofSetColor(color.r, color.g, color.b);
        }
		
        if (doCircle) {
            ofFill();
            ofCircle(0, 0, radius);
        }
        
        if (doDrawImage &&  image != NULL) {
            image->draw(-radius, -radius, radius*2, radius*2);
        }
        
        //ofDrawBitmapString(ofToString(life), 0, 0);
		
		glPopMatrix();
		
	}
};


class bounce : public ofBaseApp {
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
	
    
    
    
private:
    
    
    // Methods //
    void addBall();
    
    
    // Fields //
	
	// Mouse point
	float mouseX;
	float mouseY;
	
	// kinect
	ofxKinect kinect;
    float kinectAngle;
	
	// Open CV
	ofxCvColorImage	colorImg;
	ofxCvColorImage	liveImg;
	ofxCvGrayscaleImage grayImage;
	ofxCvGrayscaleImage grayThresh;
	ofxCvGrayscaleImage grayThreshFar;
    int minBlobSize;
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
	
    ofPoint gravity;
    ofPoint polyOffset;
	ofxBox2dPolygon poly;
    int triangulateSampleSize, ballFrameMod, ballLifeMin, ballLifeMax;
    bool doAutoBall, doBallAtMouse, doKillBalls, isBounds;
    bool doColour, doDrawImage, doCircle, doDrawPolys;
    float ballDensity, ballBounce, ballFriction, minBallRad, maxBallRad;
    
    //
    // images
    ImageList images;
    ofImage bg;
	
	
};
