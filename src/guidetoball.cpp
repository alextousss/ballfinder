#include "guidetoball.hpp"

using namespace cv;
using namespace std;


Point midpoint(const Point& a, const Point& b) {
    Point ret;
    ret.x = (a.x + b.x) / 2;
    ret.y = (a.y + b.y) / 2;
    return ret;
}


BallGuider::BallGuider() : mStreamer({0, 1}) {
    namedWindow("left");
    namedWindow("right");
}


void BallGuider::update() {
    Mat srcLeft = mBaseSrcLeft.clone();
    Mat srcRight = mBaseSrcRight.clone();
    
    Mat srcGrayLeft, srcGrayRight;

    Mat temp1;
    Mat temp2;
    while (mStreamer.frame_queue[1]->try_pop(temp2)) { 
        if(!temp2.empty()) {
            srcRight = temp2.clone();
            mBaseSrcRight = temp2.clone();
        }
    }
    while (mStreamer.frame_queue[0]->try_pop(temp1)) { 
        srcLeft = temp1.clone();
        mBaseSrcLeft = temp1.clone();
    }
    if(srcLeft.empty() || srcRight.empty()) {
        return;
    } 
    auto start = chrono::steady_clock::now();
    transpose(srcRight, srcRight);
    flip(srcRight, srcRight, 0);
    transpose(srcLeft, srcLeft);
    flip(srcLeft, srcLeft, 0);

    flip(srcLeft, srcLeft, 0);
    flip(srcLeft, srcLeft, +1);

    mLeftFinder.update(srcLeft);
    mRightFinder.update(srcRight);
    Ball pos1 = mLeftFinder.getBallPos();
    Ball pos2 = mRightFinder.getBallPos();
    circle(srcLeft, pos1.pos, 3, Scalar(0, 0, 255), -1, 8, 0);
    circle(srcLeft, pos2.pos, 3, Scalar(0, 255, 0), -1, 8, 0);

    Point order(130, 225);
    Point ballPos = midpoint(pos1.pos, pos2.pos);
    circle(srcLeft, order, 3, Scalar(255, 0, 255), -1, 8, 0);



    int distanceX = pos1.pos.x - pos2.pos.x;
    int distanceY = pos1.pos.y - pos2.pos.y;


    cv::putText(srcLeft, //target image
        "d = " + to_string(distanceX), //text
        cv::Point(10, 50), //top-left position
        cv::FONT_HERSHEY_DUPLEX,
        1.0,
        CV_RGB(118, 185, 0), //font color
        2
    );
    

    if (distanceX < -10 && abs(distanceY) < 15 && norm(order-ballPos) < 20) {
        mCommand.pince = true;
        mCommand.z = 0;
    } else {
        mCommand.z = 1;
        mCommand.pince = false;
    }

    if(abs(distanceY) < 5 && pos1.pos.x != 0 && pos2.pos.x != 0 && norm(order-ballPos) > 10 ) {
        mCommand.x = ballPos.x < order.x ? -1 : 1;
        mCommand.y = ballPos.y < order.y ? -1 : 1;
    } else {
        mCommand = {0, 0, 0, 0};
    }

    if (distanceX < -10 && abs(distanceY) < 15 && norm(order-ballPos) < 20) {
        mCommand.pince = true;
    } else {
        mCommand.pince = false;
    }

    auto end = chrono::steady_clock::now();

    if(waitKey(1) >= 0) exit(0);

    cout << "Elapsed time in ms : " 
        << chrono::duration_cast<chrono::milliseconds>(end - start).count()
        << " ms" << endl;

    /// Show your results
    imshow("left", srcLeft);
    imshow("right", srcRight);

}