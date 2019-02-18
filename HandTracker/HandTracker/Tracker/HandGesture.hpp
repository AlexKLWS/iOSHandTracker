#ifndef _HAND_GESTURE_
#define _HAND_GESTURE_ 

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include "ImageSource.hpp"

using namespace cv;
using namespace std;

#define PI 3.14159

class HandGesture {
public:
    HandGesture();
	ImageSource m;
	vector<vector<cv::Point> > contours;
	vector<vector<int> >hullIndex;
	vector<vector<cv::Point> >hullPoint;
	vector<vector<Vec4i> > defects;
	vector <cv::Point> fingerTips;
	cv::Rect rect;
	void printGestureInfo(Mat src);
	int cIdx;
	int frameNumber;
	int mostFrequentFingerNumber;
	int numberOfDefects;
	cv::Rect bRect;
	double bRect_width;
	double bRect_height;
	bool isHand;
	bool detectIfHand();
	void initVectors();
	void getFingerNumber(ImageSource *m);
	void eleminateDefects();
	void getFingertips(ImageSource *m);
	void drawFingertips(ImageSource *m);
private:
	string bool2string(bool tf);
	int fontFace;
	int prevNrFingerTips;
	void checkForOneFinger(ImageSource *m);
	float getAngle(cv::Point s, cv::Point f, cv::Point e);
	vector<int> fingerNumbers;
	void analyzeContours();
	string intToString(int number);
	void computeFingerNumber();
	void addNumberToImg(ImageSource *m);
	vector<int> numbers2Display;
	void addFingerNumberToVector();
	Scalar numberColor;
	int nrNoFinger;
	float distanceP2P(cv::Point a, cv::Point b);
	void removeRedundantEndPoints(vector<Vec4i> newDefects);
	void removeRedundantFingerTips();
};

#endif
