// FOUND AT : https://putuyuwono.wordpress.com/2015/05/29/multi-thread-multi-camera-capture-using-opencv/


#pragma once
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <tbb/concurrent_queue.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

 
using namespace std;
using namespace tbb;
using namespace cv;

class CameraStreamer{
public:
//this holds camera stream urls
vector<string> camera_source;
//this holds usb camera indices
vector<int> camera_index;
//this holds OpenCV VideoCapture pointers
vector<VideoCapture*> camera_capture;
//this holds queue(s) which hold images from each camera
vector<concurrent_queue<Mat>*> frame_queue;
//this holds thread(s) which run the camera capture process
vector<thread*> camera_thread;
 
//Constructor for IP Camera capture
CameraStreamer(vector<string> source);
//Constructor for USB Camera capture
CameraStreamer(vector<int> index);
//Destructor for releasing resource(s)
~CameraStreamer();
 
private:
bool isUSBCamera;
int camera_count;
//initialize and start the camera capturing process(es)
void startMultiCapture();
//release all camera capture resource(s)
void stopMultiCapture();
//main camera capturing process which will be done by the thread(s)
void captureFrame(int index);
};