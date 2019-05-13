#include <opencv2/imgproc.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/highgui.hpp>

#include "camerastreamer.hpp"
#include <thread>
#include <mutex>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <chrono>

using namespace std;
using namespace cv;


/** @function main */
int main(int argc, char **argv)
{
   // set the default tracking algorithm
    Ptr<TrackerCSRT> leftTracker = TrackerCSRT::create();
    Ptr<TrackerCSRT> rightTracker = TrackerCSRT::create();

    Rect2d leftRoi, rightRoi;
    Point leftCenter, rightCenter;
    bool leftTrackInited = false;
    bool rightTrackInited = false;
  
    CameraStreamer cam({0, 1});
 
    Mat baseSrcLeft, baseSrcRight;

    for(;;)
    {
        Mat srcLeft = baseSrcLeft.clone();
        Mat srcRight = baseSrcRight.clone();

        Mat srcGrayLeft, srcGrayRight;

        Mat temp1;
        Mat temp2;
        while (cam.frame_queue[1]->try_pop(temp2)) { 
            srcRight = temp2.clone();
            baseSrcRight = temp2.clone();
        }
        while (cam.frame_queue[0]->try_pop(temp1)) { 
            srcLeft = temp1.clone();
            baseSrcLeft = temp1.clone();
        }
        if(srcLeft.empty() || srcRight.empty()) {
            continue;
        } 
        auto start = chrono::steady_clock::now();
        transpose(srcRight, srcRight);
        flip(srcRight, srcRight, 0);
        transpose(srcLeft, srcLeft);
        flip(srcLeft, srcLeft, 0);

        flip(srcLeft, srcLeft, 0);
        flip(srcLeft, srcLeft, +1);

        if(leftTrackInited) {
            if(!leftTracker->update(srcLeft, leftRoi)) {
                leftRoi=Rect2d();
                cout << "fail left" << endl;
                leftTrackInited = false;
            }
        }
        if(rightTrackInited) {
            if(!rightTracker->update(srcRight, rightRoi)) {
                rightRoi=Rect2d();
                cout << "fail right" << endl;
                rightTrackInited = false;
            }
        }


        if( waitKey(10) == 27 ) break; // stop capturing by pressing ESC 

        /// Convert it to gray
        cvtColor(srcLeft, srcGrayLeft, COLOR_BGR2GRAY);
        cvtColor(srcRight, srcGrayRight, COLOR_BGR2GRAY);

        /// Reduce the noise so we avoid false circle detection
        GaussianBlur(srcGrayLeft, srcGrayLeft, Size(9,9), 2, 2);
        GaussianBlur(srcGrayRight, srcGrayRight, Size(9,9), 2, 2);

        vector<Vec3f> circlesLeft;
        vector<Vec3f> circlesRight;

        /// Apply the Hough Transform to find the circles
        HoughCircles(srcGrayLeft, circlesLeft, HOUGH_GRADIENT, 0.8, srcGrayLeft.rows / 8, 100, 50, 0, 0);
        HoughCircles(srcGrayRight, circlesRight, HOUGH_GRADIENT, 0.8, srcGrayRight.rows / 8, 100, 50, 0, 0);
        #define MARGIN 10

        /// Draw the circles detected
        for (size_t i = 0; i < circlesLeft.size(); i++)
        {
            Point center(cvRound(circlesLeft[i][0]), cvRound(circlesLeft[i][1]));
            int radius = cvRound(circlesLeft[i][2]);
            leftCenter = center;
            // circle center
            circle(srcLeft, center, 3, Scalar(0, 255, 0), -1, 8, 0);
            // circle outline
            circle(srcLeft, center, radius, Scalar(0, 0, 255), 3, 8, 0);
            leftRoi=Rect2d(Point(center.x-radius-MARGIN/2, center.y-radius-MARGIN/2), Size(radius*2+MARGIN, radius*2+MARGIN));
            leftTracker->init(srcLeft,leftRoi);
            leftTrackInited = true;
        }
        for (size_t i = 0; i < circlesRight.size(); i++)
        {
            Point center(cvRound(circlesRight[i][0]), cvRound(circlesRight[i][1]));
            rightCenter = center;
            int radius = cvRound(circlesRight[i][2]);
            // circle center
            circle(srcLeft, center, 3, Scalar(0, 255, 255), -1, 8, 0);
            // circle outline
            circle(srcLeft, center, radius, Scalar(0, 255, 255), 3, 8, 0);
            rightRoi=Rect2d(Point(center.x-radius-MARGIN/2, center.y-radius-MARGIN/2), Size(radius*2+MARGIN, radius*2+MARGIN));
            rightTracker->init(srcRight,rightRoi);
            rightTrackInited = true;
        }
        if(leftTrackInited)  
            rectangle( srcLeft, leftRoi, Scalar( 0, 255, 0 ), 2, 1 );
        if(rightTrackInited) 
            rectangle( srcLeft, rightRoi, Scalar(0, 255, 255), 2, 1 );

        int distance = 400 - (leftRoi.tl().x + leftRoi.width / 2 - rightRoi.tl().x + rightRoi.width / 2);
        cv::putText(srcLeft, //target image
                    "d = " + to_string(distance), //text
                    cv::Point(10, 50), //top-left position
                    cv::FONT_HERSHEY_DUPLEX,
                    1.0,
                    CV_RGB(118, 185, 0), //font color
                    2);


        auto end = chrono::steady_clock::now();

        cout << "Elapsed time in ms : " 
            << chrono::duration_cast<chrono::milliseconds>(end - start).count()
            << " ms" << endl;

        /// Show your results
        imshow("Hough Circle Transform Demo left", srcLeft);
        imshow("Hough Circle Transform Demo right", srcRight);
    }
    return 0;
}
