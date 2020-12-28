#ifndef IMAGESWITCH_H
#define IMAGESWITCH_H

#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

std::string base64Decode(const char* Data, int DataByte);
std::string base64Encode(const unsigned char* Data, int DataByte);
std::string Mat2Base64(const cv::Mat &img, std::string imgType);
cv::Mat Base2Mat(std::string &base64_data);

#endif