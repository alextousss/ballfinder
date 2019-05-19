#pragma once
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/tracking.hpp>

#include <string>
#include <list>
#include <vector>

struct Ball{
    cv::Point pos;
    double radius;
};

class BallFinder {
public:
    BallFinder();
    void update(cv::Mat & frame);
    Ball getBallPos();
private:
    bool mTrackerInited; 
    std::vector<Ball> mPositionHistory; //TODO: Utiliser une structure de donnée adaptée
    cv::Ptr<cv::TrackerCSRT> mTracker;
};
