#include <iostream>

#include "guidetoball.hpp"
#include "owiarm.hpp"
#include "utils.hpp"
#include "myoband.hpp"
#include "ecginterpreter.hpp"
#include <chrono>
#include <unistd.h>
#include <thread>

using namespace std;
using namespace sf;


void continuouslyUpdateGuider(BallGuider *guider) {
    while(true) {
        guider->update();
    }
}

void continuouslyUpdateArm(OwiArm *arm) {
    while(true) {
        arm->update();
    }
}


int main(int argc, char **argv) {
    BallGuider guider;
    OwiArm arm;
    arm.update();
    MyoBand band;
    EcgInterpreter interpreter;
    RenderWindow window(VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "yArm");
    yArm::Vec3f baseOrientation = toEulerAngle(band.getOrientation());
    bool visionHasTheControl = false;
    bool pinceState = false;
    bool pinceOpen = false;
    long long lastPinceMove = 0;
    long long lastPrint = 0;
    arm.setOrientation(baseOrientation); // A l'initialisation, le bras est à la position du myo

    std::thread guiderUpdateThread(continuouslyUpdateGuider, &guider);
    std::thread armUpdateThread(continuouslyUpdateArm, &arm);

    while (window.isOpen()) {
        window.clear(sf::Color::Black);
        sf::Event event;
        //yArm::sleepMS(100);
        //guider.update();
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed)
                switch(event.key.code) {
                    case sf::Keyboard::Escape:
                        window.close();
                        break;
                    case sf::Keyboard::O:
                        arm.openPince();
                        pinceOpen = true;
                        break;

                    case sf::Keyboard::V:
                        visionHasTheControl = !visionHasTheControl;
                        break;
                    default:
                        break;
                }

        }
        double data[8];
        band.update();
        band.getEMGArray(data);
        interpreter.feedData(data);
        window.draw(interpreter);
        window.display();
        yArm::Vec4f orientation = band.getOrientation();
        yArm::Vec3f actualArmOrientation = arm.getOrientation();
        yArm::Vec3f target = toEulerAngle(orientation);
        if(yArm::secondsSince(lastPrint) > 0.1) {
            cout << "target y\t" << target.y << "\tactual z:\t" << actualArmOrientation.z << endl;
            lastPrint=yArm::getMicrotime();
        }
        target.z = 0;
        if(visionHasTheControl) {
            target = {
                arm.getOrientation().x + guider.getCommand().x * 10,
                arm.getOrientation().y - guider.getCommand().y * 10,
                arm.getOrientation().z - guider.getCommand().z * 10,
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
        }
        arm.setTargetOrientation(target);
        if(interpreter.getPrediction() && yArm::secondsSince(lastPinceMove) > 5) {
            /*if(visionHasTheControl) {

                //arm.openPince();
            } else {
                //arm.closePince();
            }*/
            visionHasTheControl = !visionHasTheControl;
            lastPinceMove = yArm::getMicrotime();
        }
    }
}
