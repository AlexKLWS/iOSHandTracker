#ifndef _MYIMAGE_
#define _MYIMAGE_ 

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;

class ImageSource {
public:
	ImageSource();
	Mat downsampled;
	Mat original;
	Mat binary;
};



#endif
