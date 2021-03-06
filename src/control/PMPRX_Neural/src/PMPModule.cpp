// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2013  Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
  * Author:Vishwanathan Mohan
  * email: Vishwanathan.Mohan@iit.it
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

#include "PMPModule.h"

using namespace yarp::os;
using namespace yarp::sig;
using namespace std;

using namespace darwin::common;
using namespace darwin::msg;
using namespace darwin::reach;

/* 
 * Configure method. Receive a previously initialized
 * resource finder object. Use it to configure your module.
 * If you are migrating from the old Module, this is the 
 *  equivalent of the "open" method.
 */

bool PMPModule::configure(yarp::os::ResourceFinder &rf) {
    /* Process all parameters from both command-line and .ini file */

    /* get the module name which will form the stem of all module port names */
    moduleName            = rf.check("name", 
                           Value("/PMP"), 
                           "module name (string)").asString();
    /*
    * before continuing, set the module name before getting any other parameters, 
    * specifically the port names which are dependent on the module name
    */
    setName(moduleName.c_str());

    /*
    * get the robot name which will form the stem of the robot ports names
    * and append the specific part and device required
    */
    robotName               = rf.check("robot", 
                           Value("icub"), 
                           "Robot name (string)").asString();
                           
    printf("robotName in Module %s \n", robotName.c_str());
    //robotPortName         = "/" + robotName + "/head";

    inputPortName           = rf.check("inputPortName",
			                Value(":i"),
                            "Input port name (string)").asString();

                            
    if (rf.check("verboseTerm")) {
        verboseTerm = true;
    }
    else {
        verboseTerm = false;
    }
    
    if (rf.check("verboseFile")) {
        verboseFile = true;
    }
    else {
        verboseFile = false;
    }
    
    /*
    * attach a port of the same name as the module (prefixed with a /) to the module
    * so that messages received from the port are redirected to the respond method
    */
    handlerPortName =  "";
    handlerPortName += getName();         // use getName() rather than a literal 

    if (!handlerPort.open(handlerPortName.c_str())) {           
        cout << getName() << ": Unable to open port " << handlerPortName << endl;  
        return false;
    }

    attach(handlerPort);                  // attach to port
    if (rf.check("config")) {
        configFile=rf.findFile(rf.find("config").asString().c_str());
        if (configFile=="") {
            return false;
        }
    }
    else {
        configFile.clear();
    }

	_quit = false;
	bool returnval = true;
	//create the thread and configure it using the ResourceFinder
    _kinematicsThread = new KinematicsThread();
	returnval &= _kinematicsThread->configure(rf); 
 
	/* create the thread and pass pointers to the module parameters */
    rThread = new PMPThread(robotName, configFile);
    rThread->setName(getName().c_str());
    rThread->setRobotName(robotName);
    rThread->setVerbose(verboseFile,verboseTerm);
	rThread->setKinematicsAccess(_kinematicsThread);
	_kinematicsThread->setPMPAccess(rThread);

    if (rf.check("neuralNet")) {
        int nLayers  = rf.check("nLayers",
			                Value(3),
                            "number of layers in the neural network (int)").asInt();
        printf("\n working with neural network of size %d \n", nLayers);

        
        std::ostringstream o;

        for(int i=1; i <=nLayers; i++){
            printf("checking for layer %d characteristics \n",i);
            string s,s1="weights",s2="biases";

         

            //o << i;
            //s = o.str();
            
            string sLayerN(" ");
            sprintf((char*)sLayerN.c_str(),"%d",i);
            s1.append(sLayerN);
            
            printf("string: %s \n",s1.c_str());
            if (rf.check(s1.c_str())) {
                weightsPath = rf.findFile(rf.find(s1.c_str()).asString().c_str());
                if (weightsPath=="") {
                    return false;
                }
                else {
                    printf("found weight file %s \n", weightsPath.c_str());
                }
            }
            else {
                weightsPath.clear();
            }        
            
            rThread->setWeightsPath(weightsPath,i);

            s2.append(sLayerN);    

            if (rf.check(s2.c_str())) {
                biasesPath = rf.findFile(rf.find(s2.c_str()).asString().c_str());
                if (biasesPath=="") {
                    return false;
                }
                else {
                    printf("found biase file %s \n", biasesPath.c_str());
                }
            }
            else {
                biasesPath.clear();
            }        
            
            rThread->setBiasesPath(biasesPath,i);

        }







		for(int i=1; i <=nLayers; i++){
            printf("checking for layer %d characteristics \n",i);
            string s,s1="weights",s2="biases";

         

            //o << i;
            //s = o.str();
            
            string sLayerN(" ");
            sprintf((char*)sLayerN.c_str(),"%d",i+3);
            s1.append(sLayerN);
			
            
            printf("string: %s \n",s1.c_str());
            if (rf.check(s1.c_str())) {
                weightsPath = rf.findFile(rf.find(s1.c_str()).asString().c_str());
                if (weightsPath=="") {
                    return false;
                }
                else {
                    printf("found weight file %s \n", weightsPath.c_str());
                }
            }
            else {
                weightsPath.clear();
            }        
            
            rThread->setWeightsPath(weightsPath,i+3);

            s2.append(sLayerN);    

            if (rf.check(s2.c_str())) {
                biasesPath = rf.findFile(rf.find(s2.c_str()).asString().c_str());
                if (biasesPath=="") {
                    return false;
                }
                else {
                    printf("found biase file %s \n", biasesPath.c_str());
                }
            }
            else {
                biasesPath.clear();
            }        
            
            rThread->setBiasesPath(biasesPath,i+3);

        }

        //rThread->setNeuralNetwork(true);
    }
    
	//=======================================================================

	if (returnval) {
		returnval &= _kinematicsThread->start();
	}

	if (returnval) {
		returnval &= rThread->start(); // this calls threadInit() and it if returns true, it then calls run()
	}
	
	return returnval;   // let the RFModule know everything went well
                        // so that it will then run the module
}

bool PMPModule::interruptModule() {
    handlerPort.interrupt();
    return true;
}

bool PMPModule::close() {
    handlerPort.close();
    /* stop the thread */
    printf("stopping the thread \n");
	_kinematicsThread->stop();
    rThread->stop();
    printf("PMPModule::close:end \n");
    return true;
}

bool PMPModule::respond(const Bottle& command, Bottle& reply) 
{
    string helpMessage =  string(getName().c_str()) + 
                "commands are: \n" +  
                " help \n" +
                " quit \n" +
				" reset + lhan|rhan\n";
    reply.clear(); 

	ConstString str = command.get(0).asString();
    if (str=="quit") {
        reply.addString("quitting");
        return false;     
    }
    else if (str=="help") {
        cout << helpMessage;
        reply.addString(helpMessage.c_str());
    }
	else if (str=="reset" && command.size()==2) {
		_kinematicsThread->resetEffector(static_cast<GraspEffectorType>(command.get(1).asVocab()));
		reply.addString("Reset");
	}
	else {
		reply.addString("Unknown command");
	}
    return true;
}

/* Called periodically every getPeriod() seconds */
bool PMPModule::updateModule()
{
	if (_quit) {
		close();
		return false;
	}

    return true;
}

double PMPModule::getPeriod()
{
    /* module periodicity (seconds), called implicitly by myModule */
    return 1;
}

