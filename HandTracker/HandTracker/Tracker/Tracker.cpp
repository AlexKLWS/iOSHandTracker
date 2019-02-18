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
			c_lower[i][j] = c_lower[0][j];
			c_upper[i][j] = c_upper[0][j];
		}
	}
	// normalize all boundries so that 
	// threshold is whithin 0-255
	for (int i = 0; i < NSAMPLES; i++) {
		if ((averageColors[i][0] - c_lower[i][0]) < 0) {
			c_lower[i][0] = averageColors[i][0];
		} if ((averageColors[i][1] - c_lower[i][1]) < 0) {
			c_lower[i][1] = averageColors[i][1];
		} if ((averageColors[i][2] - c_lower[i][2]) < 0) {
			c_lower[i][2] = averageColors[i][2];
		} if ((averageColors[i][0] + c_upper[i][0]) > 255) {
			c_upper[i][0] = 255 - averageColors[i][0];
		} if ((averageColors[i][1] + c_upper[i][1]) > 255) {
			c_upper[i][1] = 255 - averageColors[i][1];
		} if ((averageColors[i][2] + c_upper[i][2]) > 255) {
			c_upper[i][2] = 255 - averageColors[i][2];
		}
	}
}

void Tracker::produceBinaries(ImageSource *imageSrc) {
	Scalar lowerBound;
	Scalar upperBound;
	for (int i = 0; i < NSAMPLES; i++) {
		normalizeColors();
		lowerBound = Scalar(averageColors[i][0] - c_lower[i][0], averageColors[i][1] - c_lower[i][1], averageColors[i][2] - c_lower[i][2]);
		upperBound = Scalar(averageColors[i][0] + c_upper[i][0], averageColors[i][1] + c_upper[i][1], averageColors[i][2] + c_upper[i][2]);
		imageSrc->bwList.push_back(Mat(imageSrc->downsampled.rows, imageSrc->downsampled.cols, CV_8U));
		inRange(imageSrc->downsampled, lowerBound, upperBound, imageSrc->bwList[i]);
	}
	imageSrc->bwList[0].copyTo(imageSrc->binary);
	for (int i = 1; i < NSAMPLES; i++) {
		imageSrc->binary += imageSrc->bwList[i];
	}
	medianBlur(imageSrc->binary, imageSrc->binary, 7);
}

int Tracker::findBiggestContour(vector<vector<cv::Point> > contours) {
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

void Tracker::drawHandContours(ImageSource *m, HandGesture *hg) {
	drawContours(m->original, hg->hullPoint, hg->cIdx, cv::Scalar(200, 0, 0), 2, 8, vector<Vec4i>(), 0, cv::Point());

	rectangle(m->original, hg->bRect.tl(), hg->bRect.br(), Scalar(0, 0, 200));
	vector<Vec4i>::iterator d = hg->defects[hg->cIdx].begin();

	vector<Mat> channels;
	Mat result;
	for (int i = 0; i < 3; i++)
		channels.push_back(m->binary);
	merge(channels, result);
	drawContours(result, hg->hullPoint, hg->cIdx, cv::Scalar(0, 0, 250), 10, 8, vector<Vec4i>(), 0, cv::Point());


	while (d != hg->defects[hg->cIdx].end()) {
		Vec4i& v = (*d);
		int startidx = v[0];
        cv::Point ptStart(hg->contours[hg->cIdx][startidx]);
		int endidx = v[1];
        cv::Point ptEnd(hg->contours[hg->cIdx][endidx]);
		int faridx = v[2];
        cv::Point ptFar(hg->contours[hg->cIdx][faridx]);
		circle(result, ptFar, 9, Scalar(0, 205, 0), 5);

		d++;
	}
}

void Tracker::makeContours(ImageSource *m, HandGesture* hg) {
	Mat aBw;
	pyrUp(m->binary, m->binary);
	m->binary.copyTo(aBw);
    findContours(aBw, hg->contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	hg->initVectors();
	hg->cIdx = findBiggestContour(hg->contours);
	if (hg->cIdx != -1) {
		hg->bRect = boundingRect(Mat(hg->contours[hg->cIdx]));
        handCoordinates = (hg->bRect.br() + hg->bRect.tl())*0.5;
		convexHull(Mat(hg->contours[hg->cIdx]), hg->hullPoint[hg->cIdx], false, true);
		convexHull(Mat(hg->contours[hg->cIdx]), hg->hullIndex[hg->cIdx], false, false);
		approxPolyDP(Mat(hg->hullPoint[hg->cIdx]), hg->hullPoint[hg->cIdx], 18, true);
		if (hg->contours[hg->cIdx].size() > 3) {
			convexityDefects(hg->contours[hg->cIdx], hg->hullIndex[hg->cIdx], hg->defects[hg->cIdx]);
			hg->eleminateDefects();
		}
		bool isHand = hg->detectIfHand();
		hg->printGestureInfo(m->original);
		if (isHand) {
			hg->getFingertips(m);
			hg->drawFingertips(m);
			drawHandContours(m, hg);
		}
	}
}

void Tracker::startTracking() {
    for (int i = 0; i < NSAMPLES; i++) {
        c_lower[i][0] = 7;
        c_upper[i][0] = 21;
        c_lower[i][1] = 0;
        c_upper[i][1] = 16;
        c_lower[i][2] = 5;
        c_upper[i][2] = 10;
    }
//    _currentState = TRACKING;
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
    pyrDown(imageSrc.original, imageSrc.downsampled);
    blur(imageSrc.downsampled, imageSrc.downsampled, Size2i(3, 3));
    cvtColor(imageSrc.downsampled, imageSrc.downsampled, ORIGCOL2COL);
    produceBinaries(&imageSrc);
    cvtColor(imageSrc.downsampled, imageSrc.downsampled, COL2ORIGCOL);
    makeContours(&imageSrc, &handGesture);
    handGesture.getFingerNumber(&imageSrc);
    pyrDown(imageSrc.binary, imageSrc.binary);
    pyrDown(imageSrc.binary, imageSrc.binary);
    cv::Rect roi(cv::Point(3 * imageSrc.original.cols / 4, 0), imageSrc.binary.size());
    vector<Mat> channels;
    Mat result;
    for (int i = 0; i < 3; i++)
        channels.push_back(imageSrc.binary);
    merge(channels, result);
    result.copyTo(imageSrc.original(roi));
    imageSource.displayed = imageSource.original.clone();
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
