#include "Tracker.hpp"

Tracker::Tracker(){}

template <std::size_t sizeA, std::size_t sizeB>
void clearArray(int (&a)[sizeA][sizeB]) {
    int* begin = &a[0][0];
    std::fill_n(begin, sizeA * sizeB, 0.0);
}

int Tracker::getMedian(vector<int> values) {
	int median;
	size_t size = values.size();
	sort(values.begin(), values.end());
	if (size % 2 == 0) {
		median = values[size / 2 - 1];
	}
	else {
		median = values[size / 2];
	}
	return median;
}

void Tracker::getAverageColor(ColorSampleROI roi, int averages[3]) {
	Mat region;
	roi.regionOfInterestPointer.copyTo(region);
	vector<int>hm;
	vector<int>sm;
	vector<int>lm;
	// generate vectors
	for (int i = 2; i < region.rows - 2; i++) {
		for (int j = 2; j < region.cols - 2; j++) {
			hm.push_back(region.data[region.channels()*(region.cols*i + j) + 0]);
			sm.push_back(region.data[region.channels()*(region.cols*i + j) + 1]);
			lm.push_back(region.data[region.channels()*(region.cols*i + j) + 2]);
		}
	}
	averages[0] = Tracker::getMedian(hm);
	averages[1] = Tracker::getMedian(sm);
	averages[2] = Tracker::getMedian(lm);
}

void Tracker::normalizeColors() {
	// copy all boundaries read from trackbar
	// to all of the different boundries
	for (int i = 1; i < NSAMPLES; i++) {
		for (int j = 0; j < 3; j++) {
			lowerColorBounds[i][j] = lowerColorBounds[0][j];
			upperColorBounds[i][j] = upperColorBounds[0][j];
		}
	}
	// normalize all boundries so that 
	// threshold is whithin 0-255
	for (int i = 0; i < NSAMPLES; i++) {
		if ((averageColors[i][0] - lowerColorBounds[i][0]) < 0) {
			lowerColorBounds[i][0] = averageColors[i][0];
		}
        if ((averageColors[i][1] - lowerColorBounds[i][1]) < 0) {
			lowerColorBounds[i][1] = averageColors[i][1];
		}
        if ((averageColors[i][2] - lowerColorBounds[i][2]) < 0) {
			lowerColorBounds[i][2] = averageColors[i][2];
		}
        if ((averageColors[i][0] + upperColorBounds[i][0]) > 255) {
			upperColorBounds[i][0] = 255 - averageColors[i][0];
		}
        if ((averageColors[i][1] + upperColorBounds[i][1]) > 255) {
			upperColorBounds[i][1] = 255 - averageColors[i][1];
		}
        if ((averageColors[i][2] + upperColorBounds[i][2]) > 255) {
			upperColorBounds[i][2] = 255 - averageColors[i][2];
		}
	}
}

Mat Tracker::generateBinaryFrom(Mat& downsampledFrame) {
	Scalar lowerBound;
	Scalar upperBound;
    vector<Mat> bwList;
    Mat binary;
	for (int i = 0; i < NSAMPLES; i++) {
		normalizeColors();
		lowerBound = Scalar(averageColors[i][0] - lowerColorBounds[i][0], averageColors[i][1] - lowerColorBounds[i][1], averageColors[i][2] - lowerColorBounds[i][2]);
		upperBound = Scalar(averageColors[i][0] + upperColorBounds[i][0], averageColors[i][1] + upperColorBounds[i][1], averageColors[i][2] + upperColorBounds[i][2]);
		bwList.push_back(Mat(downsampledFrame.rows, downsampledFrame.cols, CV_8U));
		inRange(downsampledFrame, lowerBound, upperBound, bwList[i]);
	}
	bwList[0].copyTo(binary);
	for (int i = 1; i < NSAMPLES; i++) {
		binary += bwList[i];
	}
	medianBlur(binary, binary, 7);
    return binary;
}

int Tracker::findBiggestContour(vector<vector<cv::Point>> contours) {
	int indexOfBiggestContour = -1;
	unsigned long sizeOfBiggestContour = 0;
	for (int i = 0; i < contours.size(); i++) {
		if (contours[i].size() > sizeOfBiggestContour) {
			sizeOfBiggestContour = contours[i].size();
			indexOfBiggestContour = i;
		}
	}
	return indexOfBiggestContour;
}

void Tracker::drawHandContours(ImageSource *m) {
	drawContours(m->original, handDetector.hullPoint, handDetector.cIdx, Scalar(200, 0, 0), 2, 8, vector<Vec4i>(), 0, cv::Point());

	rectangle(m->original, handDetector.bRect.tl(), handDetector.bRect.br(), Scalar(0, 0, 200));
	vector<Vec4i>::iterator d = handDetector.defects[handDetector.cIdx].begin();

	vector<Mat> channels;
	Mat result;
	for (int i = 0; i < 3; i++)
		channels.push_back(m->binary);
	merge(channels, result);
	drawContours(result, handDetector.hullPoint, handDetector.cIdx, Scalar(0, 0, 250), 10, 8, vector<Vec4i>(), 0, cv::Point());


	while (d != handDetector.defects[handDetector.cIdx].end()) {
		Vec4i& v = (*d);
		int startidx = v[0];
        cv::Point ptStart(handDetector.contours[handDetector.cIdx][startidx]);
		int endidx = v[1];
        cv::Point ptEnd(handDetector.contours[handDetector.cIdx][endidx]);
		int faridx = v[2];
        cv::Point ptFar(handDetector.contours[handDetector.cIdx][faridx]);
		circle(result, ptFar, 9, Scalar(0, 205, 0), 5);

		d++;
	}
}

void Tracker::makeContours(ImageSource *m) {
	Mat aBw;
	pyrUp(m->binary, m->binary);
	m->binary.copyTo(aBw);
    findContours(aBw, handDetector.contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	handDetector.initVectors();
	handDetector.cIdx = findBiggestContour(handDetector.contours);
	if (handDetector.cIdx != -1) {
		handDetector.bRect = boundingRect(Mat(handDetector.contours[handDetector.cIdx]));
        handCoordinates = (handDetector.bRect.br() + handDetector.bRect.tl())*0.5;
		convexHull(Mat(handDetector.contours[handDetector.cIdx]), handDetector.hullPoint[handDetector.cIdx], false, true);
		convexHull(Mat(handDetector.contours[handDetector.cIdx]), handDetector.hullIndex[handDetector.cIdx], false, false);
		approxPolyDP(Mat(handDetector.hullPoint[handDetector.cIdx]), handDetector.hullPoint[handDetector.cIdx], 18, true);
		if (handDetector.contours[handDetector.cIdx].size() > 3) {
			convexityDefects(handDetector.contours[handDetector.cIdx], handDetector.hullIndex[handDetector.cIdx], handDetector.defects[handDetector.cIdx]);
			handDetector.eleminateDefects();
		}
		bool isHand = handDetector.detectIfHand();
		handDetector.printGestureInfo(m->original);
		if (isHand) {
			handDetector.getFingertips(m);
			handDetector.drawFingertips(m);
			drawHandContours(m);
		}
	}
}

void Tracker::startTracking() {
    for (int i = 0; i < NSAMPLES; i++) {
        lowerColorBounds[i][0] = 7;
        upperColorBounds[i][0] = 21;
        lowerColorBounds[i][1] = 0;
        upperColorBounds[i][1] = 16;
        lowerColorBounds[i][2] = 5;
        upperColorBounds[i][2] = 10;
    }
}

void Tracker::getColorSamples(Mat& image) {
    cvtColor(image, image, ORIGCOL2COL);
    for (int i = 0; i < NSAMPLES; i++) {
        getAverageColor(colorSampleRegions[i], averageColors[i]);
        colorSampleRegions[i].drawRectangle(image);
    }
    cvtColor(image, image, COL2ORIGCOL);
    string text = string("Finding average color of hand");
    putText(image, text, cv::Point(image.cols / 2, image.rows / 10), fontFace, 1.2f, Scalar(200, 0, 0), 2);
}

void Tracker::tracking(ImageSource imageSrc) {
    Mat downsampled;
    pyrDown(imageSrc.original, downsampled);
    blur(downsampled, downsampled, Size2i(3, 3));
    cvtColor(downsampled, downsampled, ORIGCOL2COL);
    Mat binary = generateBinaryFrom(downsampled);
    cvtColor(downsampled, downsampled, COL2ORIGCOL);
    makeContours(&imageSrc);
    handDetector.getFingerNumber(&imageSrc);
    pyrDown(imageSrc.binary, imageSrc.binary);
    pyrDown(imageSrc.binary, imageSrc.binary);
    cv::Rect roi(cv::Point(3 * imageSrc.original.cols / 4, 0), imageSrc.binary.size());
    vector<Mat> channels;
    Mat result;
    for (int i = 0; i < 3; i++)
        channels.push_back(imageSrc.binary);
    merge(channels, result);
    result.copyTo(imageSrc.original(roi));
//    imageSource.displayed = imageSource.original.clone();
}

void Tracker::drawColorSampleRegions(Mat& image) {
	for (int i = 0; i < NSAMPLES; i++) {
		colorSampleRegions[i].drawRectangle(image);
	}
	string text = string("Please cover rectangles with your palm");
	putText(image, text, cv::Point(image.cols / 2, image.rows / 10), fontFace, 1.2f, Scalar(200, 0, 0), 2);
}

void Tracker::setupColorSampleImageRegions(Mat& image) {
    colorSampleRegions.push_back(ColorSampleROI(cv::Point(image.cols / 3, image.rows / 6),
                                                 cv::Point(image.cols / 3 + squareLength, image.rows / 6 + squareLength),
                                                 image));
    colorSampleRegions.push_back(ColorSampleROI(cv::Point(image.cols / 4, image.rows / 2),
                                                 cv::Point(image.cols / 4 + squareLength, image.rows / 2 + squareLength),
                                                 image));
    colorSampleRegions.push_back(ColorSampleROI(cv::Point(image.cols / 3, image.rows / 1.5),
                                                 cv::Point(image.cols / 3 + squareLength, image.rows / 1.5 + squareLength),
                                                 image));
    colorSampleRegions.push_back(ColorSampleROI(cv::Point(image.cols / 2, image.rows / 2),
                                                 cv::Point(image.cols / 2 + squareLength, image.rows / 2 + squareLength),
                                                 image));
    colorSampleRegions.push_back(ColorSampleROI(cv::Point(image.cols / 2.5, image.rows / 2.5),
                                                 cv::Point(image.cols / 2.5 + squareLength, image.rows / 2.5 + squareLength),
                                                 image));
    colorSampleRegions.push_back(ColorSampleROI(cv::Point(image.cols / 2, image.rows / 1.5),
                                                 cv::Point(image.cols / 2 + squareLength, image.rows / 1.5 + squareLength),
                                                 image));
    colorSampleRegions.push_back(ColorSampleROI(cv::Point(image.cols / 2.5, image.rows / 1.8),
                                                 cv::Point(image.cols / 2.5 + squareLength, image.rows / 1.8 + squareLength),
                                                 image));
}

//extern "C" void  GetFrame(PixelData* pixels)
//{
//    if (imageSource.original.empty() || imageSource.original.depth() != CV_8U)
//        return;
//
//    ImageSource* imageSrc = &imageSource;
//
//    flip(imageSrc->original, imageSrc->original, 1);
//
//    switch (_currentState) {
//    case AWAITNG_PALM:
//        DrawColorSampleRegions(imageSrc);
//        break;
//    case GETTING_COLOR_SAMPLE:
//        GetColorSample(imageSrc);
//        break;
//    case TRACKING:
//        Tracking(imageSource);
//        break;
//    default:
//        imageSource.displayed = imageSource.original.clone();
//        break;
//    }
//
//    int counter = 0;
//    MatIterator_<Vec3b> it, end;
//    for( it = imageSource.displayed.begin<Vec3b>(), end = imageSource.displayed.end<Vec3b>(); it != end; ++it)
//    {
//        *(pixels + counter) = PixelData ((*it)[2], (*it)[1], (*it)[0]);
//        counter++;
//    }
//}

//void GetHandCoordinates(float& x, float& y) {
//    x = handCoordinates.x;
//    y = handCoordinates.y;
//}
//
//void  Close()
//{
//    clearArray(c_lower);
//    clearArray(c_upper);
//    clearArray(_averageColors);
//    colorSampleRegions.clear();
//}
