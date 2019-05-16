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
}

void BallFinder::update(Mat & src) {
    Mat srcGray;
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

    /// Convert it to gray
    cvtColor(src, srcGray, COLOR_BGR2GRAY);

    /// Reduce the noise so we avoid false circle detection
    //GaussianBlur(srcGray, srcGray, Size(9,9), 2, 2);
    imshow("base", srcGray);
    medianBlur(srcGray, srcGray, 5);
    vector<Vec3f> circles;

    /// Apply the Hough Transform to find the circles
    HoughCircles(srcGray, circles, HOUGH_GRADIENT, 1, srcGray.rows / 8, 100, 30, 10, 40);
    #define MARGIN 25

    for (size_t i = 0; i < circles.size(); i++)
    {

        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);
        Mat mask = Mat::zeros(src.size(), CV_8UC3);
        circle(mask, center, radius * 0.8, Scalar(255, 255, 255), -1, 8, 0);
        Mat hsvSrc, res;
        cvtColor(src, hsvSrc, CV_RGB2HSV);
        cvtColor(src, res, CV_RGB2HSV);
        bitwise_and(mask,hsvSrc,res);
        Mat channels[3];
        split(res, channels);
        imshow("masked", channels[0]);
        Vec3d mean, stddev;
        meanStdDev(res, mean, stddev);
        float val = ((stddev[0] / mean[0]) / (radius))*100;

        //if(val > 29 && val < 35) {
            //circle(src, center, radius, Scalar(255, 255, 255), 2, 8, 0);
        //}
        cout << val << endl;
        //
        //waitKey(3000);

        if(val < 23) {
            circle(src, center, radius, Scalar(255, 255, 255), 2, 8, 0);
            roi=Rect2d(Point(center.x-radius-MARGIN/2, center.y-radius-MARGIN/2), Size(radius*2+MARGIN, radius*2+MARGIN));
            mTracker->init(src,roi);
            mTrackerInited = true;
        }
    }
    
    mPositionHistory.push_back(Ball{
        (roi.br() + roi.tl())*0.5,
        roi.width / 2
    });
}

#define ROLLING_MEDIAN_SIZE 5

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