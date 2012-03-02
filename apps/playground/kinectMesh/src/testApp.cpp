#include "testApp.h"
#include "ofxSimpleGuiToo.h"


//--------------------------------------------------------------
void testApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);
	
    // enable depth->rgb image calibration
	kinect.setRegistration(true);
    
	kinect.init();
	//kinect.init(true); // shows infrared instead of RGB video image
	//kinect.init(false, false); // disable video image (faster fps)
	kinect.open();
	
#ifdef USE_TWO_KINECTS
	kinect2.init();
	kinect2.open();
#endif
	
	colorImg.allocate(kinect.width, kinect.height);
	grayImage.allocate(kinect.width, kinect.height);
	grayThreshNear.allocate(kinect.width, kinect.height);
	grayThreshFar.allocate(kinect.width, kinect.height);
	
	nearThreshold = 230;
	farThreshold = 70;
	bThreshWithOpenCV = true;
	
	ofSetFrameRate(60);
	
	// zero the tilt on startup
	angle = 0;
	kinect.setCameraTiltAngle(angle);
	
	// start from the front
	bDrawPointCloud = false;
    
    mesh.columnCount = 320;
    mesh.rowCount = 240;
    mesh.setup(640, 480);
    
    ofEnableSmoothing();
    
    
    // lights
    ofSetGlobalAmbientColor(ofColor(0,0,0,1));
    ofSetSmoothLighting(true);
    for (int i = 0; i < LIGHT_COUNT; i++) {
        lightConfigs[i].isEnabled = false;
        lightConfigs[i].type = LIGHT_TYPE_DIR;
        lightConfigs[i].pos = ofVec3f(0,0,0);
        lightConfigs[i].target = ofVec3f(0,0,0);
        lightConfigs[i].concentration = 0;
        lightConfigs[i].cutoff = 0;
        lightConfigs[i].attenuationConst = 0;
        lightConfigs[i].lightColorAmbient = ofFloatColor(0,0,0);
        lightConfigs[i].lightColorDiffuse = ofFloatColor(1,1,1);
        lightConfigs[i].lightSpecular = ofFloatColor(1,1,1);
    }
    
    
    
    //
    // GUI
    //
    gui.setDefaultKeys(true);
    gui.addToggle("lights enabled", areLightsEnabled);
    //lights
    string LightTypeLabels[] = {"directional","spot","point"};
    for (int i = 0; i < LIGHT_COUNT; i++) {
        gui.addPage( ofToString(i) + " light" );
        gui.addToggle( ofToString(i) + " light enabled", lightConfigs[i].isEnabled);
        gui.addComboBox(ofToString(i) + " light type", lightConfigs[i].type, 3,  LightTypeLabels);
        gui.addSlider( ofToString(i) + " pos x", lightConfigs[i].pos.x, -2000, 2000);
        gui.addSlider( ofToString(i) + " pos y", lightConfigs[i].pos.y, -2000, 2000);
        gui.addSlider( ofToString(i) + " pos z", lightConfigs[i].pos.z, -2000, 2000);
        gui.addSlider( ofToString(i) + " target x", lightConfigs[i].target.x, -1000, 1000);
        gui.addSlider( ofToString(i) + " target y", lightConfigs[i].target.y, -1000, 1000);
        gui.addSlider( ofToString(i) + " target z", lightConfigs[i].target.z, -1000, 1000);
        gui.addSlider( ofToString(i) + " conc", lightConfigs[i].concentration, 0, 100);
        gui.addSlider( ofToString(i) + " cutoff", lightConfigs[i].cutoff, 0, 100);
        gui.addSlider( ofToString(i) + " attenuation const", lightConfigs[i].attenuationConst, 0, 2);
        gui.addColorPicker( ofToString(i) + " colour ambient", &lightConfigs[i].lightColorAmbient.r);
        gui.addColorPicker( ofToString(i) + " colour diffuse", &lightConfigs[i].lightColorDiffuse.r);
        gui.addColorPicker( ofToString(i) + " colour specular", &lightConfigs[i].lightSpecular.r);
    }
}

//--------------------------------------------------------------
void testApp::update() {
	
	ofBackground(255, 255, 255);
	
	kinect.update();
	
	// there is a new frame and we are connected
	if(kinect.isFrameNew()) {
		
		// load grayscale depth image from the kinect source
		grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
		
		// we do two thresholds - one for the far plane and one for the near plane
		// we then do a cvAnd to get the pixels which are a union of the two thresholds
		if(bThreshWithOpenCV) {
			grayThreshNear = grayImage;
			grayThreshFar = grayImage;
			grayThreshNear.threshold(nearThreshold, true);
			grayThreshFar.threshold(farThreshold);
			cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
		} else {
			
			// or we do it ourselves - show people how they can work with the pixels
			unsigned char * pix = grayImage.getPixels();
			
			int numPixels = grayImage.getWidth() * grayImage.getHeight();
			for(int i = 0; i < numPixels; i++) {
				if(pix[i] < nearThreshold && pix[i] > farThreshold) {
					pix[i] = 255;
				} else {
					pix[i] = 0;
				}
			}
		}
		
		// update the cv images
		grayImage.flagImageChanged();
		
		// find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
		// also, find holes is set to true so we will get interior contours as well....
		contourFinder.findContours(grayImage, 10, (kinect.width*kinect.height)/2, 20, false);
	}
	
#ifdef USE_TWO_KINECTS
	kinect2.update();
#endif
}

//--------------------------------------------------------------
void testApp::draw() {
	
	ofSetColor(255, 255, 255);
	
	if(bDrawPointCloud) {
		easyCam.begin();
		
        startLights();
        
        //drawPointCloud();
        
        
        ofPushMatrix();
        // the projected points are 'upside down' and 'backwards' 
        ofScale(1, -1, -1);
        ofTranslate(0, 0, -1000);
        
        glEnable(GL_DEPTH_TEST);
        float lastZ = 1500;
        
        for(int i=0; i<mesh.mesh.getNumVertices(); i++) {
            ofVec3f mV = mesh.mesh.getVertex(i);
            mV.z = kinect.getDistanceAt(mV.x, mV.y);
            if (mV.z > 1500 || mV.z <= 0) {
                mV.z = lastZ;
                mesh.mesh.setColor(i, ofFloatColor(1,1,1));
            } else {
                lastZ = mV.z;
                mesh.mesh.setColor(i, kinect.getColorAt(mV.x, mV.y));
            }
            mesh.mesh.setVertex(i, mV);
            
            
            //ofLogVerbose("z: " + ofToString(mV.z));
        }
        //mesh.doDrawWireframe = true;
        //mesh.doDrawWireframe = false;
        mesh.doDrawPlane = true;
        mesh.draw();
        //mesh.mesh.draw();
        glDisable(GL_DEPTH_TEST);
        ofPopMatrix();
        
        endLights();
        
		easyCam.end();
	} else {
		// draw from the live kinect
		kinect.drawDepth(10, 10, 400, 300);
		kinect.draw(420, 10, 400, 300);
		
		grayImage.draw(10, 320, 400, 300);
		contourFinder.draw(10, 320, 400, 300);
		
#ifdef USE_TWO_KINECTS
		kinect2.draw(420, 320, 400, 300);
#endif
	}
	
	// draw instructions
    /*
	ofSetColor(255, 255, 255);
	stringstream reportStream;
	reportStream << "accel is: " << ofToString(kinect.getMksAccel().x, 2) << " / "
	<< ofToString(kinect.getMksAccel().y, 2) << " / "
	<< ofToString(kinect.getMksAccel().z, 2) << endl
	<< "press p to switch between images and point cloud, rotate the point cloud with the mouse" << endl
	<< "using opencv threshold = " << bThreshWithOpenCV <<" (press spacebar)" << endl
	<< "set near threshold " << nearThreshold << " (press: + -)" << endl
	<< "set far threshold " << farThreshold << " (press: < >) num blobs found " << contourFinder.nBlobs
	<< ", fps: " << ofGetFrameRate() << endl
	<< "press c to close the connection and o to open it again, connection is: " << kinect.isConnected() << endl
	<< "press UP and DOWN to change the tilt angle: " << angle << " degrees" << endl;
	ofDrawBitmapString(reportStream.str(),20,652);
    */
    
    gui.draw();
}

void testApp::drawPointCloud() {
	int w = 640;
	int h = 480;
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_POINTS);
    //mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
	int step = 2;
	for(int y = 0; y < h; y += step) {
		for(int x = 0; x < w; x += step) {
			if(kinect.getDistanceAt(x, y) > 0) {
				mesh.addColor(kinect.getColorAt(x,y));
				mesh.addVertex(kinect.getWorldCoordinateAt(x, y));
			}
		}
	}
    //mesh.setupIndicesAuto();
	glPointSize(3);
	ofPushMatrix();
	// the projected points are 'upside down' and 'backwards' 
	ofScale(1, -1, -1);
	ofTranslate(0, 0, -1000); // center the points a bit
	glEnable(GL_DEPTH_TEST);
	mesh.drawVertices();
    mesh.draw();
	glDisable(GL_DEPTH_TEST);
	ofPopMatrix();
}

void testApp::startLights(){
    //
    // lights
    if (areLightsEnabled) {
        ofEnableLighting();
        //glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
        for (int i = 0; i < LIGHT_COUNT; i++) {
            if (lightConfigs[i].isEnabled) {
                lights[i].enable();
                // Set light type
                switch (lightConfigs[i].type) {
                    case LIGHT_TYPE_DIR:
                        lights[i].setDirectional();
                        break;
                    case LIGHT_TYPE_SPOT:
                        lights[i].setSpotlight();
                        lights[i].lookAt(lightConfigs[i].target);
                        lights[i].setSpotConcentration(lightConfigs[i].concentration);
                        lights[i].setSpotlightCutOff(lightConfigs[i].cutoff);
                        lights[i].setAttenuation(lightConfigs[i].attenuationConst, 0, 0);
                        break;
                    case LIGHT_TYPE_POINT:
                        lights[i].setPointLight();
                        break;
                    default:
                        lights[i].setDirectional();
                        break;
                }
                // set pos and colours
                lights[i].setPosition(lightConfigs[i].pos);
                lights[i].setAmbientColor(lightConfigs[i].lightColorAmbient);
                lights[i].setDiffuseColor(lightConfigs[i].lightColorDiffuse);
                lights[i].setSpecularColor(lightConfigs[i].lightSpecular);
            } else {
                // light is disabled
                lights[i].disable();
            }
        }
    } else {
        ofDisableLighting();
    }
}

void testApp::endLights(){
    if (areLightsEnabled) {
        ofDisableLighting();
    }
}

//--------------------------------------------------------------
void testApp::exit() {
	kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();
	
#ifdef USE_TWO_KINECTS
	kinect2.close();
#endif
}

//--------------------------------------------------------------
void testApp::keyPressed (int key) {
	switch (key) {
		case ' ':
			bThreshWithOpenCV = !bThreshWithOpenCV;
			break;
			
		case'p':
			bDrawPointCloud = !bDrawPointCloud;
			break;
			
		case '>':
		case '.':
			farThreshold ++;
			if (farThreshold > 255) farThreshold = 255;
			break;
			
		case '<':
		case ',':
			farThreshold --;
			if (farThreshold < 0) farThreshold = 0;
			break;
			
		case '+':
		case '=':
			nearThreshold ++;
			if (nearThreshold > 255) nearThreshold = 255;
			break;
			
		case '-':
			nearThreshold --;
			if (nearThreshold < 0) nearThreshold = 0;
			break;
			
		case 'w':
			kinect.enableDepthNearValueWhite(!kinect.isDepthNearValueWhite());
			break;
			
		case 'o':
			kinect.setCameraTiltAngle(angle); // go back to prev tilt
			kinect.open();
			break;
			
		case 'c':
			kinect.setCameraTiltAngle(0); // zero the tilt
			kinect.close();
			break;
			
		case OF_KEY_UP:
			angle++;
			if(angle>30) angle=30;
			kinect.setCameraTiltAngle(angle);
			break;
			
		case OF_KEY_DOWN:
			angle--;
			if(angle<-30) angle=-30;
			kinect.setCameraTiltAngle(angle);
			break;
	}
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h)
{}
