// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2013  Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
  * Author:Ajaz Bhat
  * email: ajaz.bhat@iit.it
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
 * @file main.cpp
 * @brief main code for the emPlotter module.
 */

#include "visualizationControlModule.h" 

using namespace yarp::os;
using namespace yarp::sig;


int main(int argc, char * argv[])
{
    
    Network yarp;
    visualizationControlModule module; 

    ResourceFinder rf;
    rf.setVerbose(true);
    rf.setDefaultConfigFile("visualizationControl.ini");    //overridden by --from parameter
    rf.setDefaultContext("morphoGenApp/conf");    //overridden by --context parameter
    rf.configure(argc, argv);  
 
    module.runModule(rf);
    return 0;
}


