
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <CanvasPoint.h>
#include <Colour.h>
#include <CanvasTriangle.h>
#include <TextureMap.h>
#include <TexturePoint.h>
#include <ModelTriangle.h>
#include <sstream> 
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <RayTriangleIntersection.h>

//320,240

#define WIDTH 320
#define HEIGHT 240
enum RenderMode { WIREFRAME, RASTERIZING, RAYTRACING };
RenderMode rendermode = RASTERIZING;

Colour BLACK(255,255,255);


std::vector<std::vector<float>> depthbuffer;
glm::vec3 cameraposition(0.0, 0.0, 4.0);
float camposchange = 0.1;
float focallength = 2.0 ;
glm::vec3 lightsource(0.0, 0.85, 0.0);

std::string mappath;

float rotchange = glm::radians(0.4);
glm::mat3 rotmatrix = glm::mat3(1.0);
/*glm::mat3 orientationmat = glm::mat3(
   1.0, 0.0, 0.0, // first column 
   0.0, 1.0, 0.0, // second column
   0.0, 0.0, 1.0  // third column
);*/

bool orbit = false;

std::vector<float> interpolateSingleFloats(float from, float to, float numberOfValues) {

	float range = (to-from)/float(numberOfValues-1) ;

	std::vector<float> resvector ;

	for(float i=0; i<numberOfValues; i++) {
        resvector.push_back(from + i*range);
    }
    return resvector;

}

std::vector<glm::vec3> interpolateThreeElementValues( glm::vec3 from, glm::vec3 to, int numberOfValues) {
	
	glm::vec3 range = (to-from)/float(numberOfValues-1) ;


	std::vector<glm::vec3> resvector ;

	for(int i=0; i<numberOfValues; i++) {
        resvector.push_back(from + range*float(i));
    }
    return resvector;


}

uint32_t colourpixel (int red, int blue, int green ){

	uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
	return colour;
}

CanvasTriangle genRandomTriangle(DrawingWindow &window){

	float rpoint1 = rand() % window.width ;
	float rpoint2 = rand() % window.height ;
	float rpoint3 = rand() % window.width ;
	float rpoint4 = rand() % window.height ;
	float rpoint5 = rand() % window.width ;
	float rpoint6 = rand() % window.height  ;

	int r = rand() % 256 ;
	int g = rand() % 256 ;
	int b = rand() %256 ;

	Colour colour(r,g,b);

	CanvasPoint v1;
	v1.x=rpoint1;
	v1.y=rpoint2;

	CanvasPoint v2;
	v2.x=rpoint3;
	v2.y=rpoint4;

	CanvasPoint v3;
	v3.x=rpoint5;
	v3.y=rpoint6;

	CanvasTriangle trianglepoints(v1,v2,v3);

	return trianglepoints;

}

void drawLine( CanvasPoint from, CanvasPoint to, Colour colour, DrawingWindow &window ){

	float fromX = (from.x);
	float fromY = from.y;
	float fromdepth = from.depth;

	float toX = (to.x);
	float toY = to.y;
	float todepth = to.depth;

	float Xdiff = toX - fromX ;
	float Ydiff = toY - fromY ;
	float depthdiff = todepth - fromdepth ;

	float numberofsteps = std::max(abs(Xdiff), abs(Ydiff));
	float xstepsize = Xdiff/numberofsteps;
	float ystepsize = Ydiff/numberofsteps;
	float depthstepsize= depthdiff/numberofsteps;
	
	

		for (float i=0.0; i<=numberofsteps+1; i++){

		float x = (fromX + (xstepsize*i));
		float y = (fromY+ (ystepsize*i));
		float depth = fromdepth + (depthstepsize*i);

		
		if ((y>=0 && x>=0) &&(x<WIDTH && y < HEIGHT) ){

			/*if( depth == 0.0){
				window.setPixelColour(int(x), int(y),  colourpixel(colour.red, colour.blue, colour.green));
				depthbuffer[int(x)][int(y)]= depth;

			}*/

			 if ( depth >= depthbuffer[x][y] ){
			
			//std::cout<< depth<<std::endl;
			window.setPixelColour(int(x), int(y),  colourpixel(colour.red, colour.blue, colour.green));
			depthbuffer[int(x)][int(y)]= depth;
			 
			}

		}
		

		//window.setPixelColour(round(x), round(y),  colourpixel(colour.red, colour.blue, colour.green));

	}
	
	

}

void drawTexturedLine(CanvasPoint from, CanvasPoint to, TexturePoint textfrom, TexturePoint textto, TextureMap map, DrawingWindow &window ){

	float Xdiff = to.x - from.x ;
	
	float Ydiff = to.y - from.y ;

	float numberofsteps = std::max(std::abs(Xdiff), std::abs(Ydiff));
	float xstepsize = Xdiff/numberofsteps;
	float ystepsize = Ydiff/numberofsteps;

	//texture stuff
	float textXdiff = textto.x - textfrom.x ;
	float textYdiff = textto.y - textfrom.y ;

	float textxstepsize = textXdiff/numberofsteps;
	float textystepsize = textYdiff/numberofsteps;

	for (float i=0.0; i< numberofsteps; i++){

		float x = floor(from.x + (xstepsize*i));
		float y = floor(from.y+ (ystepsize*i));

		float textx = floor(textfrom.x + (textxstepsize*i));
		float texty = floor(textfrom.y+ (textystepsize*i));

		window.setPixelColour(x, y,  map.pixels[ (textx)+ (map.width * texty) ]);

	}
	//tester
}

void drawStrokedTriangles(CanvasTriangle vertices, Colour colour, DrawingWindow &window){

	drawLine(vertices.v0(),vertices.v1(),colour, window);
	drawLine(vertices.v1(),vertices.v2(),colour, window);
	drawLine(vertices.v0(),vertices.v2(),colour, window);


}

void interpolateAndFillTriangle(CanvasPoint v0, CanvasPoint v1, CanvasPoint middlepoint, float h, Colour colour, DrawingWindow &window){

	//interpolate between v0 and middlepoint
	std::vector<float> v0midx = interpolateSingleFloats(v0.x, middlepoint.x, h+1);
	std::vector<float> v0midy = interpolateSingleFloats(v0.y, middlepoint.y, h+1);
	//interpolate depth values
	std::vector<float> v0middepth = interpolateSingleFloats(v0.depth, middlepoint.depth, h+1);


	

	//interpolate between v0 and v1
	std::vector<float> v0v1x = interpolateSingleFloats(v0.x, v1.x, h+1);
	std::vector<float> v0v1y = interpolateSingleFloats(v0.y, v1.y, h+1);

	std::vector<float> v0v1depth = interpolateSingleFloats(v0.depth, v1.depth, h+1);

	//for loop to draw lines from v0mid points to v0v1 points
	for(float i=0.0; i<=h; i++){

		CanvasPoint from(v0midx[i], v0midy[i], v0middepth[i]);
		CanvasPoint to(v0v1x[i], v0v1y[i], v0v1depth[i]);

		drawLine(from,to,colour,window);
		//drawLine(middlepoint, v1, colour, window);
		//drawLine(v0, v1, colour, window);
		

	
	}


}

void interpolateAndFillTexture(CanvasPoint v0, CanvasPoint v1, CanvasPoint middlepoint, float h, float texth, TextureMap map, Colour colour, DrawingWindow &window){
	//did some changes to height check again 
	//interpolate between v0 and middlepoint
	std::vector<float> v0midx = interpolateSingleFloats(v0.x, middlepoint.x, h+1);
	std::vector<float> v0midy = interpolateSingleFloats(v0.y, middlepoint.y, h+1);

	//interpolate between v0 and v1
	std::vector<float> v0v1x = interpolateSingleFloats(v0.x, v1.x, h+1);
	std::vector<float> v0v1y = interpolateSingleFloats(v0.y, v1.y, h+1);

	//interpolate between v0 and mid textures
	std::vector<float> textv0midx = interpolateSingleFloats(v0.texturePoint.x, middlepoint.texturePoint.x, h+1);
	std::vector<float> textv0midy = interpolateSingleFloats(v0.texturePoint.y, middlepoint.texturePoint.y, h+1);

	//interpolate between v0 and v1 textures
	std::vector<float> textv0v1x = interpolateSingleFloats(v0.texturePoint.x, v1.texturePoint.x, h+1);
	std::vector<float> textv0v1y = interpolateSingleFloats(v0.texturePoint.y, v1.texturePoint.y, h+1);

	//for loop to draw lines from v0mid points to v0v1 points
	for(float i=0.0; i< h; i++){

		CanvasPoint from(v0midx[i], v0midy[i]);
		CanvasPoint to(v0v1x[i], v0v1y[i]);

		TexturePoint textfrom(textv0midx[i], textv0midy[i]);
		TexturePoint textto(textv0v1x[i], textv0v1y[i]);

		drawTexturedLine(from,to, textfrom, textto, map, window);
	}


}

void fillTextureTriangle(CanvasTriangle trianglepoints,  DrawingWindow &window, Colour colour){
	//did some changes to height
	TextureMap map = TextureMap(mappath);
	

	if(trianglepoints.v0().y > trianglepoints.v1().y) std::swap( trianglepoints.v0(), trianglepoints.v1());
	if(trianglepoints.v0().y > trianglepoints.v2().y) std::swap( trianglepoints.v0(), trianglepoints.v2());
	if(trianglepoints.v1().y > trianglepoints.v2().y) std::swap( trianglepoints.v1(), trianglepoints.v2());

	//finding x coordinate of middle point of the triangle using slope and making it a canvas point
	float slope = (trianglepoints.v2().y - trianglepoints.v0().y ) / (trianglepoints.v2().x - trianglepoints.v0().x);
	float middleX = ( (trianglepoints.v1().y - trianglepoints.v0().y) / slope ) + trianglepoints.v0().x ;
	CanvasPoint middlepoint( middleX, trianglepoints.v1().y);
	
	drawStrokedTriangles(trianglepoints, colour, window);


	//finding middle point of texture using ratio of two triangles

	//v0-mid x and y coordinates differences
	float v0midx = trianglepoints.v0().x - middlepoint.x ; 
	float v0midy = trianglepoints.v0().y - middlepoint.y ; 

	//v0-v2 x and y coordinates differences
	float v0v2x = trianglepoints.v0().x - trianglepoints.v2().x ; 
	float v0v2y = trianglepoints.v0().y - trianglepoints.v2().y ; 

	//v0mid ratio for x nd y 
	float v0midxratio = v0midx/v0v2x ;
	float v0midyratio = v0midy/v0v2y ;

	//texture v0-v2 x and y coordinates ratios 
	float textv0v2x = trianglepoints.v0().texturePoint.x - trianglepoints.v2().texturePoint.x ; 
	float textv0v2y = trianglepoints.v0().texturePoint.y - trianglepoints.v2().texturePoint.y ; 

	float textv0midx = v0midxratio * textv0v2x ;
	float textv0midy = v0midyratio * textv0v2y ;

	//solving for middlepoints texture point 
	middlepoint.texturePoint.x = trianglepoints.v0().texturePoint.x - textv0midx ; 
	middlepoint.texturePoint.y = trianglepoints.v0().texturePoint.y - textv0midy ; 

	//making top and bottom triangles
	//CanvasTriangle toptriangle(trianglepoints.v0(), middlepoint, trianglepoints.v1());
	//CanvasTriangle bottomtriangle( middlepoint, trianglepoints.v1(), trianglepoints.v2());


	//top triangle filling
	//drawStrokedTriangles(toptriangle, colour, window);
	float htop = middlepoint.y - trianglepoints.v0().y ; //height of top triangle
	float texthtop = middlepoint.texturePoint.y - trianglepoints.v0().texturePoint.y ; 
	interpolateAndFillTexture(trianglepoints.v0(), trianglepoints.v1(), middlepoint, htop, texthtop, map, colour, window);

	//bottom triangle filling 
	//drawStrokedTriangles(bottomtriangle, colour, window);
	float hbottom = trianglepoints.v2().y - middlepoint.y ;  //height of bottom triangle
	float texthbottom = middlepoint.texturePoint.y - trianglepoints.v2().texturePoint.y ;
	interpolateAndFillTexture(trianglepoints.v2(), trianglepoints.v1(), middlepoint, hbottom, texthbottom, map, colour, window);



}

void fill( CanvasTriangle trianglepoints, Colour colour, DrawingWindow &window){


	//sorting vertices based on height
	if(trianglepoints.v0().y > trianglepoints.v1().y) std::swap( trianglepoints.v0(), trianglepoints.v1());
	if(trianglepoints.v0().y > trianglepoints.v2().y) std::swap( trianglepoints.v0(), trianglepoints.v2());
	if(trianglepoints.v1().y > trianglepoints.v2().y) std::swap( trianglepoints.v1(), trianglepoints.v2());

	//finding x coordinate of middle point of the triangle using slope and making it a canvas point
	float slope = (trianglepoints.v2().y - trianglepoints.v0().y ) / (trianglepoints.v2().x - trianglepoints.v0().x);
	float middleX = ( (trianglepoints.v1().y - trianglepoints.v0().y) / slope ) + trianglepoints.v0().x ;
	CanvasPoint middlepoint( middleX, trianglepoints.v1().y);

	//finding depth of middle point
	//v0-mid  y coordinates differences
	float v0midy = trianglepoints.v0().y - middlepoint.y ; 
	//v0-v2 y coordinates differences
	float v0v2y = trianglepoints.v0().y - trianglepoints.v2().y ; 
	//v0mid ratio for  y 
	float v0midyratio = v0midy/v0v2y ;
	float v0v2depth =  (trianglepoints.v0().depth - trianglepoints.v2().depth);
	middlepoint.depth = trianglepoints.v0().depth - ( v0midyratio * v0v2depth );


	//making top and bottom triangles
	CanvasTriangle toptriangle(trianglepoints.v0(), trianglepoints.v1(), middlepoint);//swapped v1 and middlepoint
	CanvasTriangle bottomtriangle( middlepoint, trianglepoints.v1(), trianglepoints.v2());

	

	//top triangle filling
	drawStrokedTriangles(toptriangle, colour, window);
	float htop = middlepoint.y - trianglepoints.v0().y ; //height of top triangle
	interpolateAndFillTriangle(trianglepoints.v0(), trianglepoints.v1(), middlepoint, htop, colour, window);

	drawLine(middlepoint,trianglepoints.v1(), colour, window);

	//bottom triangle filling 
	drawStrokedTriangles(bottomtriangle, colour, window);
	float hbottom = trianglepoints.v2().y - middlepoint.y ;  //height of bottom triangle
	interpolateAndFillTriangle(trianglepoints.v2(), trianglepoints.v1(), middlepoint, hbottom, colour, window);//used to be v2v1middlepoint

	//final white stroked triangle border
	// int r =255;
	// int g =255;
	// int b=255;
	// Colour whitecolour(r,g,b);
	// drawStrokedTriangles(trianglepoints, whitecolour, window);

}

CanvasPoint getCanvasIntersectionPoint(glm::vec3  camerapostion, glm::vec3 vertexpostion, float focalLength){

	glm::vec3  currvertexpos = vertexpostion - camerapostion ;
	currvertexpos = rotmatrix*currvertexpos; 


	
	float imagepointx =   (-180 * (focalLength * (currvertexpos.x / currvertexpos.z))) + (WIDTH/ 2) ;
	float imagepointy =   (180 * (focalLength * (currvertexpos.y / currvertexpos.z))) + (HEIGHT/ 2) ;

	float depth = INFINITY;

	if (currvertexpos.z != 0.0){
		depth = abs(1/currvertexpos.z);
	}

	

	return CanvasPoint (imagepointx, imagepointy, depth);

}

void renderPointCloud (std::vector<ModelTriangle> triangles, glm::vec3 cameraposition, float focallength, DrawingWindow &window ){

	int r = 255 ;
	int g = 255 ;
	int b = 255 ;

	uint32_t colour = (255 << 24) + (int(r) << 16) + (int(g) << 8) + int(b);

	for( int i=0; i<triangles.size(); i++){

		CanvasPoint v0 = getCanvasIntersectionPoint(cameraposition, triangles[i].vertices[0], focallength);
		CanvasPoint v1 = getCanvasIntersectionPoint(cameraposition, triangles[i].vertices[1], focallength);
		CanvasPoint v2 = getCanvasIntersectionPoint(cameraposition, triangles[i].vertices[2], focallength);

		if( (v0.x >= 0 && v0.x< WIDTH) && (v0.y >= 0 && v0.y< HEIGHT)){
			
			window.setPixelColour(v0.x, v0.y, colour);
			

		}
		if( (v1.x >= 0 && v1.x< WIDTH) && (v1.y >= 0 && v1.y< HEIGHT)){

			window.setPixelColour(v1.x, v1.y, colour);

		}
		if( (v2.x >= 0 && v2.x< WIDTH) && (v2.y >= 0 && v2.y< HEIGHT)){

			window.setPixelColour(v2.x, v2.y, colour);

		}
		
		
	}

}

void wireFrameRender(std::vector<ModelTriangle> triangles, glm::vec3 cameraposition, float focallength, DrawingWindow &window ){

	int r = 255 ;
	int g = 255 ;
	int b = 255 ;

	//uint32_t colour = (255 << 24) + (int(r) << 16) + (int(g) << 8) + int(b);
	Colour colour(r,g,b);

	for( int i=0; i<triangles.size(); i++){

		CanvasPoint v0 = getCanvasIntersectionPoint(cameraposition, triangles[i].vertices[0], focallength);
		CanvasPoint v1 = getCanvasIntersectionPoint(cameraposition, triangles[i].vertices[1], focallength);
		CanvasPoint v2 = getCanvasIntersectionPoint(cameraposition, triangles[i].vertices[2], focallength);

		CanvasTriangle triangle(v0, v1, v2);

		drawStrokedTriangles(triangle,colour, window);
		
	}

}

void rasterizedRender(std::vector<ModelTriangle> triangles, glm::vec3 cameraposition, float focallength, DrawingWindow &window ){
	

	for( int i=0; i<triangles.size(); i++){

		CanvasPoint v0 = getCanvasIntersectionPoint(cameraposition, triangles[i].vertices[0], focallength);
		CanvasPoint v1 = getCanvasIntersectionPoint(cameraposition, triangles[i].vertices[1], focallength);
		CanvasPoint v2 = getCanvasIntersectionPoint(cameraposition, triangles[i].vertices[2], focallength);

		v0.texturePoint= triangles[i].texturePoints[0];
		v1.texturePoint= triangles[i].texturePoints[1];
		v2.texturePoint= triangles[i].texturePoints[2];

		//std::cout<< v0.texturePoint;
		

		CanvasTriangle imageplanetriangle{v0, v1, v2};

		int r = triangles[i].colour.red ;
		int g = triangles[i].colour.green;
		int b = triangles[i].colour.blue;

		Colour colour(r, g,b);

		/*if ( triangles[i].texturePoints.empty() == false ){
			fillTextureTriangle(imageplanetriangle, window, colour);
		}*/

		fill(imageplanetriangle, colour, window);

		
		

		
	}


}

glm::mat3 lookAt(glm::vec3 target) {
	glm::vec3 forward = glm::normalize(cameraposition-target);
	glm::vec3 right = -glm::normalize(glm::cross(forward, glm::vec3(0,1,0)));
	glm::vec3 up = glm::normalize(glm::cross(forward, right));

	//right up forward
	return transpose(glm::mat3( right, up, forward ));
}

void orbitrender(){
	glm::vec3 target(0.0,0.0,0.0);

	rotmatrix = lookAt(target);
}

void drawRasterizedScene(DrawingWindow &window, std::vector<ModelTriangle> triangle, bool orbit) {

	for(int x = 0; x < WIDTH; x++) {
		for(int y = 0; y < HEIGHT; y++) {
			depthbuffer[x][y] = 0.0;
		}
	}
	window.clearPixels(); 

	 

	if (orbit){
		glm::mat3 newrotationMat(
			glm::cos(rotchange),0.0,-glm::sin(rotchange),
			0.0,1.0,0.0,
			glm::sin(rotchange),0.0,glm::cos(rotchange)
		);
		cameraposition = newrotationMat * cameraposition;
		orbitrender();
	}
	//rotmatrix = newrotationMat * rotmatrix;
	

	

	rasterizedRender(triangle, cameraposition, focallength, window);
	

}

RayTriangleIntersection getClosestIntersection(glm::vec3 startpostion, glm::vec3 raydirection, std::vector<ModelTriangle> triangle){


	RayTriangleIntersection closeintersectionpoint ;
	
	float closevalcheck = INFINITY;


	for(int i = 0 ; i<triangle.size(); i++){
		
		glm::vec3 e0 = triangle[i].vertices[1] - triangle[i].vertices[0];
		glm::vec3 e1 = triangle[i].vertices[2] - triangle[i].vertices[0];
		glm::vec3 SPVector = startpostion - triangle[i].vertices[0];
		glm::mat3 DEMatrix(-raydirection, e0, e1);
		glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;

		// (u >= 0.0) && (u <= 1.0)
		// (v >= 0.0) && (v <= 1.0)
		// (u + v) <= 1.0
		if ( 	(possibleSolution.y>= 0.0 && possibleSolution.y <= 1.0 )&& 
				(possibleSolution.z>= 0.0 && possibleSolution.z <= 1.0) && 
				(possibleSolution.y+possibleSolution.z <= 1.0) && 
				(possibleSolution.x > 0 ) && 
				(possibleSolution.x < closevalcheck)
			
			){
			
			closevalcheck = possibleSolution.x;

			//storing closest intersection point values
			glm::vec3 intersecpoint = triangle[i].vertices[0] + possibleSolution.y * (triangle[i].vertices[1]-triangle[i].vertices[0]) + possibleSolution.z*(triangle[i].vertices[2] - triangle[i].vertices[0]);

			closeintersectionpoint.intersectionPoint = intersecpoint;
			closeintersectionpoint.distanceFromCamera= possibleSolution.x;
			closeintersectionpoint.intersectedTriangle = triangle[i];
			closeintersectionpoint.triangleIndex = i;

		}
	
	}

	return closeintersectionpoint;	
	
}
RayTriangleIntersection getClosestIntersection(glm::vec3 startpostion, glm::vec3 raydirection, std::vector<ModelTriangle> triangle, int triangleindex){

	RayTriangleIntersection closeintersectionpoint ;
	float closevalcheck = INFINITY;

	float distance =glm::length(raydirection);


	for(int i = 0 ; i<triangle.size(); i++){
		
		glm::vec3 e0 = triangle[i].vertices[1] - triangle[i].vertices[0];
		glm::vec3 e1 = triangle[i].vertices[2] - triangle[i].vertices[0];
		glm::vec3 SPVector = startpostion - triangle[i].vertices[0];
		glm::mat3 DEMatrix(-raydirection, e0, e1);
		glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;

		// (u >= 0.0) && (u <= 1.0)
		// (v >= 0.0) && (v <= 1.0)
		// (u + v) <= 1.0
		if ( 	(possibleSolution.y>= 0.0 && possibleSolution.y <= 1.0 )&& 
				(possibleSolution.z>= 0.0 && possibleSolution.z <= 1.0) && 
				(possibleSolution.y+possibleSolution.z <= 1.0) && 
				(possibleSolution.x > 0 ) && 
				(possibleSolution.x < closevalcheck) &&
				( i != triangleindex) 
				//(possibleSolution.x < distance)
				
			){
			
			closevalcheck = possibleSolution.x;

			//storing closest intersection point values
			glm::vec3 intersecpoint = triangle[i].vertices[0] + possibleSolution.y * (triangle[i].vertices[1]-triangle[i].vertices[0]) + possibleSolution.z*(triangle[i].vertices[2] - triangle[i].vertices[0]);

			closeintersectionpoint.intersectionPoint = intersecpoint;
			closeintersectionpoint.distanceFromCamera= possibleSolution.x;
			closeintersectionpoint.intersectedTriangle = triangle[i];
			closeintersectionpoint.triangleIndex = i;

		}
	
	}

	return closeintersectionpoint;	
	
}

void drawRayTrace(DrawingWindow &window, std::vector<ModelTriangle> triangle){

	for(int x = 0; x < WIDTH; x++) {
		for(int y = 0; y < HEIGHT; y++) {
			depthbuffer[x][y] = 0.0;
		}
	}
	

	for (float i=0.0; i< WIDTH ; i++){
		for (float j =0.0; j< HEIGHT ; j++){

			CanvasPoint xpoint ={i,j};

		// float depth = INFINITY;

			//float imagepointx =   (-180 * (focalLength * (currvertexpos.x / currvertexpos.z))) + (WIDTH/ 2) ;
		
			float x = ((i- WIDTH/ 2) / 180) / focallength;
		
			float y= - (( j- HEIGHT/ 2) / 180) / focallength;
			float z = -1;

			glm::vec3 raypoint(x,y,z);
			glm::vec3 raydirection = glm::normalize(raypoint);

			

			RayTriangleIntersection ip = getClosestIntersection(cameraposition, raydirection, triangle);

			glm::vec3 light = lightsource - ip.intersectionPoint ; 
			glm::vec3 lightdirec = glm::normalize(light);


			RayTriangleIntersection shadowip = getClosestIntersection(ip.intersectionPoint, lightdirec ,triangle, ip.triangleIndex);

		
			
			int red = ip.intersectedTriangle.colour.red;
			int blue = ip.intersectedTriangle.colour.blue;
			int green = ip.intersectedTriangle.colour.green;

			if (shadowip.distanceFromCamera < glm::length(lightdirec)){

				window.setPixelColour(i,j,colourpixel(0, 0, 0));
			}
			else{
				window.setPixelColour(i,j,colourpixel(red,blue,green));

			}

		}
	}
}

std::vector<ModelTriangle> parseObj (std::string mtlfilepath, std::string objfilepath, float scale){

	std::ifstream objfile(objfilepath);
	std::ifstream mtlfile(mtlfilepath);

	std::string objline ;
	std::string mtlline ;
	
	std::vector<ModelTriangle> triangles ; 
	std::vector<glm::vec3> vertices ; 
	std::vector<TexturePoint> texturevertices ;

	std::unordered_map<std::string, Colour> colhashmap; 
	std::string colourname ;


	while(getline(mtlfile, mtlline)){

		std::vector<std::string> token = split(mtlline, ' ');

		if(token[0]== "newmtl"){

			std::string mtlcolourname = token[1];

			//going to next line for values
			getline(mtlfile, mtlline);
			token = split(mtlline, ' ');

			int r = 255* stof(token[1]) ;
			int g = 255* stof(token[2]);
			int b = 255* stof(token[3]) ;

			colhashmap[mtlcolourname] = Colour(mtlcolourname, r, g, b );
		}

		if(token[0]== "map_Kd"){
			 mappath =  token[1];
		}

	}
	mtlfile.close();


	//reading obj file
	while(getline(objfile, objline)){

		
		std::vector<std::string> token = split(objline, ' ');

		if (token[0]=="usemtl"){

			colourname = token[1];

		}

		if(token[0]== "v"){

			vertices.push_back(glm::vec3( (stof(token[1]))*scale, (stof(token[2]))*scale, (stof(token[3]))*scale ) );

		}
		if(token[0]== "vt"){

			float xt = stof(token[1]);
			float yt = stof(token[2]);
			TexturePoint vt;
			vt.x=xt;
			vt.y=yt;

			texturevertices.push_back(vt);
		}

		if(token[0]== "f"){

			std::string f1= token[1];
			std::string f2= token[2];
			std::string f3= token[3];
			std::vector<std::string> fslash1 = split(f1, '/');
			std::vector<std::string> fslash2 = split(f2, '/');
			std::vector<std::string> fslash3 = split(f3, '/');

			//For each index in f
			std::array<glm::vec3, 3> trianglePoints;
			std::array<TexturePoint, 3> selectedTexturePoints;


			if ((fslash1[1].empty() == true) && (fslash2[1].empty() == true) && (fslash3[1].empty() == true)){

				triangles.push_back(ModelTriangle( vertices[ stoi(fslash1[0])- 1], vertices[ stoi(fslash2[0])- 1], vertices[ stoi(fslash3[0])- 1], colhashmap[colourname] ) );
			}
			else{

				triangles.push_back(ModelTriangle( vertices[ stoi(fslash1[0])- 1], vertices[ stoi(fslash2[0])- 1], vertices[ stoi(fslash3[0])- 1], colhashmap[colourname] ) );
				selectedTexturePoints[0] = texturevertices[stoi(fslash1[1]) - 1];
				selectedTexturePoints[1] = texturevertices[stoi(fslash2[1]) - 1];
				selectedTexturePoints[2] = texturevertices[stoi(fslash3[1]) - 1];
			}

			ModelTriangle triangle = ModelTriangle();
			triangle.texturePoints= selectedTexturePoints;

		}
	}

	objfile.close();

	return triangles;

}

//keypress takes in window, randmoizes canvas points makes a canvas traingle; randomizes a color and makes a triangle with all of the above 
void draw(DrawingWindow &window, std::vector<ModelTriangle> triangle){

	for(int x = 0; x < WIDTH; x++) {
		for(int y = 0; y < HEIGHT; y++) {
			depthbuffer[x][y] = 0.0;
		}
	}

	window.clearPixels();
	
	switch (rendermode)
	{
	case WIREFRAME:
		
		wireFrameRender(triangle, cameraposition, focallength, window);
		break;

	case RASTERIZING:
		rasterizedRender(triangle, cameraposition, focallength, window);
		break;

	case RAYTRACING:

		drawRayTrace(window, triangle);
		break;
	}
	

	//std::vector<ModelTriangle>  x = parseObj("cornell-box.mtl","cornell-box.obj", 0.35);

	
	

	//renderPointCloud(x, cameraposition, focallength, window);

	//wireFrameRender(x, cameraposition, focallength, window);

	//drawRayTrace(window, x);

	//rasterizedRender(x, cameraposition, focallength, window);

	//fill(trianglepoints, colour, window);




}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_w) cameraposition.y = cameraposition.y-camposchange;
		else if (event.key.keysym.sym == SDLK_a) cameraposition.x = cameraposition.x+camposchange;
		else if (event.key.keysym.sym == SDLK_s) cameraposition.y = cameraposition.y+camposchange;
		else if (event.key.keysym.sym == SDLK_d) cameraposition.x = cameraposition.x-camposchange;
		else if (event.key.keysym.sym == SDLK_q) cameraposition.z = cameraposition.z-camposchange;
		else if (event.key.keysym.sym == SDLK_e) cameraposition.z = cameraposition.z+camposchange;

		else if (event.key.keysym.sym == SDLK_v) rendermode=WIREFRAME;
		else if (event.key.keysym.sym == SDLK_b) rendermode=RASTERIZING;
		else if (event.key.keysym.sym == SDLK_n) rendermode=RAYTRACING;
		

		else if (event.key.keysym.sym == SDLK_j) {
			glm::mat3 newrotationMat(
				1.0,0.0,0.0,
				0.0,glm::cos(-rotchange),glm::sin(-rotchange),
				0.0, -glm::sin(-rotchange),glm::cos(-rotchange) 
			);
			rotmatrix = newrotationMat * rotmatrix;
			//orientationmat = newrotationMat * orientationmat;
		}
		else if (event.key.keysym.sym == SDLK_u) {

			glm::mat3 newrotationMat(
				1.0,0.0,0.0,
				0.0,glm::cos(rotchange),glm::sin(rotchange),
				0.0, -glm::sin(rotchange),glm::cos(rotchange) 
			);
			rotmatrix = newrotationMat * rotmatrix;
			//orientationmat = newrotationMat * orientationmat;

		}
		else if (event.key.keysym.sym == SDLK_k) {
			glm::mat3 newrotationMat(
				glm::cos(-rotchange),0.0,-glm::sin(-rotchange),
				0.0,1.0,0.0,
				glm::sin(-rotchange),0.0,glm::cos(-rotchange)
			);
			rotmatrix = newrotationMat * rotmatrix;
			//orientationmat = newrotationMat * orientationmat;
		}
		else if (event.key.keysym.sym == SDLK_h) {

			glm::mat3 newrotationMat(
				glm::cos(rotchange),0.0,-glm::sin(rotchange),
				0.0,1.0,0.0,
				glm::sin(rotchange),0.0,glm::cos(rotchange)
			);
			rotmatrix = newrotationMat * rotmatrix;
			//orientationmat = newrotationMat * orientationmat;

		}

		else if(event.key.keysym.sym == SDLK_o) {
			orbit = !orbit;
		}

	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;

	//making all values of depth buffer 0.0
	depthbuffer.resize(WIDTH);
	for (int x =0 ; x< WIDTH ; x++){
		depthbuffer[x].resize(HEIGHT);
		for(int y = 0; y<HEIGHT; y++ ){
			depthbuffer[x][y]=0.0;
		}
	}

	std::vector<ModelTriangle>  x = parseObj("cornell-box.mtl","cornell-box.obj", 0.35);
	//std::vector<ModelTriangle>  x = parseObj("materials.mtl","logo.obj", 0.35);

	// texture mapping points
	// CanvasPoint v0 ;
	// v0.x=160;
	// v0.y=10;
	// v0.texturePoint.x=195;
	// v0.texturePoint.y=5;
	// CanvasPoint v1 ;
	// v1.x=300;
	// v1.y=230;
	// v1.texturePoint.x=395;
	// v1.texturePoint.y=380;
	// CanvasPoint v2 ;
	// v2.x=10;
	// v2.y=150;
	// v2.texturePoint.x=65;
	// v2.texturePoint.y=330;
	// CanvasTriangle trianglepoints(v0,v1,v2) ;

	// Colour colour;
	// colour.blue=255;
	// colour.green=255;
	// colour.red=255;


	



	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		//fillTextureTriangle(trianglepoints, window, colour);
		
		//drawRasterizedScene(window,x,orbit);
		draw(window, x);

		//drawLine(from,to,colour,window);
		//drawStrokedTriangles(trianglepoints,colour,window);

		
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
