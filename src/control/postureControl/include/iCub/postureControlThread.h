// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2013  Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
  * Author:Francesco Rea
  * email: francesco.reak@iit.it
  * Permission is granted to copy, distribute, and/or modify this program
  * under the terms of the GNU General Public License, version 2 or any
  * later version published by the Free Software Foundation.
  *
  * A copy of the license can be found at
  * http://www.robotcub.org/icub/license/gpl.txt
  *
  * This program is distributed in the hope that it will be useful, but
  * WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
  * Public License for more details
*/

/**
 * @file postureControlThread.h
 * @brief Definition of a thread that receives an RGN image from input port and sends it to the output port.
 */


#ifndef _POSTURE_CONTROL_THREAD_H_
#define _POSTURE_CONTROL_THREAD_H_

#include <yarp/sig/all.h>
#include <yarp/os/all.h>
#include <yarp/dev/all.h>
#include <yarp/os/RateThread.h>
#include <iostream>
#include <fstream>
#include <time.h>

#define COMMAND_RIGHT_ARM    VOCAB4('r','a','r','m')
#define COMMAND_LEFT_ARM     VOCAB4('l','a','r','m')
#define COMMAND_TORSO        VOCAB4('t','o','r','s')
#define COMMAND_RIGHT_HAND   VOCAB4('r','h','a','n')
#define COMMAND_LEFT_HAND    VOCAB4('l','h','a','n')

class postureControlThread : public yarp::os::Thread {
private:
    int jntsRightArm;               // number of jointRight arm
    int jntsLeftArm;                // number of jointLeft arm
    int jntsTorso;                  // number of jointTorso
    
    std::string robot;              // name of the robot
    std::string configFile;         // name of the configFile where the parameter of the camera are set
    std::string inputPortName;      // name of input port for incoming events, typically from aexGrabber
    
    yarp::sig::Vector encodersRightArm;      // encoders position
    yarp::sig::Vector encodersLeftArm;       // encoders position
    yarp::sig::Vector encodersTorsoArm;      // encoders position
    yarp::sig::Vector encoders;              // encoders position

    yarp::dev::PolyDriver        *robotDeviceRightArm;  // device right arm
    yarp::dev::PolyDriver        *robotDeviceLeftArm;   // device left arm
    yarp::dev::PolyDriver        *robotDeviceTorso;     // device torso
    
    yarp::dev::IPositionControl  *posRightArm;          // position control of the robot RightArm
    yarp::dev::IEncoders         *encsRightArm;         // encoders readings from the robot RightArm
    yarp::dev::IControlMode      *ictrlRightArm;        // sets the modality of the control RightArm
    yarp::dev::IImpedanceControl *iimpRightArm;         // impedence control of the robot RightArm
    yarp::dev::ITorqueControl    *itrqRightArm;         // torque control of the robot RightArm

    yarp::dev::IPositionControl  *posLeftArm;           // position control of the robot LeftArm
    yarp::dev::IEncoders         *encsLeftArm;          // encoders readings from the robot LeftArm
    yarp::dev::IControlMode      *ictrlLeftArm;         // sets the modality of the control LeftArm
    yarp::dev::IImpedanceControl *iimpLeftArm;          // impedence control of the robot LeftArm
    yarp::dev::ITorqueControl    *itrqLeftArm;          // torque control of the robot LeftArm

    yarp::dev::IPositionControl  *posTorso;             // position control of the robot Torso
    yarp::dev::IEncoders         *encsTorso;            // encoders readings from the robot Torso
    yarp::dev::IControlMode      *ictrlTorso;           // sets the modality of the control Torso
    yarp::dev::IImpedanceControl *iimpTorso;            // impedence control of the robot Torso
    yarp::dev::ITorqueControl    *itrqTorso;            // torque control of the robot Torso

    yarp::sig::ImageOf<yarp::sig::PixelRgb>* inputImage;

    yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> > inputCallbackPort;
    yarp::os::BufferedPort<yarp::os::Bottle > inputRightArm;                             //input port for right arm
    yarp::os::BufferedPort<yarp::os::Bottle > inputLeftArm;                              //input port for left arm
    yarp::os::BufferedPort<yarp::os::Bottle > interfaceIn;                               //input port for left arm
    
    std::string name;                                                                    // rootname of all the ports opened by this thread
    
    static const double armMin[];
    static const double armMax[];
    static const double torsoMin[];
    static const double torsoMax[];
    
public:
    /**
    * constructor default
    */
    postureControlThread();

    /**
    * constructor 
    * @param robotname name of the robot
    */
    postureControlThread(std::string robotname,std::string configFile);

    /**
     * destructor
     */
    ~postureControlThread();

    /**
    *  initialises the thread
    */
    bool threadInit();

    /**
    *  correctly releases the thread
    */
    void threadRelease();

    /**
    *  active part of the thread
    */
    void run(); 

    /**
    *  on stopping of the thread
    */
    void onStop();

    /*
    * function that sets the rootname of all the ports that are going to be created by the thread
    * @param str rootnma
    */
    void setName(std::string str);
    
    /**
    * function that returns the original root name and appends another string iff passed as parameter
    * @param p pointer to the string that has to be added
    * @return rootname 
    */
    std::string getName(const char* p);

    /*
    * function that sets the inputPort name
    */
    void setInputPortName(std::string inpPrtName);

    /*
    * function that sets the robotName
    */
    void setRobot(std::string robotName){robot = robotName;};

    /*
    * function that initialises the controller of all the bodyparts
    */
    bool initController();

    /*
    * function that checks the validity of the content of the arm bottle
    * @param b bottle that has arm position
    */
    const bool checkArm(const yarp::os::Bottle* b);

    /*
    * function that checks the validity of the hand bottle
    * @param b bottle that has arm position
    */
    const bool checkHand(const yarp::os::Bottle* b);

    /*
    * function that checks the validity of the content of the torso bottle
    * @param b bottle that has torso position
    */
    const bool checkTorso(const yarp::os::Bottle* b);

    /**
     * function that parses the bottle received and generate the vector of the control pos
     * @param b received bottle
     * @param dim dimension of the control vector
     * @result vector of the control of the body part
     */
    void parseBottle(yarp::dev::IEncoders *encs, yarp::os::Bottle* b,const int dim,const int shift, yarp::sig::Vector* result);


};



#endif  //_POSTURE_CONTROL_THREAD_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------

