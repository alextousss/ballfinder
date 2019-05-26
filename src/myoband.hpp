#ifndef MYO_WRAPPER
#define MYO_WRAPPER

#include "utils.hpp"
#include <myolinux/myoclient.h>
#include <myolinux/serial.h>


class MyoBand {
    public:
    MyoBand();
    void update();
    void vibrate(myolinux::myo::Vibration vib) {
        mClient.vibrate(vib);
    }
    // Accessors
    yArm::Vec4f getOrientation()  { return mOrientation; }
    yArm::Vec3f getAcceleration() { return mAcceleration; }
    yArm::Vec3f getAngularSpeed() { return mAngularSpeed; }
    void getEMGArray(double emgData[]) {
      for(unsigned int i = 0 ; i < 8 ; i++)
         emgData[i] = mEmgData[i];
    }
    private:
    yArm::Vec4f       mOrientation;
    yArm::Vec3f       mAcceleration;
    yArm::Vec3f       mAngularSpeed;
    double      mEmgData[8];
    myolinux::myo::Client mClient;
};


#endif
