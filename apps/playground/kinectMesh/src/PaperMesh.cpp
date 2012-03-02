//
//  PaperMesh.cpp
//  furnitureMaterials
//
//  Created by Chris Mullany on 21/10/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "PaperMesh.h"


// Constructor / Destructor //////////////////////////////

PaperMesh::PaperMesh(){
    //
    // defaults
    //
    // layout, dimensions
    columnCount = 5.0;
    rowCount = 5.0;
    width = 500.0;
    height = 500.0;
    maxZDeform = 150;
    deformChance = 0.6;
    
}

PaperMesh::~PaperMesh(){}


// Public Methods ////////////////////////////////////////


void PaperMesh::setup(float width, float height){
    this->width = width;
    this->height = height;
    setup();
}

void PaperMesh::setup(){
    
    // Meshes will be managed by a pool so need to clear up and reset themselves
    
    // state
    progress = 0;
    doExpand = doContract = doDrawPlane = false;
    doDrawWireframe = true;
    doDrawTexture =  doDrawPoints = false;
    doGenerateNewMesh = false;
    
    // setup  the mesh
    setupMesh();
}


void PaperMesh::setupMesh(){
    
    //
    // setup  the mesh
    //
    pointsCount = columnCount * rowCount;
    mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
    mesh.setUsage(GL_DYNAMIC_DRAW);
    
    mesh.clear();
    
    // create vertex start/target points and setup the mesh
    float x,y,z,zRange,rowPi,colPi,fromCentre, zRangeMin;
    int count = 0;
    ofVec3f attractor(width*0.5,height*0.5,-150);
    // rows and columns
    for(int row=0; row < rowCount; row++){
        for (int col=0; col < columnCount; col++) {
            
            // set current x y z
            x = width * (col/(columnCount-1));
            y = height * (row/(rowCount-1));
            z = zRange = 0;
            
            // target points are flat plane
            //targetPoints[count] = ofVec3f(x,y,z);
            //startPoints[count] = ofVec3f(x,y,z);
            
            // mesh
            // add colour, vertex and texture coord
            mesh.addColor(ofFloatColor(1,1,1,1));
            //mesh.addNormal(ofVec3f(0, 0, 1));
            mesh.addVertex(ofVec3f(x, y, z));
            
            mesh.addTexCoord(ofVec2f( col/(columnCount-1), row/(rowCount-1)) ); 
            count++;
        }
    }
    
    
    // add indices to mesh for triangle strip generation
    int n = 0;  
    int colSteps = columnCount * 2;  
    int rowSteps = rowCount - 1;  
    vector<ofIndexType> indices;
    for ( int r = 0; r < rowSteps; r++ ) {  
        for ( int c = 0; c < colSteps; c++ ) {  
            int t = c + r * colSteps;  
            if ( c == colSteps - 1 ) {  
                indices.push_back( n );
            } else {  
                indices.push_back( n );
                ( t%2 == 0 ) ? n += columnCount : (r%2 == 0) ? n -= (columnCount-1) : n -= (columnCount+1);  
            }  
        }  
    }
    mesh.addIndices(indices);
    
}


void PaperMesh::update(){
    
    if(doGenerateNewMesh){
        doGenerateNewMesh = false;
        mesh.clear();
        setupMesh();
    }
    
    if(doExpand) {
        ofLogVerbose("doExpand");
        doExpand = false;
    }
    
    if (doContract) {
        ofLogVerbose("doContract");
        doContract = false;
    }
    /*
    for(int i=0; i<mesh.getNumVertices(); i++) {
        ofVec3f v;
        v.x = ofMap(progress, 0, 1, startPoints[i].x, targetPoints[i].x);
        v.y = ofMap(progress, 0, 1, startPoints[i].y, targetPoints[i].y);
        v.z = ofMap(progress, 0, 1, startPoints[i].z, targetPoints[i].z);
        
        ofVec3f mV = mesh.getVertex(i);
        if (mV.x != v.x || mV.y != v.y || mV.z != v.z) {
            mesh.setVertex(i, v);
        }
    }
     */
    //if (!mesh.haveVertsChanged()) ofLogVerbose("mesh static");
    
}


void PaperMesh::draw(){
    drawMesh();
}

void PaperMesh::drawMesh(){
    //glPushMatrix();
    // need to invert the texture when using ofCamera
    //glScalef(1, -1, -1);
    // centre the mesh
    glTranslatef(-width*.5, -height*.5, 0);
    // bind texture and draw in planes, wireframe and points
    if (doDrawTexture) texture->bind();
    if (doDrawPlane) mesh.draw();
    if (doDrawWireframe) mesh.drawWireframe();
    if (doDrawPoints) mesh.drawVertices();
    if (doDrawTexture) texture->unbind();
    //glPopMatrix();
}

// Handlers //////////////////////////////////////////////
// Private Methods ///////////////////////////////////////
