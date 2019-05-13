#include "trackball.hpp"

using namespace cv;

BallFinder::BallFinder() {

}

void BallFinder::update(Mat & src) {
    Mat srcGray;
    Rect2d roi;
    
    if(mTrackerInited) {
        if(!mTracker->update(src, roi)) {
            cout << "fail " << endl;
            mTrackerInited = false;
        }
    }

    /// Convert it to gray
    cvtColor(src, srcGray, COLOR_BGR2GRAY);

    /// Reduce the noise so we avoid false circle detection
    GaussianBlur(srcGray, srcGray, Size(9,9), 2, 2);

    vector<Vec3f> circles;

    /// Apply the Hough Transform to find the circles
    HoughCircles(srcGray, circles, HOUGH_GRADIENT, 0.8, srcGray.rows / 8, 100, 50, 0, 0);
    #define MARGIN 10

    /// Draw the circles detected
    for (size_t i = 0; i < circles.size(); i++)
    {
        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);
        Center = center;
        // circle center
        circle(src, center, 3, Scalar(0, 255, 0), -1, 8, 0);
        // circle outline
        circle(src, center, radius, Scalar(0, 0, 255), 3, 8, 0);
        roi=Rect2d(Point(center.x-radius-MARGIN/2, center.y-radius-MARGIN/2), Size(radius*2+MARGIN, radius*2+MARGIN));
        mTracker->init(src,roi);
        mTrackerInited = true;
    }
    
    return Ball{
        (roi.br() + roi.tl())*0.5,
        roi.width() / 2
    }

}