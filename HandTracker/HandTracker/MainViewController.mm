//
//  ViewController.m
//  HandTracker
//
//  Created by Alex Korzh on 2/12/19.
//  Copyright © 2019 Alex Korzh. All rights reserved.
//

#import "MainViewController.h"


@interface MainViewController ()

@property (weak, nonatomic) IBOutlet UIImageView *imageView;
@property (weak, nonatomic) IBOutlet UIButton *button;

@end

@implementation MainViewController

int const defaultFPS = 30;
TrackerState currentTrackerState = NONE;

- (void)viewDidLoad {
    [super viewDidLoad];
    
    _videoCamera = [[CvVideoCamera alloc] initWithParentView:_imageView];
    _videoCamera.defaultAVCaptureDevicePosition = AVCaptureDevicePositionFront;
    _videoCamera.defaultAVCaptureSessionPreset = AVCaptureSessionPreset640x480;
    _videoCamera.defaultAVCaptureVideoOrientation = AVCaptureVideoOrientationPortrait;
    [_videoCamera setDefaultFPS:defaultFPS];
    _videoCamera.delegate = self;
    
    [_button addTarget:self action:@selector(onButtonTap) forControlEvents:UIControlEventTouchUpInside];
}

#ifdef __cplusplus
- (void)processImage:(Mat &)image {
    //
    switch (currentTrackerState){
        case NONE:
//            SetupFrameForPalmColorSampling();
            currentTrackerState = AWAITNG_PALM;
            break;
        case AWAITNG_PALM:
//            DrawColorSampleRegions();
            break;
        case GETTING_COLOR_SAMPLE:
            break;
        case TRACKING:
            break;
    }
}
#endif

- (void)onButtonTap {
    [self.videoCamera start];
}

@end
