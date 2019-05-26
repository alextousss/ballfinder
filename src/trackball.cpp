#include "trackball.hpp"

using namespace cv;
using namespace std;


double squareOfDist(Point p) {
    return p.x * p.x + p.y * p.y;
}


bool compareDists(const pair<int, int>&i, const pair<int, int>&j)
{
    return i.second < j.second;
}


BallFinder::BallFinder() {
    mTracker = TrackerCSRT::create();
    mTrackerInited = false;
}

void BallFinder::update(Mat & src) {
    Rect2d roi;

    if(mTrackerInited) {
        if(!mTracker->update(src, roi)) {
            cout << "fail " << endl;
            mTracker = TrackerCSRT::create();
            mTrackerInited = false;
        }
        Point newCenter = (roi.br() + roi.tl())*0.5;
        if ( mPositionHistory.size() && norm(mPositionHistory.back().pos-newCenter) > 50 ) {
            cout << "ABHERRANT " << endl;
            mTracker = TrackerCSRT::create();
            mTrackerInited = false;
        }
    }
    Mat splittedSrc[3];
    split(src, splittedSrc);
    for(unsigned int c = 0; c < 3; c++) {
        Mat cSrc = splittedSrc[c];
        /// Reduce the noise so we avoid false circle detection
        GaussianBlur(cSrc, cSrc, Size(5,5), 2, 2);
        //medianBlur(srcGray, srcGray, 5);
        vector<Vec3f> circles;

        /// Apply the Hough Transform to find the circles
        HoughCircles(cSrc, circles, HOUGH_GRADIENT, 1, cSrc.rows / 8, 100, 30, 10, 40);
        #define MARGIN 25
        bool drawed=false;
        for (size_t i = 0; i < circles.size(); i++)
        {
            Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
            int radius = cvRound(circles[i][2]);
            Mat mask = Mat::zeros(src.size(), CV_8UC1);
            circle(mask, center, radius, Scalar(255), -1, 8, 0);
            Mat hsvSrc;
            cvtColor(src, hsvSrc, CV_RGB2HSV);
            Mat channels[3];
            split(hsvSrc, channels);
            Vec3d mean, stddev;
            meanStdDev(channels[0], mean, stddev, mask);
            float val = (stddev[0] / mean[0])*100;

            //if(val > 29 && val < 35) {
                //circle(src, center, radius, Scalar(255, 255, 255), 2, 8, 0);
            //}
            //cout << val << endl;
            //
            // circle(cSrc, center, radius, Scalar(255, 0, 255), 2, 8, 0);
            if(val < 2) {
                drawed=true;
                circle(src, center, radius, Scalar(255, 255, 255), 2, 8, 0);
                roi=Rect2d(Point(center.x-radius-MARGIN/2, center.y-radius-MARGIN/2), Size(radius*2+MARGIN, radius*2+MARGIN));
                mTracker->init(src,roi);
                mTrackerInited = true;
            }

        }
        //imshow("d = " + to_string(c), cSrc);
        if(drawed) {
           // waitKey(10000);
        }
    }
    mPositionHistory.push_back(Ball{
        (roi.br() + roi.tl())*0.5,
        roi.width / 2
    });
}

#define ROLLING_MEDIAN_SIZE 1

Ball BallFinder::getBallPos() {
    if(mPositionHistory.size() < ROLLING_MEDIAN_SIZE)
        return Ball{};
    std::pair<unsigned int, double> lastElements[ROLLING_MEDIAN_SIZE];
    for(unsigned int i = mPositionHistory.size()-ROLLING_MEDIAN_SIZE; i < mPositionHistory.size(); i++) {
        lastElements[i-(mPositionHistory.size()-ROLLING_MEDIAN_SIZE)] = pair<unsigned int, double>(i, squareOfDist(mPositionHistory[i].pos));
    }
    std::sort(std::begin(lastElements), std::end(lastElements), compareDists);
    unsigned int medianIndex = lastElements[ROLLING_MEDIAN_SIZE/2].first;
    return mPositionHistory[medianIndex];
}
