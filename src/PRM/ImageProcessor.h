#pragma once
#ifndef _IMAGE_PROCESSOR_H_
#define _IMAGE_PROCESSOR_H_

#include "opencv2/imgproc.hpp"
#include <GL/glew.h>

cv::Mat ocvImgFromGlTex(GLuint glTexID);

double detectThirds(cv::Mat src);
double detectNormals(cv::Mat src);

#endif