//
//  MainViewController.h
//  HandTracker
//
//  Created by Alex Korzh on 2/12/19.
//  Copyright © 2019 Alex Korzh. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <opencv2/videoio/cap_ios.h>
using namespace cv;

@interface MainViewController : UIViewController<CvVideoCameraDelegate>

@property (nonatomic, retain) CvVideoCamera* videoCamera;

@end

typedef enum {
    NONE,
    AWAITNG_PALM,
    GETTING_COLOR_SAMPLE,
    TRACKING
} TrackerState;
