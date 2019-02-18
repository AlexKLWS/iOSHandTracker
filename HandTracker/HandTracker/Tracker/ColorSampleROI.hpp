#ifndef ROI 
#define ROI

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

class ColorSampleROI {
public:
	ColorSampleROI();
    ColorSampleROI(cv::Point upperCorner, cv::Point lowerCorner, cv::Mat source);
	cv::Point upperCorner, lowerCorner;
    cv::Mat regionOfInterestPointer;
	int borderThickness;
    void drawRectangle(cv::Mat src);
    const cv::Scalar color = cv::Scalar(0, 255, 0);
};

#endif
