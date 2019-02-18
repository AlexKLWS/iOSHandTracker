#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include "ColorSampleROI.hpp"

ColorSampleROI::ColorSampleROI() {
	upperCorner = cv::Point(0, 0);
	lowerCorner = cv::Point(0, 0);

}

ColorSampleROI::ColorSampleROI(cv::Point upperCorner, cv::Point lowerCorner, cv::Mat source) {
	this->upperCorner = upperCorner;
	this->lowerCorner = lowerCorner;
	borderThickness = 2;
	regionOfInterestPointer = source(cv::Rect(upperCorner.x, upperCorner.y, lowerCorner.x - upperCorner.x, lowerCorner.y - upperCorner.y));
}

void ColorSampleROI::drawRectangle(cv::Mat src) {
	rectangle(src, upperCorner, lowerCorner, color, borderThickness);
}
