/*
 *  ImageList.cpp
 *  backdrop
 *
 *  Created by Mark on 14/02/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ImageList.h"

ImageList::ImageList(){
	current_ = 0;
}

ImageList::~ImageList(){
}

ofImage& ImageList::operator[](unsigned i) {
	return images[i];
}

ofImage& ImageList::current(){
	return images[current_];
}

ofImage& ImageList::next(){
	current_++;
	if(current_ >= size())
		current_ = 0;
	return images[current_];
}

ofImage& ImageList::previous(){
	current_--;
	if(current_ < 0)
		current_ = size() - 1;
	return images[current_];
}

unsigned ImageList::size() const{
	return images.size();
}

void ImageList::addFolder(string folderpath, ofImageType imageType, int width, int height){
	
	ofDirectory dir;
	int nImages = dir.listDir(folderpath);
	
	for(int i=0; i<nImages; i++){
		ofImage img;	
		//img.setUseTexture(true);
		img.loadImage(dir.getPath(i));
		img.setImageType(imageType);
		if(width != 0 && height != 0){
			img.resize(width, height);
		}
		images.push_back(img);
	}
}

int ImageList::getCurrentIndex(){
    return current_;
}