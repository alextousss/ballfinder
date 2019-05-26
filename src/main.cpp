#include <iostream>

#include "guidetoball.hpp"
#include "owiarm.hpp"
#include "utils.hpp"
#include <chrono>
#include <unistd.h>

using namespace std;




int main(int argc, char **argv) {
    auto lastArmUpdate = chrono::steady_clock::now();
    BallGuider guider;
    OwiArm arm;
    bool pinceOpen = false;
    for(;;) {
        guider.update();
        auto end = chrono::steady_clock::now();

		if(chrono::duration_cast<chrono::milliseconds>(end - lastArmUpdate).count() > 10) {
            lastArmUpdate = chrono::steady_clock::now();
            yArm::Vec3f nextTarget = {
                arm.getOrientation().x + guider.getCommand().x * 10,
                arm.getOrientation().y + guider.getCommand().y * 10,
                arm.getOrientation().z + guider.getCommand().z * 10,
            };
            if(guider.getCommand().pince) {
                if(pinceOpen) {
                    arm.closePince();
                }
            } else if(!pinceOpen) {
                arm.openPince();
                cout << "open pince" << endl;
                pinceOpen = true;
            }
            arm.setTargetOrientation(nextTarget);


            auto start = chrono::steady_clock::now();
            arm.update();
            auto end = chrono::steady_clock::now();
            cout << "Time to update : "
                << chrono::duration_cast<chrono::milliseconds>(end - start).count()
                << " ms" << endl;
        }


        cout << "x:" << guider.getCommand().x << "\t";
        cout << "y:" << guider.getCommand().y << "\t";
        cout << "z:" << guider.getCommand().z << "\t";
        cout << "pince:" << guider.getCommand().pince << endl;

    }
}
