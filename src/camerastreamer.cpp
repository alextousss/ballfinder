#include "camerastreamer.hpp"
 
CameraStreamer::CameraStreamer(vector<string> stream_source)
{
camera_source = stream_source;
camera_count = camera_source.size();
isUSBCamera = false;
 
startMultiCapture();
}
 
CameraStreamer::CameraStreamer(vector<int> capture_index)
{
camera_index = capture_index;
camera_count = capture_index.size();
isUSBCamera = true;
 
startMultiCapture();
}
 
CameraStreamer::~CameraStreamer()
{
stopMultiCapture();
}
 
void CameraStreamer::captureFrame(int index)
{
VideoCapture *capture = camera_capture[index];
while (true)
{
Mat frame;
//Grab frame from camera capture
(*capture) >> frame;
//Put frame to the queue
frame_queue[index]->push(frame);
//relase frame resource
frame.release();
}
}
 
void CameraStreamer::startMultiCapture()
{
VideoCapture *capture;
thread *t;
concurrent_queue<Mat> *q;
for (int i = 0; i < camera_count; i++)
{
//Make VideoCapture instance
if (!isUSBCamera){
string url = camera_source[i];
capture = new VideoCapture(url);
cout << "Camera Setup: " << url << endl;
}
else{
int idx = camera_index[i];
capture = new VideoCapture(idx);
    capture->set(CV_CAP_PROP_FOURCC, CV_FOURCC('M','J','P','G'));
    capture->set(CV_CAP_PROP_FPS , 10);
    capture->set(CV_CAP_PROP_FRAME_WIDTH,640);
    capture->set(CV_CAP_PROP_FRAME_HEIGHT,480);

cout << "Camera Setup: " << to_string(idx) << endl;
}
 
//Put VideoCapture to the vector
camera_capture.push_back(capture);
 
//Make thread instance
t = new thread(&CameraStreamer::captureFrame, this, i);
 
//Put thread to the vector
camera_thread.push_back(t);
 
//Make a queue instance
q = new concurrent_queue<Mat>;
 
//Put queue to the vector
frame_queue.push_back(q);
}
}
 
void CameraStreamer::stopMultiCapture()
{
VideoCapture *cap;
for (int i = 0; i < camera_count; i++) {
cap = camera_capture[i];
if (cap->isOpened()){
//Relase VideoCapture resource
cap->release();
cout << "Capture " << i << " released" << endl;
}
}
}