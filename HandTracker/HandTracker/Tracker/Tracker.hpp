#ifndef _TRACKER_HEADER_
#define _TRACKER_HEADER_

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include "ColorSampleROI.hpp"
#include "HandGesture.hpp"

#define ORIGCOL2COL COLOR_BGR2HLS
#define COL2ORIGCOL COLOR_HLS2BGR
#define NSAMPLES 7

class Tracker {
private:
    const int fontFace = FONT_HERSHEY_PLAIN;
    const int squareLength = 20;
    int averageColors[NSAMPLES][3];
    int lowerColorBounds[NSAMPLES][3];
    int upperColorBounds[NSAMPLES][3];
    vector <ColorSampleROI> colorSampleRegions;
    Point2f handCoordinates;
    
    ImageSource imageSource;
    HandGesture handDetector;
    
    int getMedian(vector<int> values);
    void getAverageColor(ColorSampleROI roi, int averages[3]);
    void normalizeColors();
    Mat generateBinaryFrom(Mat& downsampledFrame);
    int findBiggestContour(vector<vector<cv::Point> > contours);
    void drawHandContours(ImageSource *m);
    void makeContours(ImageSource *m);
public:
    Tracker();
    void startTracking();
    void getColorSamples(Mat& image);
    void tracking(ImageSource imageSrc);
    void drawColorSampleRegions(Mat& image);
    void setupColorSampleImageRegions(Mat& image);
};

#endif
