#pragma once
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/tracking.hpp>

#include <list>

struct Ball{
    cv::Point pos;
    float radius;
}

class BallFinder {
public:
    BallFinder();
    void update(Mat & frame);
    Ball getBallPos();
private:
    bool mTrackerInited;
    list<Ball> mPositionHistory;
    Ptr<TrackerCSRT> mTracker;
};
