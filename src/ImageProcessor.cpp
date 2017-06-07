#include "ImageProcessor.h"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>

#define FLIP_ON_X_AXIS 0

using namespace cv;
using namespace std;

// http://stackoverflow.com/questions/38584301/convering-opengl-texture-to-opencv-matrix
Mat ocvImgFromGlTex(GLuint glTexID) {
	glBindTexture(GL_TEXTURE_2D, glTexID);
	GLenum gl_texture_width, gl_texture_height;

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, (GLint*)&gl_texture_width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, (GLint*)&gl_texture_height);

	unsigned char* gl_texture_bytes = (unsigned char*) malloc(sizeof(unsigned char)*gl_texture_width*gl_texture_height*3);
	glGetTexImage(GL_TEXTURE_2D, 0 /* mipmap level */, GL_BGR, GL_UNSIGNED_BYTE, gl_texture_bytes);

	Mat temp;
	flip(Mat(gl_texture_height, gl_texture_width, CV_8UC3, gl_texture_bytes), temp, FLIP_ON_X_AXIS);

	return temp;
}

double detectNormals(Mat src) {
   Mat dst;
   inRange(src, Scalar(102,0,0), Scalar(255,255,255), dst);

   // namedWindow("Normals", CV_WINDOW_AUTOSIZE);
   // imshow("Normals", dst);

   return (float)countNonZero(dst) / (dst.size().height * dst.size().width);
}

double detectThirds(Mat src) {
	Mat src_gray;
	int thresh = 50;
	int picWidth = 0, picHeight = 0;

	// Load source image and convert it to gray
   picWidth = src.size().width;
   picHeight = src.size().height;

   // Convert image to gray and blur it
   cvtColor(src, src_gray, CV_BGR2GRAY);
   blur(src_gray, src_gray, Size(5,5));

	Mat threshold_output;
   vector<vector<Point> > contours;
   vector<Vec4i> hierarchy;

   // Detect edges using Threshold
   threshold(src_gray, threshold_output, thresh, 255, THRESH_BINARY);
   // Find contours
   findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

   // Approximate contours to polygons + get bounding rects and circles
   vector<vector<Point> > contours_poly(contours.size());
   vector<Rect> boundRect(contours.size());

   for (int i = 0; i < contours.size(); i++) {
      approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
      boundRect[i] = boundingRect(Mat(contours_poly[i]));
   }

   // Draw polygonal contour + bonding rects + circles
   Mat drawing = Mat::zeros(threshold_output.size(), CV_8UC3);
   // src.copyTo(drawing);
   src_gray.copyTo(drawing);

   // draw lines on the thirds
   line(drawing, Point(picWidth / 3, 0), Point(picWidth / 3, picHeight), Scalar(192, 192, 192), 1, 8, 0);
   line(drawing, Point(picWidth / 3 * 2, 0), Point(picWidth / 3 * 2, picHeight), Scalar(192, 192, 192), 1, 8, 0);
   line(drawing, Point(0, picHeight / 3), Point(picWidth, picHeight / 3), Scalar(192, 192, 192), 1, 8, 0);
   line(drawing, Point(0, picHeight / 3 * 2), Point(picWidth, picHeight / 3 * 2), Scalar(192, 192, 192), 1, 8, 0);

   int biggestBox = 0, biggestI = -1;
   for (int i = 0; i< contours.size(); i++) {
      if (boundRect[i].width > biggestBox) {
         biggestBox = boundRect[i].width;
         biggestI = i;
      }
   }
   if (biggestI != -1) {
      Rect bigRect = boundRect[biggestI];
      rectangle(drawing, bigRect.tl(), bigRect.br(), Scalar(255, 255, 255), 2, 8, 0);

      // rule of thirds using y position of top left corner
      double yThirds = 1.0 - (double) bigRect.tl().y / picHeight;

      /// Show in a window
   	// namedWindow("Thirds", CV_WINDOW_AUTOSIZE);
   	// imshow("Thirds", drawing);

   	return yThirds;
   }

   return -1; // TODO: Error check this
}