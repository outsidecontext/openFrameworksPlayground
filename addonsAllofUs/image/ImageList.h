/*
 *  ImageList.h
 *  
 *
 *  Created by Mark Hauenstein on 14/02/2010.
 *  Updated by Chris Mullany on 20/10/2011 for oF 007 compatability
 *  Copyright 2010 AllofUs. All rights reserved.
 *
 */

#pragma once
#include "ofMain.h"

class ImageList {

public:
	
	ImageList();
	~ImageList();
	
	vector<ofImage> images;
	
	ofImage& operator[](unsigned i);
	
	// following three methods will cause an error if the images vector does not contain any images!!!
	ofImage& next(); // increments current and returns current
	ofImage& previous(); // decrements current and returns current
	ofImage& current(); // returns current image

	unsigned size() const;
	
	void addFolder(string folderpath,  ofImageType imageType = OF_IMAGE_COLOR, int width = 0, int height = 0);
    
    int getCurrentIndex();
	
private:
	int current_;
	
};