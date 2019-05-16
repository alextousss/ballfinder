#pragma once
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/highgui.hpp>


#include "trackball.hpp"
#include "camerastreamer.hpp"
#include <thread>
#include <mutex>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <chrono>

struct Command {
    int x;
    int y;
    int z;
    bool pince;
};

class BallGuider {
public:
    BallGuider();
    void update(); // TO CALL A LOT
    Command getCommand() { return mCommand; }
private:
    Command mCommand;
    CameraStreamer mStreamer;
    BallFinder mLeftFinder, mRightFinder;
    cv::Mat mBaseSrcLeft, mBaseSrcRight;

};