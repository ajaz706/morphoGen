// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2013  Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
  * Author:Vishwanathan Mohan, Rea Francesco, Ajaz A Bhat
  * email: Vishwanathan.Mohan@iit.it, francesco.rea@iit.it, ajaz.bhat@iit.it

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

#include <PMPThread.h>
#include <utils.h>
#include <cstring>
#include <string>
#include <time.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <time.h>
#include <string>

using namespace yarp::dev;
using namespace yarp::os;
using namespace yarp::sig;
using namespace std;

#define GoalCodePMP_DISC	1
#define GoalCodePMP_CONT	21
#define GoalCodePMP_INIT	19
#define BodyTorsoArm_RIGHT	0
#define BodyTorsoArm_LEFT	1
#define BodyTorsoArm_BOTH	2
#define MSim_SIMULATION		0
#define MSim_MOVEMENT		1
#define Trajectory_LINE		0
#define Trajectory_BUMP		1
#define Trajectory_CUSP		2
#define Trajectory_CURV		4
#define Wrist_SIDEGRASP		0
#define Wrist_TOPGRASP		90
#define rad2degree          180/3.14159


#define NEURALNETWORK   //By default PMP uses Neural network solution to PMP
                        //Please comment out this line if you want to run the analytical version of PMP

#define CMD_RIGHT_ARM	VOCAB4('r','a','r','m')
#define CMD_LEFT_ARM	VOCAB4('l','a','r','m')
#define CMD_RIGHT_HAND	VOCAB4('r','h','a','n')
#define CMD_LEFT_HAND	VOCAB4('l','h','a','n')
#define CMD_TORSO 		VOCAB4('t','o','r','s')

PMPThread::PMPThread() {
    robot = "icub";
}

PMPThread::PMPThread(string _robot, string _configFile){
    robot = _robot;
    configFile = _configFile;
    printf("\n robot:\t %s", robot.c_str());
}

PMPThread::~PMPThread() {
    // do nothing
}

bool PMPThread::threadInit() {


    /*if (!Inp3D.open(getName("/input/coordinates:i").c_str())) {
        cout << ": unable to open port to send unmasked events "  << endl;
        return false;  // unable to open; let RFModule know so that it won't run
    }


    if (!MotCom.open(getName("/v:o").c_str())) {
        cout << ": unable to open port to send unmasked events "  << endl;
        return false;  // unable to open; let RFModule know so that it won't run
    } */

    if (!PMPResponse.open(getName("/PMPreply:io").c_str())) {
        cout << ": unable to open port to send unmasked events "  << endl;
        return false;  // unable to open; let RFModule know so that it won't run
    }

    /*if (!Inpjoints.open(getName("/input/joints:i").c_str())) {
        cout << ": unable to open port /input/joints to send unmasked events "  << endl;
        return false;  // unable to open; let RFModule know so that it won't run
    }*/

    if (!cmdRight_armPort.open(getName("/cmd/right_arm:o").c_str())) {
        cout << ": unable to open port to send unmasked events "  << endl;
        return false;  // unable to open; let RFModule know so that it won't run
    }

    if (!cmdLeft_armPort.open(getName("/cmd/left_arm:o").c_str())) {
        cout << ": unable to open port to send unmasked events "  << endl;
        return false;  // unable to open; let RFModule know so that it won't run
    }

    if (!cmdTorsoPort.open(getName("/cmd/torso:o").c_str())) {
        cout << ": unable to open port to send unmasked events "  << endl;
        return false;  // unable to open; let RFModule know so that it won't run
    }

    if (!cmdInterfacePort.open(getName("/cmd/interface:o").c_str())) {
        cout << ": unable to open port to send unmasked events "  << endl;
        return false;  // unable to open; let RFModule know so that it won't run
    }

	if (!cmdEndEffectorPort.open(getName("/cmd/endEffector:o").c_str())) {
        cout << ": unable to open port to send unmasked events "  << endl;
        return false;  // unable to open; let RFModule know so that it won't run
    }

    if (!activationsPort.open(getName("/activations:o").c_str())) {
        cout << ": unable to open port to send activations "  << endl;
        return false;  // unable to open; let RFModule know so that it won't run
    }

	if (!gazePort.open(getName("/gazeCoord:o").c_str())) {
        cout << ": unable to open port to send activations "  << endl;
        return false;  // unable to open; let RFModule know so that it won't run
    }
    if (!right_arm_joints_Current.open(getName("/rightArmJoints:i").c_str())) {
        cout << ": unable to open port to send activations "  << endl;
        return false;  // unable to open; let RFModule know so that it won't run
    }
    if (!left_arm_joints_Current.open(getName("/leftArmJoints:i").c_str())) {
        cout << ": unable to open port to send activations "  << endl;
        return false;  // unable to open; let RFModule know so that it won't run
    }
    if (!torso_joints_Current.open(getName("/torsoJoints:i").c_str())) {
        cout << ": unable to open port to send activations "  << endl;
        return false;  // unable to open; let RFModule know so that it won't run
    }

    if(!(Network::isConnected("/PMP/cmd/interface:o", "/psControl/interface:i"))){
		Network::connect("/PMP/cmd/interface:o", "/psControl/interface:i");
	}
	if(!(Network::isConnected("/PMP/gazeCoord:o", "/iKinGazeCtrl/xd:i"))){
		Network::connect("/PMP/gazeCoord:o", "/iKinGazeCtrl/xd:i");
	}
    //depreciated hardcoded connections

   // Network::connect("/PMP/cmd/right_arm:o","/icub/right_arm/rpc:i");
    //Network::connect("PMP/cmd/left_arm:o","/icub/left_arm/rpc:i");
  //  Network::connect("/PMP/cmd/torso:o","/icub/torso/rpc:i");

    //if (Network::exists("/v:o"))
    //Network::connect("/v:o", "/input/moveRobots");  //check this
    //if (Network::exists("/input/joints:i"))
    //Network::connect("/output/alljoints","/input/joints:i");


	//initial Joint angles
	memset(Jan ,0,10*sizeof(double));
	memset(JanL,0,10*sizeof(double));
    InitializeJan();
    // handling verbose option
    if (verboseTerm) {
        // Stores Output Gamma Function
        wr.open("Gamma.txt");
        // Stores Solution in Joint angles
        // Output of Target Generator
        wr1.open("target.txt");
        // X/Y Position reached
        posi.open("position.txt");
        // Stores Output Gamma Function
        wrL.open("GammaDisc.txt");
        // Stores Solution in Joint angles
        wr_GamL.open("resultL.txt");
        // Output of Target Generator
        wr1L.open("targetL.txt");
        // X/Y Position reached
        posiL.open("positionL.txt");
        wr_Gam.open("result.txt");
        jacFile = fopen ("jacobian.txt" , "w");
        wr_Test.open("resultTest.txt");///testing the result..temporary
    }
#ifdef NEURALNETWORK
    loadNeuralNetwork();
#endif
    //initialization of the state of PMP (default state)
    GoalCodePMP  = 1;
    BodyChain    = 0;
    MSimExec     = 1;
    TrajType     = 4;
    WristO       =  0;
	Wrist2		= 0;
    MiniGoal[0]  =  -300;  // target right arm x
    MiniGoal[1]  =  10;   // target right arm y
    MiniGoal[2]  =  25;
    MiniGoal[3]  =  0.0;   //obstacle x
    MiniGoal[4]  =  0.0;
    MiniGoal[5]  =  0.0;
    MiniGoal[6]  = -300;   //target left arm x
    MiniGoal[7]  =  -10;
    MiniGoal[8]  =  25;
    MiniGoal[9]  =  0.0;
    MiniGoal[10] =  0.0;
    MiniGoal[11] =  0.0;

    printf("\nPMP Thread initialization: successfully completed \n");


    return true;
}

void PMPThread::setName(string str) {

    this->name=str;
    printf("name: %s \n", name.c_str());
}


std::string PMPThread::getName(const char* p) {

    string str(name);
    str.append(p);
    return str;
}


void PMPThread::setWeightsPath(std::string s, int i) {
    switch(i){
    case 1: {weights1Path = s;} break;
    case 2: {weights2Path = s;} break;
    case 3: {weights3Path = s;} break;
    }

}

void PMPThread::setBiasesPath(std::string s, int i) {
    switch(i){
    case 1: {biases1Path = s;} break;
    case 2: {biases2Path = s;} break;
    case 3: {biases3Path = s;} break;
    }

}

void PMPThread::setInputPortName(string InpPort) {

}

void PMPThread::loadNeuralNetwork(){
    printf("\n loading Neural Network.... \n");
    int u,i;
    float s=0;
    int dec;
    Weight1=fopen(weights1Path.c_str(),"r");//input file to be given to the program
  	Weight2=fopen(weights2Path.c_str(),"r");
  	Weight3=fopen(weights3Path.c_str(),"r");
  	bias1=fopen(biases1Path.c_str(),"r");
  	bias2=fopen(biases2Path.c_str(),"r");
  	bias3=fopen(biases3Path.c_str(),"r");


	if(	bias1==NULL) {
		printf("Cant read the input file for bias1\n");
		return;
	}
	for(u=0;u<hiddenL1;u++) {
  		fscanf(bias1,"%f",&s);
  		//printf("\n\ndata testing  %f",s);
   		b1[u]=s;
        //printf("bias1 inserted values %f \n",b1[u]);
	}


	if(	Weight1==NULL) {
		printf("Cant read the input file for weights1 \n");
		return;
	}

	for(u=0;u<hiddenL1;u++) {
		for(i=0;i<inputL;i++) {
      		fscanf(Weight1,"%f",&s);
	  		//printf("\n\ndata testing  %f",s);
	   		w1[u][i]=s;
            //printf("bias1 inserted values %f \n",w1[u][i]);
	    }
	}

	if(	bias2==NULL) {
		printf("Cant read the input file for bias2\n");
		return;
	}
	for(u=0;u<hiddenL2;u++) {
  		fscanf(bias2,"%f",&s);
  		//printf("\n\ndata testing  %f",s);
   		b2[u]=s;

	}

	if(	Weight2==NULL)	{
		printf("Cant read the input file for weights2\n");
		return;
	}
	for(u=0;u<hiddenL2;u++) {
		for(i=0;i<hiddenL1;i++) {
      		fscanf(Weight2,"%f",&s);
	 		//printf("\n\ndata testing  %f",s);
	  		w2[u][i]=s;
	 	}
	}

	if(	Weight3==NULL){
		printf("Cant read the input file for weights3\n");
		return;
	}
	for(u=0;u<outputL;u++) {
		for(i=0;i<hiddenL2;i++) {
      		fscanf(Weight3,"%f",&s);
  			//printf("\n\ndata testing  %f",s);
	   		w3[u][i]=s;
		}
	}

	if(	bias3==NULL) {
		printf("Cant read the input file for bias3\n");
		return;
	}
	for(u=0;u<outputL;u++) {
  		fscanf(bias3,"%f",&s);
  		//printf("\n\ndata testing  %f",s);
   		b3[u]=s;

	}

    printf(" Neural network correctly loaded \n");

}


void PMPThread::setRobotName(string robName) {

    this->robot = robName;
    printf("Robot name: %s", robot.c_str());
}

void PMPThread::run() {
	//InitializeJan();
	//MiniGoal[0]=-300;
 //   MiniGoal[1]=200;
 //   MiniGoal[2]=200;
 //   ResPM = VTGS(MiniGoal,GoalCodePMP,BodyChain,MSimExec,WristO,TrajType);
	///*MiniGoal[0]=-400;
 //   MiniGoal[1]=150;
 //   MiniGoal[2]=80;*/
	//MiniGoal[0]=-400;
 //   MiniGoal[1]=-100;
 //   MiniGoal[2]=-150;
 //   ResPM = VTGS(MiniGoal,GoalCodePMP,BodyChain,MSimExec,WristO,TrajType);

//testing the  yxz data accuracy ..lines for generating data
/*    InitializeJan();
    for (int x = -200; x > -400 ; x-=30)
        for (int y = 0; y < 200 ; y+=30)//{
               //int z = 100;
            for (int z = 0; z < 200 ; z+=30){
                MiniGoal[0]=x;
                MiniGoal[1]=y;
                MiniGoal[2]=z;
                ResPM = VTGS(MiniGoal,GoalCodePMP,BodyChain,MSimExec,WristO,TrajType);

            }

*/
/*
//////////////////////////////////////////////
///testing neural activatons
    InitializeJan();

               x = -303;
               z = 23;
            for (int y = 0; y < 200 ; y+=){
                MiniGoal[0]=x;
                MiniGoal[1]=y;
                MiniGoal[2]=z;
                ResPM = VTGS(MiniGoal,GoalCodePMP,BodyChain,MSimExec,WristO,TrajType);
            }


*/
//    readCurrentJoints();
////////////////////original code////////////////////////////////
        printf("You can communicate with PMP with following VOCAB commands at rpc port /PMPreply:i \n");
        printf("Type in a VOCAB command followed by numbers if needed \n");
        printf("Choose action type - CACT 1 [simple reaching], 21 [reaching via a curve], 19 [initialize the robot] \n"); 
        printf("Choose the Body Chain for action - BCHA 0 ([Right_arm+Torso], 1 (Left_arm+Torso], 2 [Both arms and torso]  \n");
        printf(" Mode of Action Execution - MSIM 0 [Mental Simulation only], 1 [Send Motor Commands to Robot)]\n");   
        printf(" Set Target Location - MICG (x y z) [specifiy z y z location in cm]\n");   
        printf(" Command PMP to Reach - REA (after setting up above vocabs)\n"); 
        printf("Waiting for your next Vocab Command \n");
    while (isStopping() != true) {

		if (PMPResponse.getInputCount()) {
			if(!(Network::isConnected("/PMP/cmd/interface:o", "/psControl/interface:i"))){
				Network::connect("/PMP/cmd/interface:o", "/psControl/interface:i");
			}
			if(!(Network::isConnected("/PMP/gazeCoord:o", "/iKinGazeCtrl/xd:i"))){
				Network::connect("/PMP/gazeCoord:o", "/iKinGazeCtrl/xd:i");
			}
            //this will be replaced with a client request from Observer that is responsible for executing
            //========================================================================================
            Bottle ObsReq, ObsResp;
            Bottle readjoints;

            bool ok  = false;
            bool rec = false; // is the command recognized?
            float c1[3], c2[3];
            
            PMPResponse.read(ObsReq,true);

            if(!ObsReq.isNull()) {
                printf("%s \n",ObsReq.toString().c_str());
                cout<<"Vocab message received"<<endl;
                // request present
                // reading the typology of the command first element
				
                int cmd = ObsReq.get(0).asVocab();
                switch(cmd){
                case COMMAND_VOCAB_CACT:{
                  rec = true;
                  //Choice Act:1=reach, 21=curve, 19=initialize
                  printf("Goal Code PMP - choice act \n");
                  GoalCodePMP=ObsReq.get(1).asInt();
                  ok = true;
                }break;

               case COMMAND_VOCAB_BCHA:{
                  rec = true;
                  printf("Body Chain \n");
                  //Body Chain: 0,1,2
                  BodyChain=ObsReq.get(1).asInt();
                  ok = true;
                }break;

                case COMMAND_VOCAB_MSIM:{
                  rec = true;
                  printf(" MSIM : Mental Simulation\n");
                  //MSimExec: 0/1
                  MSimExec=ObsReq.get(1).asInt();
                  ok = true;
                }break;

                case COMMAND_VOCAB_TRAJ:{
                  rec = true;
                  printf("TRAJ :  Trajectory Type \n");
                  //TrajType: 0,1,2 for line bump and cusp if goal code is 21 i.e traj synthesis
                  TrajType=ObsReq.get(1).asInt();
                  ok = true;
                }break;

                case COMMAND_VOCAB_WRIO:{
                  rec = true;
                  printf("WRIO : Wrist Orientation \n");
                  // wristO:orient: in angle degrees
                  WristO=ObsReq.get(1).asDouble();
                  ok = true;
                }break;
	
				case COMMAND_VOCAB_WRIS:{
                  rec = true;
                  printf("WRI2 :3rd Wrist joint Orientation \n");
                  // wristO:orient: in angle degrees
                  Wrist2=ObsReq.get(1).asDouble();
                  ok = true;
                }break;

                case COMMAND_VOCAB_MICG:{
                  rec = true;
                  Bottle* Coord = ObsReq.get(1).asList();
                  
                  for(int i = 0; i < 3; i++)	{
					c1[i] = Coord->get(i).asDouble();
					cout << "MICG : Receiving micro goal from the Observer client" <<  c1[i] << endl;
                  }
                    //////setting micro goal MICG coordinates correctly
                  if (BodyChain == 0){
                    for(int i = 0; i < 3; i++)	{
					    MiniGoal[i] = c1[i];
                    }
                  }
				  if (BodyChain == 1) {
                    for(int i = 0; i < 3; i++)	{
					    MiniGoal[6 + i] = c1[i];
                     }  
                  }
                  ok = true;
                }break;


                case COMMAND_VOCAB_OBST:{
                  rec = true;
                  Bottle* Coord = ObsReq.get(1).asList();
                  for(int i = 0; i < 3; i++)	{
					c2[i] = Coord->get(i).asDouble();
					cout << "OBST : Receiving trajectory point from the Observer client" <<  c2[i] << endl;
                  }
                ////////////////////setting via point  OBST coordinates correctly
                  if (BodyChain == 0) {
                    for(int i = 0; i < 3; i++)	{
					    MiniGoal[3+i] = c2[i];
                    }
                  }
				  if (BodyChain == 1) {
                    for(int i = 0; i < 3; i++)	{
					    MiniGoal[9 + i] = c2[i];
                     }
                  }
                  ok = true;
                }break;
                case COMMAND_VOCAB_REA :{
					InitializeJan();
					ObsReq.clear();
                  rec = true;
                  XmitGreen=0;
				                    

                  //placing the arm close to the destination...
                  if((GoalCodePMP == GoalCodePMP_DISC)||(GoalCodePMP == GoalCodePMP_CONT))	{
					cout << "REA : request to Reach Target received from Client" << endl;
					//InitializeJan(); //moved to threadInit
				    ResPM = VTGS(MiniGoal,GoalCodePMP,BodyChain,MSimExec,WristO,TrajType);
					cout << "Response from PMP:" << ResPM << endl;
                    readCurrentJoints();
					XmitGreen=0;
					if(ResPM==1)   {
                      cout << "Seems Doable::motor commands generated" << endl;
					//  cin >> XmitGreen;
					//  if(XmitGreen==1){
					  //MessagePassT();
					  //Time::delay(2);
                      //MessagePassR();
					  //}
                      XmitGreen=1;
                      ResPM=0;
					}
                  }
                  else if(GoalCodePMP==GoalCodePMP_INIT)	{

                   	ResPM=0;
                   	cout << "INIT : Request for Robot Initialization received from Client" << endl;
                   	initiCubUp(WristO);
                   	if (MSimExec == 1) {
                      if (cmdInterfacePort.getOutputCount()) {

                        //Bottle& outBot = cmdInterfacePort.prepare();
                        //cmdInterfacePort.setStrict();
                        //Send output commands in a bottle to motor interface
                        cmdInterfacePassT();       // torso
						Time::delay(2);
                        cmdInterfacePassR();       // right arm
						Time::delay(3);
						cmdInterfacePassL();       // left arm
						Time::delay(3);
                        //cmdInterfacePassRhand();   // right hand
                       // cmdInterfacePassL();       // left arm
                      //  cmdInterfacePassLhand();   // left hand
                      }
                      else {
                        //forcing to the rpc port of the controller
                        MessagePassT();
                        MessagePassR();
                        MessagePassL();
						  printf("WARNING: Sending motor commands with no PostureControl\n");
                      }
                    }
                  }
                  cout << "Sending out Result of requested Primitive behaviour to Client Observer" << endl;

                  ObsResp.addDouble(221);
                  ObsResp.addDouble(XmitGreen);
                  for(int i=0;i<3;i++)	{
                    if (BodyChain == 0)
                        ObsResp.addDouble(X_pos[i]);
                    if (BodyChain == 1)
                        ObsResp.addDouble(-X_posL[i]);
                  }
                  if (BodyChain == 0) {
                    ObsResp.addDouble(ang1);
                    ObsResp.addDouble(ang2);
                    ObsResp.addDouble(ang3); //torso
                    ObsResp.addDouble(ang4);
                    ObsResp.addDouble(ang5);
                    ObsResp.addDouble(ang6);
                    ObsResp.addDouble(ang7);
                    ObsResp.addDouble(ang8);
                    ObsResp.addDouble(ang9);
                    ObsResp.addDouble(ang10);// right arm
                  }
                  if (BodyChain == 1) {
                    ObsResp.addDouble(-ang1);
                    ObsResp.addDouble(-ang2);
                    ObsResp.addDouble(ang3); //torso
                    ObsResp.addDouble(ang4L);
                    ObsResp.addDouble(ang5L);
                    ObsResp.addDouble(ang6L);
                    ObsResp.addDouble(ang7L);
                    ObsResp.addDouble(ang8L);
                    ObsResp.addDouble(ang9L);
                    ObsResp.addDouble(ang10L);
                    //ObsResp.addDouble(angCupL);//left arm
                  }
                  ok = true;
                }break;

                default:{
                  printf("Command not recognized \n");
                }break;

                }


                if (!rec){
                  ObsResp.addVocab(COMMAND_VOCAB_FAILED);
                }


                if (!ok) {
                  ObsResp.clear();
                  ObsResp.addVocab(COMMAND_VOCAB_FAILED);
                }
                else {
                  ObsResp.addVocab(COMMAND_VOCAB_OK);
                }
            }
            else {
                cout<<"null request"<<endl;
            }


            PMPResponse.reply(ObsResp);
			ObsResp.clear();

        //========================================================================================
        }
    }

}


void PMPThread::threadRelease() {
  printf("PMPThread::threadRelease:beginning \n");
  //cmdRight_armPort.interrupt();
  //cmdRight_armPort.close();
  //cmdLeft_armPort.interrupt();
  //cmdLeft_armPort.close();
  //cmdTorsoPort.interrupt();
  //cmdTorsoPort.close();
  //cmdInterfacePort.interrupt();
  //cmdInterfacePort.close();
  //PMPResponse.interrupt();
  //PMPResponse.close();

  if (verboseTerm) {
    wr_Gam.close();
    posiL.close();
    wr.close();
    wr1.close();
    posi.close();
    wr1L.close();
    wr_GamL.close();
    fclose(jacFile);
    wr_Test.close();
  }
  printf("PMPThread::threadRelease:end \n");
}

void PMPThread::onStop() {
  printf("PMPThread::onStop:beginning \n");
  cmdLeft_armPort.interrupt();
  cmdRight_armPort.interrupt();
  cmdTorsoPort.interrupt();
  cmdInterfacePort.interrupt();
  PMPResponse.interrupt();
  activationsPort.interrupt();
  gazePort.interrupt();
  cmdLeft_armPort.close();
  cmdRight_armPort.close();
  cmdTorsoPort.close();
  cmdInterfacePort.close();
  PMPResponse.close();
  activationsPort.close();
  gazePort.close();
  printf("PMPThread::onStop:end \n");
}


/**
* function that computes the forwardKinematic using joint angles
* @param u joint angles
* @param l lenght of the vector
*/
double* PMPThread::forward_Kinematics(double *u , int l)	{

   	double *p,h1[hiddenL1],h2[hiddenL2];//,a[outputL];
   	//double a[3];
   	double T_Len=0; double T_Ori=0;


#ifdef NEURALNETWORK
    //double *p;
	//double h1[48],h2[55],a[3];
   	for(int v=0;v<hiddenL1;v++) {
		double sum = 0;
		for(int i=0;i<inputL;i++) {
      		sum = sum + w1[v][i]*u[i];
	    }
		h1[v] = sum + b1[v];
		h1[v] = tanh(h1[v]);
	}

	for(int v=0;v<hiddenL2;v++) {
		double sum = 0;
		for(int i=0;i<hiddenL1;i++) {
      		sum = sum + w2[v][i]*h1[i];
	    }
		h2[v] = sum + b2[v];
		h2[v] = tanh(h2[v]);
	}

	for(int v=0;v<outputL;v++) {
		double sum = 0;
		for(int i=0;i<hiddenL2;i++) {
      		sum = sum + w3[v][i]*h2[i];
	    }
		a[v] = sum + b3[v];
		//a[v] = tanh(a[v]);
	}

#else
	computeKinm(a,u);
    //printf("values from compute Jac and targets %f %f %f %f   %f  %f\n,a[0],a[1],a[2],u[0],[u1],[u2]");
#endif

   	p=a;
   	return p;
   	// When an array is passed to a function what is actually passed
   	// is its initial elements location in memory
};

double* PMPThread::forward_KinematicsL(double *uL , int lef)	{
    double *pLFK;
    //double aL[3];


	computeKinmL(aL,uL);
   	pLFK=aL;
   	return pLFK;
};

 double* PMPThread::forward_KinematicsLRH(double *uLRH , int lefRH)	{
    double *pLFKRH;
    //double aLRH[3];

    computeKinmLRH(aLRH,uLRH);
   	pLFKRH=aLRH;
   	return pLFKRH;
};

double* PMPThread::forcefield(double *w, double*v)	{
    int j;
    //double *ptr;
    double pos[3], tar[3];//,res[3];
    for(j=0;j<3;j++)	{
    	pos[j]=*(w+j);
    	tar[j]=*(v+j);
    	res[j]=KFORCE*(tar[j]-pos[j]);// Virtual stiffness is multiplied here //0.019 0.8
    }
    ptr=res;
    return ptr;
};

double* PMPThread::forcefieldL(double *wL, double*vL)	{
    int jLFK;
    double *ptrLFK;
    double posL[3], tarL[3];//resL[3];
    for(jLFK=0;jLFK<3;jLFK++)	{
    	posL[jLFK]=*(wL+jLFK);
    	tarL[jLFK]=*(vL+jLFK);
    	resL[jLFK]=KFORCE*(tarL[jLFK]-posL[jLFK]);// Virtual stiffness is multiplied here //0.019 0.8
    }
    ptrLFK=resL;
    return ptrLFK;
};







void PMPThread::computeNeuralJacobian(double* JacobIn, double* JanIn) {

}

 double* PMPThread::PMP(double *force,double *forceL)	{
//##############################################################################################


    int i,u;
    double pi=3.14;
    double ff[3],ffLFK[3];
    double Jacob[30],JacobL[30];
    double Joint_Field[10],Joint_FieldL[10];


    for(i=0;i<3;i++)
	{
	ff[i]=*(force+i);
   	}
    ff[0]=ff[0]*10;
	ff[1]=ff[1]*3;
	ff[2]=ff[2]*4;
   // cout<< ff[0] <<  ff[1] <<  ff[2] << endl;

	for(i=0;i<3;i++)
	{
	ffLFK[i]=*(forceL+i);
   	}
    ffLFK[0]=ffLFK[0]*10;
	ffLFK[1]=ffLFK[1]*3;
	ffLFK[2]=ffLFK[2]*4;



#ifdef NEURALNETWORK
//==========================ANN implementation =====//

	double h[48][1],z[48][1],p[55][1],hinter[48][1],p1[55][1],pinter[55][1],Jack[3][10],JacT[10][3];
//==================== Loading Weights, Biases for the ANN ====================

	for(u=0;u<hiddenL1;u++) {
		double sum=0;
		for(i=0;i<inputL;i++){
       		sum=sum+(w1[u][i]*Jan[i]);
	   	}
		h[u][0]=sum+b1[u]; // Inner variable of Layer 1
		z[u][0]=tanh(h[u][0]);// output of layer 1
	hinter[u][0]=1-(pow(z[u][0],2));    //(1-tanh(h(b,1))^2)
	}
//================W1/B1work over here, u have 'h' and 'z'===================

	for(u=0;u<hiddenL2;u++) {
		double sum2=0;
		for(i=0;i<hiddenL1;i++) {

       		sum2=sum2+w2[u][i]*z[i][0];
		}
		p[u][0]=sum2+b2[u]; // here u get p int variable of layer 2 ////////
		p1[u][0]=tanh(p[u][0]);// output of layer 1
		pinter[u][0]=1-(pow(p1[u][0],2));

	}

/////////////////////////////////////////////////////////Added SEND ACTIVATIONS To PORT FOR DISPLAY
/*	int iterVis= ITERATION;
	if((iterVis % 200)==0){
		if (activationsPort.getOutputCount()) {
			Bottle& actv =activationsPort.prepare();
    		actv.clear();
			for(int i=0; i<hiddenL1; i++)
				actv.addDouble(hinter[i][0]);
			for(int i=0; i<hiddenL2; i++)
				actv.addDouble(pinter[i][0]);
			//cout<<"Sending Activations for Display"<<endl;
       		activationsPort.write();
			//Time::delay(0.03);
		}
	}*/
////////////////////////////////////////////////////////////////////////////////////



	//int l=hiddenL2,j=hiddenL1,k,n,a,b;

	for(int k=0;k<outputL;k++) {
		for(int n=0;n<inputL;n++) {
			double inter1=0;
			for(int a=0;a<hiddenL2;a++) {
				for(int b=0;b<hiddenL1;b++) {
    				inter1=inter1 +((w3[k][a]*pinter[a][0])*((w2[a][b]*hinter[b][0])*w1[b][n]));
				}
			}
			Jack[k][n]=inter1;
			//fprintf(writeJ,"\n \t  %f",Jack[k][n]);
		}
	}

    //=======================================================================================
    meanJan[0] =  0.0000;
    meanJan[1] =  0.0000;
    meanJan[2] =  0.0000;
    meanJan[3] = -0.7854;
    meanJan[4] =  0.35;
    meanJan[5] =  0.7098;
    meanJan[6] =  0.9730;
    meanJan[7] =  1.600;
    meanJan[8] =  0.0000;
    meanJan[9] =  0.0000;


    Joint_Field[0]=(meanJan[0]-Jan[0]) * J0H; // Multiply by Joint compliance
    Joint_Field[1]=(meanJan[1]-Jan[1]) * J1H; //0.000041; //0.52 / Modified in June at Crete
    Joint_Field[2]=(meanJan[2]-Jan[2]) * J2H;     //1.8
    Joint_Field[3]=(meanJan[3]-Jan[3]) * J3H;   //4.5 //0.95
    Joint_Field[4]=(meanJan[4]-Jan[4]) * 400; //J3H
    Joint_Field[5]=(meanJan[5]-Jan[5]) * J5H;  //5; // Multiply by Joint compliance
    Joint_Field[6]=(meanJan[6]-Jan[6]) * J6H;  //0.041; //0.52 / Modified in June at Crete
    Joint_Field[7]=(meanJan[7]-Jan[7]) * 400;    //75;  //1.8
    Joint_Field[8]=(meanJan[8]-Jan[8]) * 400;     //4.5 //0.95
    Joint_Field[9]=(meanJan[9]-Jan[9]) * 400;

/*
    Joint_Field[0]=(0-Jan[0])*J0H; // Multiply by Joint compliance
    Joint_Field[1]=(0-Jan[1])*J1H;//0.000041; //0.52 / Modified in June at Crete
	Joint_Field[2]=(0.0-Jan[2])*J2H;  //1.8
	Joint_Field[3]=(-0.7854-Jan[3])*J3H;  //4.5 //0.95
	Joint_Field[4]=(0-Jan[4])*J4H;//J3H
	Joint_Field[5]=(0.7098-Jan[5])*J5H; //5; // Multiply by Joint compliance
    Joint_Field[6]=(0.9730-Jan[6])*J6H;//0.041; //0.52 / Modified in June at Crete
	Joint_Field[7]=(1.2-Jan[7])*J7H;//75;  //1.8
	Joint_Field[8]=(0-Jan[8])*J8H;  //4.5 //0.95
	Joint_Field[9]=(0-Jan[9])*J9H;
*/
//=======================================================================================
    for(int a=0;a<inputL;a++) {
		double jvelo=0;
    	for(int n=0;n<outputL;n++) {
	   		JacT[a][n]=Jack[n][a];
       		jvelo=jvelo+(JacT[a][n]*ff[n]);
		}
//    	Jvel[a]=0.002*(jvelo+Joint_Field[a]);
        if((a==0)){
           Jvel[a]=KOMP_WAISZT*(jvelo+Joint_Field[a]);
        }
		if((a==1)){
           Jvel[a]=0;//KOMP_WAISZT1*(jvelo+Joint_Field[a]);
        }
    	if(a==2){
           Jvel[a]=KOMP_WAISZT2*(jvelo+Joint_Field[a]);
        }
	    if(a>2){
           Jvel[a]=KOMP_JANG*(jvelo+Joint_Field[a]);
        }

   //cout << Jvel[a]<< endl;
    }



///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////
//Coding for left arm in symmetry

//==================== Loading Weights, Biases for the ANN ====================

	for(u=0;u<hiddenL1;u++) {
		double sum=0;
		for(i=0;i<inputL;i++){
       		sum=sum+(w1[u][i]*JanL[i]);
	   	}
		h[u][0]=sum+b1[u]; // Inner variable of Layer 1
		z[u][0]=tanh(h[u][0]);// output of layer 1
	hinter[u][0]=1-(pow(z[u][0],2));    //(1-tanh(h(b,1))^2)
	}
//================W1/B1work over here, u have 'h' and 'z'===================

	for(u=0;u<hiddenL2;u++) {
		double sum2=0;
		for(i=0;i<hiddenL1;i++) {

       		sum2=sum2+w2[u][i]*z[i][0];
		}
		p[u][0]=sum2+b2[u]; // here u get p int variable of layer 2 ////////
		p1[u][0]=tanh(p[u][0]);// output of layer 1
		pinter[u][0]=1-(pow(p1[u][0],2));

	}

/////////////////////////////////////////////////////////Added SEND ACTIVATIONS To PORT FOR DISPLAY
/*	iterVis= ITERATION;
	if((iterVis % 200)==0){
		if (activationsPort.getOutputCount()) {
			Bottle& actv =activationsPort.prepare();
    		actv.clear();
			for(int i=0; i<hiddenL1; i++)
				actv.addDouble(hinter[i][0]);
			for(int i=0; i<hiddenL2; i++)
				actv.addDouble(pinter[i][0]);
			//cout<<"Sending Activations for Display"<<endl;
       		activationsPort.write();
			//Time::delay(0.03);
		}
	} */
////////////////////////////////////////////////////////////////////////////////////



	//int l=hiddenL2,j=hiddenL1,k,n,a,b;

	for(int k=0;k<outputL;k++) {
		for(int n=0;n<inputL;n++) {
			double inter1=0;
			for(int a=0;a<hiddenL2;a++) {
				for(int b=0;b<hiddenL1;b++) {
    				inter1=inter1 +((w3[k][a]*pinter[a][0])*((w2[a][b]*hinter[b][0])*w1[b][n]));
				}
			}
			Jack[k][n]=inter1;
			//fprintf(writeJ,"\n \t  %f",Jack[k][n]);
		}
	}

    //=======================================================================================
    meanJan[0] =  0.0000;
    meanJan[1] =  0.0000;
    meanJan[2] =  0.0000;
    meanJan[3] = -0.7854;
    meanJan[4] =  0.35;
    meanJan[5] =  0.7098;
    meanJan[6] =  0.9730;
    meanJan[7] =  1.600;
    meanJan[8] =  0.0000;
    meanJan[9] =  0.0000;


    Joint_Field[0]=(meanJan[0]-JanL[0]) * J0H; // Multiply by Joint compliance
    Joint_Field[1]=(meanJan[1]-JanL[1]) * J1H; //0.000041; //0.52 / Modified in June at Crete
    Joint_Field[2]=(meanJan[2]-JanL[2]) * J2H;     //1.8
    Joint_Field[3]=(meanJan[3]-JanL[3]) * J3H;   //4.5 //0.95
    Joint_Field[4]=(meanJan[4]-JanL[4]) * 400; //J3H
    Joint_Field[5]=(meanJan[5]-JanL[5]) * J5H;  //5; // Multiply by Joint compliance
    Joint_Field[6]=(meanJan[6]-JanL[6]) * J6H;  //0.041; //0.52 / Modified in June at Crete
    Joint_Field[7]=(meanJan[7]-JanL[7]) * 400;    //75;  //1.8
    Joint_Field[8]=(meanJan[8]-JanL[8]) * 400;     //4.5 //0.95
    Joint_Field[9]=(meanJan[9]-JanL[9]) * 400;

/*
    Joint_Field[0]=(0-Jan[0])*J0H; // Multiply by Joint compliance

    Joint_Field[1]=(0-Jan[1])*J1H;//0.000041; //0.52 / Modified in June at Crete
	Joint_Field[2]=(0.0-Jan[2])*J2H;  //1.8
	Joint_Field[3]=(-0.7854-Jan[3])*J3H;  //4.5 //0.95
	Joint_Field[4]=(0-Jan[4])*J4H;//J3H
	Joint_Field[5]=(0.7098-Jan[5])*J5H; //5; // Multiply by Joint compliance
    Joint_Field[6]=(0.9730-Jan[6])*J6H;//0.041; //0.52 / Modified in June at Crete

	Joint_Field[7]=(1.2-Jan[7])*J7H;//75;  //1.8
	Joint_Field[8]=(0-Jan[8])*J8H;  //4.5 //0.95
	Joint_Field[9]=(0-Jan[9])*J9H;
*/
//=======================================================================================
	for(int a=0;a<inputL;a++) {
		double jvelo=0;
    	for(int n=0;n<outputL;n++) {
	   		JacT[a][n]=Jack[n][a];
       		jvelo=jvelo+(JacT[a][n]*ffLFK[n]);
		}
//    	Jvel[a+10]=0.002*(jvelo+Joint_Field[a]);

        if((a==0)){
           Jvel[a+10]=KOMP_WAISZT*(jvelo+Joint_Field[a]);
        }
		if((a==1)){
           Jvel[a+10]=0;//KOMP_WAISZT1*(jvelo+Joint_Field[a])
        }
    	if(a==2){
           Jvel[a+10]=KOMP_WAISZT2*(jvelo+Joint_Field[a]);
        }
	    if(a>2){
           Jvel[a+10]=KOMP_JANG*(jvelo+Joint_Field[a]);
        }
   //cout << Jvel[a]<< endl;
	}



#else
 	computeJacobian(Jacob,JacobL,Jan,JanL);


    for(int i = 0; i < 30 ; i++) {
        fprintf(jacFile,"%f ", Jacob[i]);
    }
    fprintf(jacFile,"\n\n\n");

//=======================================================================================


    meanJan[0] =  0.5200;
    meanJan[1] =  0.0000;
    meanJan[2] =  0.0000;
    meanJan[3] = -0.7854;
    meanJan[4] =  0.0000;

    meanJan[5] =  0.7098;
    meanJan[6] =  0.9730;
    meanJan[7] =  1.2000;
    meanJan[8] =  0.0000;

    meanJan[9] =  0.0000;

    JHd[0] = J0H;
    JHd[1] = 0.000041;

    JHd[2] = J2H;
    JHd[3] = 0.041;
    JHd[4] = 0.041;
    JHd[5] = 5;

    JHd[6] = 0.041;
    JHd[7] = 75;
    JHd[8] = J8H;
    JHd[9] = J9H;


    Joint_Field[0]=(meanJan[0]-Jan[0]) * JHd[0]; // Multiply by Joint compliance
    Joint_Field[1]=(meanJan[1]-Jan[1]) * JHd[1]; //0.52 / Modified in June at Crete
    Joint_Field[2]=(meanJan[2]-Jan[2]) * JHd[2];      //1.8

    Joint_Field[3]=(meanJan[3]-Jan[3]) * JHd[3];    //4.5 //0.95
    Joint_Field[4]=(meanJan[4]-Jan[4]) * JHd[4];
    Joint_Field[5]=(meanJan[5]-Jan[5]) * JHd[5];        // Multiply by Joint compliance
    Joint_Field[6]=(meanJan[6]-Jan[6]) * JHd[6];    //0.52 / Modified in June at Crete
    Joint_Field[7]=(meanJan[7]-Jan[7]) * JHd[7];       //1.8

    Joint_Field[8]=(meanJan[8]-Jan[8]) * JHd[8];      //4.5 //0.95
    Joint_Field[9]=(meanJan[9]-Jan[9]) * JHd[9];

//=======================================================================================

//=======================================================================================

    meanJanL[0] =  0.5200;
    meanJanL[1] =  0.0000;

    meanJanL[2] =  0.0000;
    meanJanL[3] = -0.7854;
    meanJanL[4] =  0.0000;
    meanJanL[5] =  0.7098;

    meanJanL[6] =  0.9730;
    meanJanL[7] =  1.2000;
    meanJanL[8] =  0.0000;
    meanJanL[9] =  0.0000;



    JHdL[0] = J0H;
    JHdL[1] = 0.000041;
    JHdL[2] = J2H;
    JHdL[3] = 0.041;
    JHdL[4] = 0.041;
    JHdL[5] = 5;
    JHdL[6] = 0.041;
    JHdL[7] = 75;
    JHdL[8] = J8H;
    JHdL[9] = J9H;

    Joint_FieldL[0]=(meanJanL[0]-JanL[0]) * JHdL[0]; // Multiply by Joint compliance

    Joint_FieldL[1]=(meanJanL[1]-JanL[1]) * JHdL[1]; //0.52 / Modified in June at Crete
    Joint_FieldL[2]=(meanJanL[2]-JanL[2]) * JHdL[2];  //1.8
    Joint_FieldL[3]=(meanJanL[3]-JanL[3]) * JHdL[3];  //4.5 //0.95
    Joint_FieldL[4]=(meanJanL[4]-JanL[4]) * JHdL[4];
    Joint_FieldL[5]=(meanJanL[5]-JanL[5]) * JHdL[5]; // Multiply by Joint compliance
    Joint_FieldL[6]=(meanJanL[6]-JanL[6]) * JHdL[6]; //0.52 / Modified in June at Crete
    Joint_FieldL[7]=(meanJanL[7]-JanL[7]) * JHdL[7];  //1.8
    Joint_FieldL[8]=(meanJanL[8]-JanL[8]) * JHdL[8];  //4.5 //0.95
    Joint_FieldL[9]=(meanJanL[9]-JanL[9]) * JHdL[9];

//=======================================================================================

   	//double arig=0.00061;
    Jvel[0]= 1*((ff[0]*Jacob[0])+(ff[1]*Jacob[10])+(ff[2]*Jacob[20])+Joint_Field[0]);
    Jvel[1]= 0*((ff[0]*Jacob[1])+(ff[1]*Jacob[11])+(ff[2]*Jacob[21])+Joint_Field[1]);
    Jvel[2]= 1*((ff[0]*Jacob[2])+(ff[1]*Jacob[12])+(ff[2]*Jacob[22])+Joint_Field[2]);
    Jvel[3]= 1*KOMP_JANG*((ff[0]*Jacob[3])+(ff[1]*Jacob[13])+(ff[2]*Jacob[23])+Joint_Field[3]);
    Jvel[4]= 1*KOMP_JANG*((ff[0]*Jacob[4])+(ff[1]*Jacob[14])+(ff[2]*Jacob[24])+Joint_Field[4]);
    Jvel[5]= 1*KOMP_JANG*((ff[0]*Jacob[5])+(ff[1]*Jacob[15])+(ff[2]*Jacob[25])+Joint_Field[5]);
    Jvel[6]= 1*KOMP_JANG*((ff[0]*Jacob[6])+(ff[1]*Jacob[16])+(ff[2]*Jacob[26])+Joint_Field[6]);
    Jvel[7]= 1*KOMP_JANG*((ff[0]*Jacob[7])+(ff[1]*Jacob[17])+(ff[2]*Jacob[27])+Joint_Field[7]);
    Jvel[8]= 1*KOMP_JANG*((ff[0]*Jacob[8])+(ff[1]*Jacob[18])+(ff[2]*Jacob[28])+Joint_Field[8]);
    Jvel[9]= 1*KOMP_JANG*((ff[0]*Jacob[9])+(ff[1]*Jacob[19])+(ff[2]*Jacob[29])+Joint_Field[9]);

    Jvel[10]= 1*((ffLFK[0]*JacobL[0])+(ffLFK[1]*JacobL[10])+(ffLFK[2]*JacobL[20])+Joint_FieldL[0]);
    Jvel[11]= 0*((ffLFK[0]*JacobL[1])+(ffLFK[1]*JacobL[11])+(ffLFK[2]*JacobL[21])+Joint_FieldL[1]);
    Jvel[12]= 1*((ffLFK[0]*JacobL[2])+(ffLFK[1]*JacobL[12])+(ffLFK[2]*JacobL[22])+Joint_FieldL[2]);
    Jvel[13]= KOMP_JANG*((ffLFK[0]*JacobL[3])+(ffLFK[1]*JacobL[13])+(ffLFK[2]*JacobL[23])+Joint_FieldL[3]);
    Jvel[14]= KOMP_JANG*((ffLFK[0]*JacobL[4])+(ffLFK[1]*JacobL[14])+(ffLFK[2]*JacobL[24])+Joint_FieldL[4]);
    Jvel[15]= KOMP_JANG*((ffLFK[0]*JacobL[5])+(ffLFK[1]*JacobL[15])+(ffLFK[2]*JacobL[25])+Joint_FieldL[5]);
    Jvel[16]= KOMP_JANG*((ffLFK[0]*JacobL[6])+(ffLFK[1]*JacobL[16])+(ffLFK[2]*JacobL[26])+Joint_FieldL[6]);
    Jvel[17]= KOMP_JANG*((ffLFK[0]*JacobL[7])+(ffLFK[1]*JacobL[17])+(ffLFK[2]*JacobL[27])+Joint_FieldL[7]);
    Jvel[18]= KOMP_JANG*((ffLFK[0]*JacobL[8])+(ffLFK[1]*JacobL[18])+(ffLFK[2]*JacobL[28])+Joint_FieldL[8]);
    Jvel[19]= KOMP_JANG*((ffLFK[0]*JacobL[9])+(ffLFK[1]*JacobL[19])+(ffLFK[2]*JacobL[29])+Joint_FieldL[9]);


    //double b_WAIST=0.00001; // one zero less //0.000003
    Jvel[0]=KOMP_WAISZT*(Jvel[0]+ Jvel[10]);
    Jvel[1]=KOMP_WAISZT*(Jvel[1]+ Jvel[11]);
    Jvel[2]=KOMP_WAISZT2*(Jvel[2]+ Jvel[12]);
#endif


//##############################################################################################
/*    int i;
    double pi=3.14;
    double ff[3],ffLFK[3],Jacob[30],JacobL[30],Joint_Field[10],Joint_FieldL[10];//,Jvel[20];
    double visco[5]={1,1,1,1,1};
    double *foof;

    for(i=0;i<3;i++)	{
    	ff[i]=*(force+i);
    }
    ff[0]=ff[0]*1;
    ff[1]=ff[1]*1;
    ff[2]=ff[2]*1;

    for(i=0;i<3;i++)	{
    	ffLFK[i]=*(forceL+i);
    }
    ffLFK[0]=ffLFK[0]*1;
    ffLFK[1]=ffLFK[1]*1;
    ffLFK[2]=ffLFK[2]*1;


//=======================================================================================
// Jacobian for the arms and torso

#ifdef NEURALNETWORK
    computeNeuralJacobian(Jacob, Jan);
#else
 	computeJacobian(Jacob,JacobL,Jan,JanL);
#endif


    for(int i = 0; i < 30 ; i++) {
        fprintf(jacFile,"%f ", Jacob[i]);
    }
    fprintf(jacFile,"\n\n\n");

//=======================================================================================

    meanJan[0] =  0.5200;
    meanJan[1] =  0.0000;
    meanJan[2] =  0.0000;
    meanJan[3] = -0.7854;
    meanJan[4] =  0.0000;
    meanJan[5] =  0.7098;
    meanJan[6] =  0.9730;
    meanJan[7] =  1.2000;
    meanJan[8] =  0.0000;
    meanJan[9] =  0.0000;

    JHd[0] = J0H;
    JHd[1] = 0.000041;
    JHd[2] = J2H;
    JHd[3] = 0.041;
    JHd[4] = 0.041;
    JHd[5] = 5;
    JHd[6] = 0.041;
    JHd[7] = 75;
    JHd[8] = J8H;
    JHd[9] = J9H;

    Joint_Field[0]=(meanJan[0]-Jan[0]) * JHd[0]; // Multiply by Joint compliance
    Joint_Field[1]=(meanJan[1]-Jan[1]) * JHd[1]; //0.52 / Modified in June at Crete
    Joint_Field[2]=(meanJan[2]-Jan[2]) * JHd[2];      //1.8
    Joint_Field[3]=(meanJan[3]-Jan[3]) * JHd[3];    //4.5 //0.95
    Joint_Field[4]=(meanJan[4]-Jan[4]) * JHd[4];
    Joint_Field[5]=(meanJan[5]-Jan[5]) * JHd[5];        // Multiply by Joint compliance
    Joint_Field[6]=(meanJan[6]-Jan[6]) * JHd[6];    //0.52 / Modified in June at Crete
    Joint_Field[7]=(meanJan[7]-Jan[7]) * JHd[7];       //1.8
    Joint_Field[8]=(meanJan[8]-Jan[8]) * JHd[8];      //4.5 //0.95
    Joint_Field[9]=(meanJan[9]-Jan[9]) * JHd[9];

//=======================================================================================
//=======================================================================================

    meanJanL[0] =  0.5200;
    meanJanL[1] =  0.0000;
    meanJanL[2] =  0.0000;
    meanJanL[3] = -0.7854;
    meanJanL[4] =  0.0000;
    meanJanL[5] =  0.7098;
    meanJanL[6] =  0.9730;
    meanJanL[7] =  1.2000;
    meanJanL[8] =  0.0000;
    meanJanL[9] =  0.0000;

    JHdL[0] = J0H;
    JHdL[1] = 0.000041;
    JHdL[2] = J2H;
    JHdL[3] = 0.041;
    JHdL[4] = 0.041;
    JHdL[5] = 5;
    JHdL[6] = 0.041;
    JHdL[7] = 75;
    JHdL[8] = J8H;
    JHdL[9] = J9H;

    Joint_FieldL[0]=(meanJanL[0]-JanL[0]) * JHdL[0]; // Multiply by Joint compliance
    Joint_FieldL[1]=(meanJanL[1]-JanL[1]) * JHdL[1]; //0.52 / Modified in June at Crete
    Joint_FieldL[2]=(meanJanL[2]-JanL[2]) * JHdL[2];  //1.8
    Joint_FieldL[3]=(meanJanL[3]-JanL[3]) * JHdL[3];  //4.5 //0.95
    Joint_FieldL[4]=(meanJanL[4]-JanL[4]) * JHdL[4];
    Joint_FieldL[5]=(meanJanL[5]-JanL[5]) * JHdL[5]; // Multiply by Joint compliance
    Joint_FieldL[6]=(meanJanL[6]-JanL[6]) * JHdL[6]; //0.52 / Modified in June at Crete
    Joint_FieldL[7]=(meanJanL[7]-JanL[7]) * JHdL[7];  //1.8
    Joint_FieldL[8]=(meanJanL[8]-JanL[8]) * JHdL[8];  //4.5 //0.95
    Joint_FieldL[9]=(meanJanL[9]-JanL[9]) * JHdL[9];

//=======================================================================================

   	//double arig=0.00061;
    Jvel[0]= 1*((ff[0]*Jacob[0])+(ff[1]*Jacob[10])+(ff[2]*Jacob[20])+Joint_Field[0]);
    Jvel[1]= 0*((ff[0]*Jacob[1])+(ff[1]*Jacob[11])+(ff[2]*Jacob[21])+Joint_Field[1]);
    Jvel[2]= 1*((ff[0]*Jacob[2])+(ff[1]*Jacob[12])+(ff[2]*Jacob[22])+Joint_Field[2]);
    Jvel[3]= 1*KOMP_JANG*((ff[0]*Jacob[3])+(ff[1]*Jacob[13])+(ff[2]*Jacob[23])+Joint_Field[3]);
    Jvel[4]= 1*KOMP_JANG*((ff[0]*Jacob[4])+(ff[1]*Jacob[14])+(ff[2]*Jacob[24])+Joint_Field[4]);
    Jvel[5]= 1*KOMP_JANG*((ff[0]*Jacob[5])+(ff[1]*Jacob[15])+(ff[2]*Jacob[25])+Joint_Field[5]);
    Jvel[6]= 1*KOMP_JANG*((ff[0]*Jacob[6])+(ff[1]*Jacob[16])+(ff[2]*Jacob[26])+Joint_Field[6]);
    Jvel[7]= 1*KOMP_JANG*((ff[0]*Jacob[7])+(ff[1]*Jacob[17])+(ff[2]*Jacob[27])+Joint_Field[7]);
    Jvel[8]= 1*KOMP_JANG*((ff[0]*Jacob[8])+(ff[1]*Jacob[18])+(ff[2]*Jacob[28])+Joint_Field[8]);
    Jvel[9]= 1*KOMP_JANG*((ff[0]*Jacob[9])+(ff[1]*Jacob[19])+(ff[2]*Jacob[29])+Joint_Field[9]);

    Jvel[10]= 1*((ffLFK[0]*JacobL[0])+(ffLFK[1]*JacobL[10])+(ffLFK[2]*JacobL[20])+Joint_FieldL[0]);
    Jvel[11]= 0*((ffLFK[0]*JacobL[1])+(ffLFK[1]*JacobL[11])+(ffLFK[2]*JacobL[21])+Joint_FieldL[1]);
    Jvel[12]= 1*((ffLFK[0]*JacobL[2])+(ffLFK[1]*JacobL[12])+(ffLFK[2]*JacobL[22])+Joint_FieldL[2]);
    Jvel[13]= KOMP_JANG*((ffLFK[0]*JacobL[3])+(ffLFK[1]*JacobL[13])+(ffLFK[2]*JacobL[23])+Joint_FieldL[3]);
    Jvel[14]= KOMP_JANG*((ffLFK[0]*JacobL[4])+(ffLFK[1]*JacobL[14])+(ffLFK[2]*JacobL[24])+Joint_FieldL[4]);
    Jvel[15]= KOMP_JANG*((ffLFK[0]*JacobL[5])+(ffLFK[1]*JacobL[15])+(ffLFK[2]*JacobL[25])+Joint_FieldL[5]);
    Jvel[16]= KOMP_JANG*((ffLFK[0]*JacobL[6])+(ffLFK[1]*JacobL[16])+(ffLFK[2]*JacobL[26])+Joint_FieldL[6]);
    Jvel[17]= KOMP_JANG*((ffLFK[0]*JacobL[7])+(ffLFK[1]*JacobL[17])+(ffLFK[2]*JacobL[27])+Joint_FieldL[7]);
    Jvel[18]= KOMP_JANG*((ffLFK[0]*JacobL[8])+(ffLFK[1]*JacobL[18])+(ffLFK[2]*JacobL[28])+Joint_FieldL[8]);
    Jvel[19]= KOMP_JANG*((ffLFK[0]*JacobL[9])+(ffLFK[1]*JacobL[19])+(ffLFK[2]*JacobL[29])+Joint_FieldL[9]);

    //double b_WAIST=0.00001; // one zero less //0.000003
    Jvel[0]=KOMP_WAISZT*(Jvel[0]+ Jvel[10]);
    Jvel[1]=KOMP_WAISZT*(Jvel[1]+ Jvel[11]);
    Jvel[2]=KOMP_WAISZT2*(Jvel[2]+ Jvel[12]);
 */
    foof=Jvel;
    return foof;
};




double PMPThread::Gamma(int _Time)	{
    double t_ramp=(_Time)*0.0015; //0.0025 to
    double t_init=0.1,t_dur=3,z,t_win,t_window,csi,csi_dot,prod1,prod2,Gamma;//3

    z=(t_ramp-t_init)/t_dur;
    t_win=(t_init+t_dur)-t_ramp;
    if (t_win>0) {
        t_window=1;
    }
    else  {
    	t_window=0;
    }
    csi=(6*pow(z,5))-(15*pow(z,4))+(10*pow(z,3));  //6z^5-15z^4+10z^3
    csi_dot=(30*pow(z,4))-(60*pow(z,3))+(30*pow(z,2)); //csi_dot=30z^4-60z^3+30z^2
    //fprintf(wrL,"\n  %f     %f \t  %f",csi,csi_dot);
    prod1=(1/(1.0001-(csi*t_window)));
    prod2=(csi_dot*0.3333*t_window);
    Gamma=prod1*prod2;
    return Gamma;
};


double PMPThread::Gamma1(int _Time1)	{

    double t_ramp1=(_Time1)*0.001;
    double t_init1=0,t_dur1=2,z1,t_win1,t_window1,csi1,prod11,prod21,Gamma1;

    z1=(t_ramp1-t_init1)/t_dur1;
    t_win1=(t_init1+t_dur1)-t_ramp1;
    if(t_win1>0)	{
    	t_window1=1;
   	}
    else	{
    	t_window1=0;
    }
    csi1=(6*pow(z1,5))-(15*pow(z1,4))+(10*pow(z1,3));  //6z^5-15z^4+10z^3
    csi_dot1=(30*pow(z1,4))-(60*pow(z1,3))+(30*pow(z1,2)); //csi_dot=30z^4-60z^3+30z^2
    prod11=(1/(1.0001-(csi1*t_window1)));
    prod21=(csi_dot1*0.3333*t_window1);
    Gamma1=(prod11*prod21);
    return Gamma1;
};

double PMPThread::Gamma_Int(double *ptr,int n)	{

    int k=1;        /* Counters in the algorithm */
    double a=0;
    double h,sum,fk;

 	// Simpsons 1/3 rule for Integration

    sum=*(ptr);             /* Initial function value */
    int c=2;
    h=1;                	/*Step Size*/
    while (k <= n-1)  { 	/* Steps through the iteration */
    	fk=*(ptr+k);
    	c=6-c;       		/* gives the 4,2,4,2,... */
        sum = (sum + c*fk);  /* Adds on the next area */
        k++;         		/* Increases k value by +1 */
    }
    sum=RAMP_KONSTANT*sum/3; 		// changed 0.0025 to

    return sum;

};

double PMPThread::Gamma_IntDisc(double *Gar,int n)	{

    int k=1;        /* Counters in the algorithm */
    double a=0;
    double h,sum,fk;

 	// Simpsons 1/3 rule for Integration

  	sum=*(Gar);             /* Initial function value */
    int c=2;
    h=1;                	/*Step Size*/
    while (k <= n-1)  { 	/* Steps through the iteration */

        fk=*(Gar+k);
        c=6-c;       		/* gives the 4,2,4,2,... */
        sum = (sum + c*fk); /* Adds on the next area */
        k++;         		/* Increases k value by +1 */
    }
    sum=RAMP_KONSTANT*sum/3; // changed 0.0025 to

    return sum;
};

double PMPThread::GammaDisc(int _Time)	{
    double t_ramp=(_Time)*RAMP_KONSTANT; //0.0025 to
    double t_init=0.1,z,t_win,t_window,csi,prod1,prod2,Gamma;

    z=(t_ramp-t_init)/t_dur;
    t_win=(t_init+t_dur)-t_ramp;
    if(t_win>0)	{
        t_window=1;
    }
    else	{
    	t_window=0;
    }
    csi=(6*pow(z,5))-(15*pow(z,4))+(10*pow(z,3));  //6z^5-15z^4+10z^3
    csi_dot=(30*pow(z,4))-(60*pow(z,3))+(30*pow(z,2)); //csi_dot=30z^4-60z^3+30z^2
    //fprintf(wrL,"\n  %f     %f \t  %f",csi,csi_dot);
    prod1=(1/(1.0001-(csi*t_window)));
    prod2=(csi_dot*0.3333*t_window);
    Gamma=prod1*prod2;
    return Gamma;

};

void PMPThread::InitializeJanObst() {	// this is the default init for a trajectory synth pose

    Jan[0]=0;
    Jan[1]=0;
    Jan[2]=0;
    Jan[3]=-0.64;// -0.6981
    Jan[4]=0.48;
    Jan[5]=0.27;
    Jan[6]=0.95;
    Jan[7]=0;
    Jan[8]=0;
    Jan[9]=0;

    janini0=0;
	janini1=0;
    janini2=0;
    janini3=-0.64;// -0.6981
    janini4=0.48;
    janini5=0.27;
    janini6=0.95;
    janini7=0;
    janini8=0;
    janini9=0;
    JanL[0]=0;
    JanL[1]=0;
    JanL[2]=0;
    JanL[3]=-1.74;// -0.6981
    JanL[4]=0.78;
    JanL[5]=0;
    JanL[6]=1.3;
    JanL[7]=0;
    JanL[8]=0;
    JanL[9]=0;

    janini3L=-1.74;// -0.6981
    janini4L=0.78;
    janini5L=0;
    janini6L=1.30;
    janini7L=0;
    janini8L=0;
    janini9L=0;

    x_iniIC=-170;
    y_iniIC=-300;
    z_iniIC=50;
    x_iniICL=256;
    y_iniICL=-160;
    z_iniICL=380;

    x_ini=-170;
    y_ini=-300;
    z_ini=50;
    x_iniL=256;
    y_iniL=-160;
    z_iniL=380;
    };

void PMPThread::InitializeJan()	 {	// init for normal reaching

/*
    Jan[0]=0;
    Jan[1]=0;
    Jan[2]=0;
    Jan[3]=-1.74;// -0.6981
    Jan[4]=0.78;
    Jan[5]=0;
    Jan[6]=1.3;
    Jan[7]=0;
    Jan[8]=0;
    Jan[9]=0;

    janini0=0;
    janini1 //missing because we dont use one of the dof of the torso
    janini2=0;
    janini3=-1.74;// -0.6981
    janini4=0.78;
    janini5=0;
    janini6=1.3;
    janini7=0;
    janini8=0;
    janini9=0;

    JanL[0]=0;
    JanL[1]=0;
    JanL[2]=0;
    JanL[3]=-1.74;// -0.6981
    JanL[4]=0.78;
    JanL[5]=0;
    JanL[6]=1.3;
    JanL[7]=0;
    JanL[8]=0;
    JanL[9]=0;

    janini3L=-1.74;// -0.6981
    janini4L=0.78;
    janini5L=0;
    janini6L=1.30;
    janini7L=0;
    janini8L=0;
    janini9L=0;

    x_iniIC  =-215;
	y_iniIC  = 245;
	z_iniIC  = 402;
	x_iniICL =-160;
	y_iniICL =-256;
	z_iniICL = 380;

	x_ini  =-215;
	y_ini  = 245;
	z_ini  = 402;
	x_iniL =-160;
	y_iniL =-256;
	z_iniL = 380;
*/
    Jan[0]=0;
    Jan[1]=0;
    Jan[2]=0.01;
    Jan[3]=-0.73;// -0.6981
    Jan[4]=0.27;
    Jan[5]=0.37;
    Jan[6]=1.03;
    Jan[7]=0;
    Jan[8]=0.37;
    Jan[9]=0.52;

    janini0=0;
    janini1=0;  //missing because we dont use one of the dof of the torso
    janini2=-0.01;
    janini3=-0.73;// -0.6981
    janini4=0.27;
    janini5=0.37;
    janini6=1.03;
    janini7=0;
    janini8=0.37;
    janini9=0.52;

    JanL[0]=0;
    JanL[1]=0;
    JanL[2]=0.01;
    JanL[3]=-0.73;// -0.6981
    JanL[4]=0.27;
    JanL[5]=0.37;
    JanL[6]=1.03;
    JanL[7]=0;
    JanL[8]=0.37;
    JanL[9]=0.52;

    janini3L=-0.73;// -0.6981
    janini4L=0.27;
    janini5L=0.37;
    janini6L=1.03;
    janini7L=0;
    janini8L=0.37;
    janini9L=0.52;

    x_iniIC  =-303;
	y_iniIC  = 9;
	z_iniIC  = 23;
	x_iniICL =-303;
	y_iniICL = -9;
	z_iniICL = 23;

	x_ini  =-303;
	y_ini  = 9;
	z_ini  = 23;
	x_iniL =-303;
	y_iniL =-9;
	z_iniL = 23;


};

void PMPThread::readCurrentJoints(){

    if(!(Network::isConnected("/icub/torso/state:o","/PMP/torsoJoints:i"))){
	    Network::connect("/icub/torso/state:o", "/PMP/torsoJoints:i");
    }
    if(!(Network::isConnected("/icub/right_arm/state:o","/PMP/rightArmJoints:i"))){
	    Network::connect("/icub/right_arm/state:o", "/PMP/rightArmJoints:i");
    }
    if(!(Network::isConnected("/icub/left_arm/state:o","/PMP/leftArmJoints:i"))){
	    Network::connect("/icub/left_arm/state:o", "/PMP/leftArmJoints:i");
    }
    double jointsCurr[10];
    if (torso_joints_Current.getInputCount())
	{
		Bottle* torsoJoints = torso_joints_Current.read(true);
        for (int i=0;i<3;i++){
		     jointsCurr[i]= torsoJoints->get(i).asDouble();
             cout <<" "<<jointsCurr[i];
             jointsCurr[i]=3.14159*(jointsCurr[i]/180);
        }
    }

    if (right_arm_joints_Current.getInputCount())
	{
		Bottle* rightJoints = right_arm_joints_Current.read(true);
        for (int i=0;i<7;i++){
		     jointsCurr[i+3]= rightJoints->get(i).asDouble();
             cout <<" "<<jointsCurr[i+3];
             jointsCurr[i+3]=3.14159*(jointsCurr[i+3]/180);
        }
    }

    double *nFK = forward_Kinematics(jointsCurr,10); // Joint Angles to Positions 3>>>>>2
    cout <<"Forward model "<<endl;
	cout <<"X coordinate is "<<*(nFK)<<"Y is "<<*(nFK+1)<<"Z is "<<*(nFK+2)<<endl;
        
}

void PMPThread::PandP() {	//is no longer used but is an old example of simple  pick and place with grasp and release functioanlity

    Kompliance(1);
    int recOb1;

    if(PickX<=0)	{

        MiniGoal[0]=PickX; MiniGoal[1]=PickY; MiniGoal[2]=PickZ; MiniGoal[6]=0; MiniGoal[7]=0; MiniGoal[8]=0;
        recOb1=VTGS(MiniGoal,0,0,1,0,0);
        //Sleep(5000);
        GraspR();
        //Sleep(10000);
        initiCubUp(76);
        MessagePassR();
        //Sleep(5000);
        MiniGoal[0]=PlacX; MiniGoal[1]=PlacY; MiniGoal[2]=PlacZ; MiniGoal[6]=0; MiniGoal[7]=0; MiniGoal[8]=0;
        recOb1=VTGS(MiniGoal,0,0,1,0,0);
        //Sleep(5000);
        CubReleaseSoft();
        MessagePassR();
        //Sleep(10000);
        CubRelease();
        MessagePassR();
        //Sleep(5000);
        initiCubUp(76);
        MessagePassR();
        //Sleep(5000);
        //Sleep(50000); //optional
    }

    if(PickX>0)	{

        MiniGoal[0]=0; MiniGoal[1]=0; MiniGoal[2]=0; MiniGoal[6]=PickX; MiniGoal[7]=PickY; MiniGoal[8]=PickZ;
        recOb1=VTGS(MiniGoal,0,1,1,0,0);
        //Sleep(5000);
        GraspL();
        //Sleep(10000);
        initiCubUp(76);
        MessagePassL();
        //Sleep(5000);
        MiniGoal[0]=0; MiniGoal[1]=0; MiniGoal[2]=0; MiniGoal[6]=PlacX; MiniGoal[7]=PlacY; MiniGoal[8]=PlacZ;
        recOb1=VTGS(MiniGoal,0,1,1,0,0);
        //Sleep(5000);
        CubReleaseSoft();
        MessagePassL();
        //Sleep(10000);
        CubRelease();
        MessagePassL();
        //Sleep(5000);
        initiCubUp(76);
        MessagePassL();
        //Sleep(5000);
        //Sleep(10000); //optional
   	}
};

void PMPThread::Reason(int typeGoal)	{
   	/*  CubRelease();
    InitializeJan();
    initiCubUp();
    MessagePassR();
    Sleep(2000);
    MessagePassL();
    Sleep(2000);
    MessagePassT();
    Sleep(2000);*/
    //=============================== This should come from random choice and placemap ==============================
    PickX=200;
    PickY=-410;
    PickZ=-30;
    PlacX= 20;
    PlacY=-410;
    PlacZ=40;
    InitializeJanObst();
    Kompliance(0);
    //MiniGoal[0]=-170; MiniGoal[1]=-300; MiniGoal[2]=50; MiniGoal[6]=0; MiniGoal[7]=0; MiniGoal[8]=0;
    //int recOb1=VTGS(MiniGoal,0,0,0,0);

    MiniGoal[0]=-100; MiniGoal[1]=-250; MiniGoal[2]=50; MiniGoal[3]=0; MiniGoal[4]=-300; MiniGoal[5]=50;
	//int recOb1=VTGS(MiniGoal,1,0,0,0);

	//	 PandP(); if you want to do stacking..


    /* initiCubUp();
    MessagePassR();
    Sleep(2000);
    MessagePassL();
    Sleep(2000);
    MessagePassT();
    Sleep(2000);*/

};

int PMPThread::FrameGoal()	{
    // at present only reach
    int i, OiDD, Ofound=0;
    cout << " Which object to act on  " << endl;
    cin >> OiDD;
    for(i=0;i<NoBjS;i++)	{
    	if(PlaceMap[i][4]==OiDD)	{
        	Ofound=i;
            cout << " Object Found  " << endl;
        }
    }
    return Ofound;

};

int PMPThread::VTGS(double *MiniGoal, int ChoiceAct, int HandAct,int MSim, double Wrist, int TrajT)	{

    int time;
    double Gam;
    double fin[3];
    double finL[3];
    int n=3;
    int retvalue=0;
    //###########################################################################################################################//
    // ****************************************** DISCRETE ACTION *************************************************************//
    //################################################################

	if(ChoiceAct==GoalCodePMP_DISC)	{
        Kompliance(1);
        if((MiniGoal[0]==0)&&(MiniGoal[1]==0)&&(MiniGoal[2]==0)) {	//note this is for iCub , for icubSim we need to divide by 1000

            MiniGoal[0]=-160;  // corrected swap x and y
            MiniGoal[1]= 263;  // corrected swap x and y
            MiniGoal[2]= 378;
        }

        if((MiniGoal[6]==0)&&(MiniGoal[7]==0)&&(MiniGoal[8]==0))	{

            MiniGoal[6]=-160; // corrected swap x and y
            MiniGoal[7]=-263; // corrected swap x and y
            MiniGoal[8]= 378;
        }

        fin[0]=MiniGoal[0]; //Final Position X
        fin[1]=MiniGoal[1];
        fin[2]=MiniGoal[2];

        finL[0]=MiniGoal[6]; //Final Position X
        finL[1]=-MiniGoal[7];/////////////////////reflected along y axis for symmetry exploitation
        finL[2]=MiniGoal[8];// TO TAKE INTO ACCOUNT SHIFT IN ORIGIN OF THE Z AXIS


        double xoffs=0,yoffs=0,zoffs=0;
        int replan=0;
        double xoffsL=0,yoffsL=0,zoffsL=0;


        x_fin=fin[0]+xoffs; //Final Position X
        y_fin=fin[1]+yoffs;
        z_fin=fin[2]+zoffs;

        x_finL=finL[0]+xoffsL; //Final Position X
        y_finL=finL[1]+yoffsL;
        z_finL=finL[2]+zoffsL;

        printf("\n Targets");
        printf("\n \n %f, \t  %f, \t %f ",x_fin,y_fin,z_fin);
        printf("\n \n %f, \t  %f, \t %f ",x_finL,-(y_finL),z_finL);

        janini0=Jan[0];
		janini1=Jan[1];
        janini2=Jan[2];
        janini3=Jan[3];//-0.9425
        janini4=Jan[4];
        janini5=Jan[5];
        janini6=Jan[6];
        janini7=Jan[7];
        janini8=Jan[8];
        janini9=Jan[9];

        janini3L=JanL[3];//-0.9425
        janini4L=JanL[4];
        janini5L=JanL[5];
        janini6L=JanL[6];
        janini7L=JanL[7];
        janini8L=JanL[8];
        janini9L=JanL[9];

        x_iniIC=x_ini;
        y_iniIC=y_ini;
        z_iniIC=z_ini;
        x_iniICL=x_iniL;
        y_iniICL=y_iniL;
        z_iniICL=z_iniL;

        for(time=0;time<ITERATION;time++) {	// 2000 incremental steps of delta 0.005

        	Gam=GammaDisc(time);
            wrL << time << "    " << Gam << endl;

            //  ====================Target Generation //=========================
            if((HandAct==BodyTorsoArm_RIGHT)||(HandAct==BodyTorsoArm_BOTH))	{

                double inter_x=(x_fin-x_ini)*Gam;
                Gam_Arr[time]=inter_x;
                double *Gar=Gam_Arr;
                x_ini=Gamma_Int(Gar,time)+x_iniIC;

                double inter_y=(y_fin-y_ini)*Gam;
                Gam_Arry[time]=inter_y;
                double *Gary=Gam_Arry;
                y_ini=Gamma_Int(Gary,time)+y_iniIC;

                double inter_z=(z_fin-z_ini)*Gam;
                Gam_Arrz[time]=inter_z;
                double *Garz=Gam_Arrz;
                z_ini=Gamma_Int(Garz,time)+z_iniIC;
            }
     		//  ====================Target Generation Left //=========================
            if((HandAct==BodyTorsoArm_LEFT)||(HandAct==BodyTorsoArm_BOTH))	{

                double inter_xL=(x_finL-x_iniL)*Gam;
                Gam_ArrL[time]=inter_xL;
                double *GarL=Gam_ArrL;
                x_iniL=Gamma_Int(GarL,time)+x_iniICL;

                double inter_yL=(y_finL-y_iniL)*Gam;
                Gam_ArryL[time]=inter_yL;
                double *GaryL=Gam_ArryL;
                y_iniL=Gamma_Int(GaryL,time)+y_iniICL;

                double inter_zL=(z_finL-z_iniL)*Gam;
                Gam_ArrzL[time]=inter_zL;
                double *GarzL=Gam_ArrzL;
                z_iniL=Gamma_Int(GarzL,time)+z_iniICL;
            }
            if (verboseTerm) {
    			wr1L << x_ini << "    " << y_ini << "    " << z_ini << "    "  << x_iniL << "    " << y_iniL << "    " << z_iniL <<endl;
    		}
 //***********************************************************************************************************//
            // Running the motor control
    		MotCon(x_ini,y_ini,z_ini, x_iniL,y_iniL,z_iniL,time,Gam,HandAct);
 //**********************************************************************************************
    		if (verboseTerm) {

    			posiL << X_pos[0] << "    " << X_pos[1] << "    " << X_pos[2] << "    "  << X_posL[0] << "    " << -(X_posL[1]) << "    " << X_posL[2] <<endl;
    			wr_Gam << Jan[0] << "  " << Jan[1]<< "  " << Jan[2]<< "  " << Jan[3]<< "  " << Jan[4]<< "  " << Jan[5]<< "  " << Jan[6]<< "  " << Jan[7]<< "  " << Jan[8]<< "  " << Jan[9]<< "  " << JanL[3]<< "  " << JanL[4]<< "  " << JanL[5]<< "  " << JanL[6]<< "  " << JanL[7]<< "  " << JanL[8]<< "  " << JanL[9] <<endl;
    		}

	/*		if (cmdEndEffectorPort.getOutputCount() >0) {
				Bottle & PlotPoints = cmdEndEffectorPort.prepare();
				PlotPoints.clear();
				PlotPoints.addDouble(Gam);
				if (HandAct==0){
					PlotPoints.addDouble(x_ini);
					PlotPoints.addDouble(y_ini);
					PlotPoints.addDouble(z_ini);
					PlotPoints.addDouble(sqrt(pow(X_pos[0]-x_ini,2)+pow(X_pos[1]-y_ini,2)+pow(X_pos[2]-z_ini,2)));
				}
				if (HandAct==1){
					PlotPoints.addDouble(x_iniL);
					PlotPoints.addDouble(y_iniL);
					PlotPoints.addDouble(z_iniL);
					PlotPoints.addDouble(sqrt(pow(X_posL[0]-x_iniL,2)+pow(X_posL[1]-y_iniL,2)+pow(X_posL[2]-z_iniL,2)));
				}
				cmdEndEffectorPort.write(true);
				Time::delay(0.02);
			} */
    	}

    	ang1=rad2degree*Jan[0];
    	ang2=rad2degree*Jan[1];
    	ang3=rad2degree*Jan[2];
    	ang4=rad2degree*Jan[3];
    	ang5=rad2degree*Jan[4];
    	ang6=rad2degree*Jan[5];
    	ang7=rad2degree*Jan[6];
    	ang8=Wrist; //this must be wrist
    	ang9=rad2degree*Jan[8];
    	ang10=Wrist2;//rad2degree*Jan[9];
    	angCup=51;
    	ang4L=rad2degree*JanL[3];
    	ang5L=rad2degree*JanL[4];
    	ang6L=rad2degree*JanL[5];
    	ang7L=rad2degree*JanL[6];
    	ang8L=Wrist; //this must be wrist
    	ang9L=rad2degree*JanL[8];
    	ang10L=Wrist2;//rad2degree*JanL[9];//
    	angCupL=48;

        printf("\n JOINT ANGLE: RIGHT and LEFT \n");
    	printf("\n  %f, \t  %f, \t %f ,\t %f ,\t %f ,\t %f, \t  %f, \t %f ,\t %f ,\t %f",ang1,ang2,ang3,ang4,ang5, ang6,ang7,ang8,ang9,ang10);
    	printf("\n  %f, \t  %f, \t %f ,\t %f ,\t %f ,\t %f, \t  %f, \t %f ,\t %f ,\t %f",-ang1,-ang2,ang3,ang4L,ang5L,ang6L,ang7L,ang8L,ang9L,ang10L);
    	printf("\n\n FINAL SOLUTION  %f, \t  %f, \t %f \t %f, \t  %f, \t %f ",X_pos[0],X_pos[1],X_pos[2],X_posL[0],-(X_posL[1]),X_posL[2]);
    	//Sleep(5000);
//temorary line
        wr_Test << MiniGoal[0] << "    " << MiniGoal[1] << "    " << MiniGoal[2] << "    " << X_pos[0] << "    " << X_pos[1] << "    " << X_pos[2] <<endl;

    	if(MSim==MSim_SIMULATION)	{
        	//  InitializeJan();
            if((HandAct==BodyTorsoArm_RIGHT) || (HandAct==BodyTorsoArm_BOTH))	{

                if(((ang4>-99)&&(ang4<-15))&&((ang5>5)&&(ang5<100)))	{

                	if((sqrt(pow(X_pos[0]-fin[0],2)+ pow(X_pos[1]-fin[1],2)+ pow(X_pos[2]-fin[2],2))>=70))	{

                    	retvalue=0;
                        printf("\n\n Target Unreachable");
                    }
                    if((sqrt(pow(X_pos[0]-fin[0],2)+ pow(X_pos[1]-fin[1],2)+ pow(X_pos[2]-fin[2],2))<70))	{
                        retvalue=1;
                    }

                }
                //MessageDevDriverT();
                //MessageDevDriverR();
            }

            if((HandAct==BodyTorsoArm_LEFT) || (HandAct==BodyTorsoArm_BOTH)) {

            	if(((ang4L>-99)&&(ang4L<-15))&&((ang5L>5)&&(ang5L<100)))  {

                    if((sqrt(pow(X_posL[0]-finL[0],2)+ pow(X_posL[1]-finL[1],2)+ pow(X_posL[2]-finL[2],2))>=70)) {

                    	retvalue=0;
                    	printf("\n\n Target Unreachable");
                    }
                    if((sqrt(pow(X_posL[0]-finL[0],2)+ pow(X_posL[1]-finL[1],2)+ pow(X_posL[2]-finL[2],2))<70))	{

                        retvalue=1;
                    }
                }
                //MessageDevDriverT();
                //MessageDevDriverR();
            }
        }

    	if(MSim==MSim_MOVEMENT) {

            if((HandAct==BodyTorsoArm_RIGHT) || (HandAct==BodyTorsoArm_BOTH)) {

                if(((ang4>-99)&&(ang4<-15))&&((ang5>0)&&(ang5<100)))  {

                    if((sqrt(pow(X_pos[0]-fin[0],2)+ pow(X_pos[1]-fin[1],2)+ pow(X_pos[2]-fin[2],2))>=70))	{

                        retvalue = 0;
                        printf("\n\n Target Unreachable");
                    }
                    if((sqrt(pow(X_pos[0]-fin[0],2)+ pow(X_pos[1]-fin[1],2)+ pow(X_pos[2]-fin[2],2))<70)) {

                    	retvalue =  1;
                    	//rightMove = true;
                    	/*
                    	MessagePassR();
                    	MessagePassT();
                    	cout<< "message passed to right arm"<<endl;
                    	*/
						cout <<"About to activate right arm motors" <<endl;
						cout <<"Body chain involved \t"<<HandAct<<endl;
						if (cmdInterfacePort.getOutputCount()) {
							gazeControl(X_pos[0],X_pos[1],X_pos[2]);
							cmdInterfacePassT();
							Time::delay(2);
							cmdInterfacePassR();
							Time::delay(3);
						}
						else
						{
                            MessagePassT();
                            MessagePassR();						
                            printf("WARNING: Sending motor commands with no PostureControl\n");
						}
                    	// here message to francescos module must be generated
                    }
                }
				else
				{
					printf("\nPose not safe using calculated joint angles\n");
				}
                //MessageDevDriverT();
                //MessageDevDriverR();
                //cout<< "message passed to right arm driver"<<endl;
            }

            if((HandAct==BodyTorsoArm_LEFT) || (HandAct==BodyTorsoArm_BOTH))	{

                if(((ang4L>-99)&&(ang4L<-15)))	{

                    if((sqrt(pow(X_posL[0]-finL[0],2)+ pow(X_posL[1]-finL[1],2)+ pow(X_posL[2]-finL[2],2))>=70)) {

                        retvalue=0;
                        printf("\n\n Target Unreachable");
                    }
                    if((sqrt(pow(X_posL[0]-finL[0],2)+ pow(X_posL[1]-finL[1],2)+ pow(X_posL[2]-finL[2],2))<70)) {

                        retvalue = 1;
                        //leftMove = true;
                        /*
                		MessagePassL();
                        MessagePassT();
                        cout<< "message passed to left arm"<<endl;
                       	*/
						cout <<"About to activate left arm motors" <<endl;
						cout <<"Body chain involved \t"<<HandAct<<endl;
						if (cmdInterfacePort.getOutputCount()) {
							gazeControl(X_posL[0],-X_posL[1],X_posL[2]);
							cmdInterfacePassT();
							Time::delay(2);
							cmdInterfacePassL();
							Time::delay(3);
						}
						else
						{
                            MessagePassT();
                            MessagePassL();
							printf("WARNING: Sending motor commands with no PostureControl\n");
						}
                        // here message to francescos module must be generated

                    }
                }
				else
				{

					printf("\n Pose not safe using calculated joint angles\n");
				}
                //MessageDevDriverT();
                //MessageDevDriverL();
                //cout<< "message passed to Left arm driver"<<endl;
            }
		}
	}

    //###########################################################################################################################//
    // ****************************************** CONTINUOUS ACTION *************************************************************//
    //################################################################
	if(ChoiceAct==GoalCodePMP_CONT)	{
		Kompliance(0);

		if((MiniGoal[0]==0)&&(MiniGoal[1]==0)&&(MiniGoal[2]==0))  {	//note this is for iCub , for icubSim we need to divide by 1000

     		MiniGoal[0]=-263;
     		MiniGoal[1]=160;
     		MiniGoal[2]=378;
    	}


    	if((MiniGoal[6]==0)&&(MiniGoal[7]==0)&&(MiniGoal[8]==0))  {

     		MiniGoal[6]=-263;
     		MiniGoal[7]=-160;
     		MiniGoal[8]=378;
     		MiniGoal[9]=-263;
     		MiniGoal[10]=-160;
     		MiniGoal[11]=378;
    	}

    	janini0=Jan[0];
		janini1=Jan[1];
    	janini2=Jan[2];
    	janini3=Jan[3];//-0.9425
    	janini4=Jan[4];
    	janini5=Jan[5];
    	janini6=Jan[6];
    	janini7=Jan[7];
    	janini8=Jan[8];
    	janini9=Jan[9];

    	janini3L=JanL[3];//-0.9425
    	janini4L=JanL[4];
    	janini5L=JanL[5];
    	janini6L=JanL[6];
    	janini7L=JanL[7];
    	janini8L=JanL[8];
    	janini9L=JanL[9];

    	x_iniIC=x_ini;
    	y_iniIC=y_ini;
    	z_iniIC=z_ini;
    	x_iniICL=x_iniL;
    	y_iniICL=y_iniL;
    	z_iniICL=z_iniL;

		double prevLoc[3],prevLocL[3];/////This is used for distance measurement instead of time%dividen for sending commands to robot
		prevLoc[0]=x_ini;
		prevLoc[1]=y_ini;
		prevLoc[2]=z_ini;
		prevLocL[0]=x_iniL;
		prevLocL[1]=y_iniL;
		prevLocL[2]=z_iniL;
    	//printf("\n\n FINAL ICON %f, \t  %f, \t %f \t %f, \t  %f, \t %f ",x_iniIC,y_iniIC,z_iniIC,x_iniICL,y_iniICL,z_iniICL);
    	//Sleep(5000);
		x_fin2=MiniGoal[0]; //Final Position X
    	y_fin2=MiniGoal[1];
    	z_fin2=MiniGoal[2];
    	x_fin1=MiniGoal[3]; //Final Position X
    	y_fin1=MiniGoal[4];
    	z_fin1=MiniGoal[5];

    	printf("\n Targets Right arm");
    	printf("\n \n %f, \t  %f, \t %f \t %f, \t  %f, \t %f ",x_fin2,y_fin2,z_fin2,x_fin1,y_fin1,z_fin1);


    	x_fin2L=MiniGoal[6]; //Final Position X
    	y_fin2L=-MiniGoal[7];//changed for left arm symmetry to right
    	z_fin2L=MiniGoal[8];
    	x_fin1L=MiniGoal[9]; //Final Position X
    	y_fin1L=-MiniGoal[10];//changed for left arm symmetry to right
    	z_fin1L=MiniGoal[11];

		printf("\n Targets left arm");
    	printf("\n \n %f, \t  %f, \t %f \t %f, \t  %f, \t %f ",x_fin2L,-(y_fin2L),z_fin2L,x_fin1L,-(y_fin1L),z_fin1L);

		Interpret(4,0,1,1); //Initialize trajectory formation parameters

    	for(time=0;time<ITERATION;time++) {	// 2000 incremental steps of delta 0.005

        	//KXA=1;	KXB=1; 	KYA=1; 	KYB=1; 	TSEC=1500; STARTini=1;dividen=300;
        	//KXA=1;	KXB=10; 	KYA=1; 	KYB=10; TSEC=1000;STARTini=2; dividen=300; // bump
        	//KXA=10;	KXB=1; 	KYA=1; 	KYB=10; TSEC=1000; STARTini=1; dividen=300;//cusp
      		double GamA,GamB;
    		GamA=Gamma(time);
    		GamB=Gamma1(time-TSEC);
    		if(time<=TSEC) {
      			GamB=0;
      			csi_dot1=0;
     		}
    		if(time>=1500)	{
      			GamA=0;
      			csi_dot=0;
     		}
    		if(time>=2500)	{
      			csi_dot1=0;
     		}
       		Gam=GamA+GamB;
       		wr << time<<"\t"<< GamA<<"\t"<<GamB<<"\t"<<Gam<<"\t"<<csi_dot<<"\t"<<csi_dot1<< endl;
        	//fprintf(wr,"\n  %d \t  %f \t  %f \t  %f \t  %f \t  %f",time,GamA,GamB,Gam,csi_dot,csi_dot1); /// v.txt has values of Gamma and time

    		double inter_x1=((x_fin1-x_ini)*GamA);
    		double inter_x2=((x_fin2-x_ini)*GamB);
    		Gam_Arr1[time]=inter_x1;
    		Gam_Arr2[time]=inter_x2;
    		double *Gar1=Gam_Arr1;
    		double *Gar2=Gam_Arr2;
    		x_ini=KXA*Gamma_Int(Gar1,time)+ KXB*Gamma_Int(Gar2,time)+x_iniIC;

    		double inter_y1=((y_fin1-y_ini)*GamA);
    		double inter_y2=((y_fin2-y_ini)*GamB);
    		Gam_Arry1[time]=inter_y1;
    		Gam_Arry2[time]=inter_y2;
    		double *Gary1=Gam_Arry1;
    		double *Gary2=Gam_Arry2;
    		y_ini=KYA*Gamma_Int(Gary1,time)+KYB*Gamma_Int(Gary2,time)+y_iniIC;

    		double inter_z1=((z_fin1-z_ini)*GamA);
    		double inter_z2=((z_fin2-z_ini)*GamB);
    		Gam_Arrz1[time]=inter_z1;
    		Gam_Arrz2[time]=inter_z2;
    		double *Garz1=Gam_Arrz1;
    		double *Garz2=Gam_Arrz2;
    		z_ini=KYA*Gamma_Int(Garz1,time)+KYB*Gamma_Int(Garz2,time)+z_iniIC;


    		double inter_x1L=((x_fin1L-x_iniL)*GamA);
    		double inter_x2L=((x_fin2L-x_iniL)*GamB);
    		Gam_Arr1L[time]=inter_x1L;
    		Gam_Arr2L[time]=inter_x2L;
    		double *Gar1L=Gam_Arr1L;
    		double *Gar2L=Gam_Arr2L;
    		x_iniL=KXAL*Gamma_Int(Gar1L,time)+ KXBL*Gamma_Int(Gar2L,time)+x_iniICL;

    		double inter_y1L=((y_fin1L-y_iniL)*GamA);
    		double inter_y2L=((y_fin2L-y_iniL)*GamB);
    		Gam_Arry1L[time]=inter_y1L;
    		Gam_Arry2L[time]=inter_y2L;
    		double *Gary1L=Gam_Arry1L;
    		double *Gary2L=Gam_Arry2L;
    		y_iniL=KYAL*Gamma_Int(Gary1L,time)+KYBL*Gamma_Int(Gary2L,time)+y_iniICL;

    		double inter_z1L=((z_fin1L-z_iniL)*GamA);
    		double inter_z2L=((z_fin2L-z_iniL)*GamB);
    		Gam_Arrz1L[time]=inter_z1L;
    		Gam_Arrz2L[time]=inter_z2L;
    		double *Garz1L=Gam_Arrz1L;
    		double *Garz2L=Gam_Arrz2L;
    		z_iniL=KYAL*Gamma_Int(Garz1L,time)+KYBL*Gamma_Int(Garz2L,time)+z_iniICL;

     		if (verboseTerm) {
     			wr1L << x_ini << "    " << y_ini << "    " << z_ini << "    "  << x_iniL << "    " << -y_iniL << "    " << z_iniL <<endl;
    		}
    		MotCon(x_ini,y_ini,z_ini, x_iniL,y_iniL,z_iniL,time,Gam,HandAct);

    		if (verboseTerm) {
    			posiL << X_pos[0] << "    " << X_pos[1] << "    " << X_pos[2] << "    "  << X_posL[0] << "    " << -X_posL[1] << "    " << X_posL[2] <<endl;
    			wr_Gam << Jan[0] << "  " << Jan[1]<< "  " << Jan[2]<< "  " << Jan[3]<< "  " << Jan[4]<< "  " << Jan[5]<< "  " << Jan[6]<< "  " << Jan[7]<< "  " << Jan[8]<< "  " << Jan[9]<< "  " << JanL[3]<< "  " << JanL[4]<< "  " << JanL[5]<< "  " << JanL[6]<< "  " << JanL[7]<< "  " << JanL[8]<< "  " << JanL[9] <<endl;
			}
    		//printf("\n\n FINAL SOLUTION  %f, \t  %f, \t %f \t %f, \t  %f, \t %f ",X_pos[0],X_pos[1],X_pos[2],X_posL[0],-(X_posL[1]),X_posL[2]);
			
///////////////////ploy x y z on yarpscope
			if (cmdEndEffectorPort.getOutputCount() >0) {
				Bottle & PlotPoints = cmdEndEffectorPort.prepare();
				PlotPoints.clear();
				PlotPoints.addDouble(Gam);
				if (HandAct==0){
					PlotPoints.addDouble( X_pos[0]);
					PlotPoints.addDouble( X_pos[1]);
					PlotPoints.addDouble( X_pos[2]);
					//PlotPoints.addDouble(sqrt(pow(X_pos[0]-x_ini,2)+pow(X_pos[1]-y_ini,2)+pow(X_pos[2]-z_ini,2)));
				}
				if (HandAct==1){
					PlotPoints.addDouble(X_posL[0]);
					PlotPoints.addDouble(-X_posL[1]);
					PlotPoints.addDouble(X_posL[2]);
					//PlotPoints.addDouble(sqrt(pow(X_posL[0]-x_iniL,2)+pow(X_posL[1]-y_iniL,2)+pow(X_posL[2]-z_iniL,2)));
				}
				cmdEndEffectorPort.write(true);
				Time::delay(0.02);
			}
/////////////////////////////////////

    		if(MSim==MSim_SIMULATION)  {

        		if((HandAct==BodyTorsoArm_RIGHT) || (HandAct==BodyTorsoArm_BOTH)) {
        	        //SIMULATE RIGHT ARM
        	        if(((ang4>-99)&&(ang4<-15))&&((ang5>9)&&(ang5<100))) {

        	            if((sqrt(pow(X_pos[0]-fin[0],2)+ pow(X_pos[1]-fin[1],2)+ pow(X_pos[2]-fin[2],2))>=70)) {

        	                retvalue=0;
        	                printf("\n\n Target Unreachable");
        	            }
        	            if((sqrt(pow(X_pos[0]-fin[0],2)+ pow(X_pos[1]-fin[1],2)+ pow(X_pos[2]-fin[2],2))<70)) {

        	                retvalue=1;
        	            }

        	        }
        	        //MessageDevDriverT();
        	        //MessageDevDriverR();
        	    }

        	    if((HandAct==BodyTorsoArm_LEFT) || (HandAct==BodyTorsoArm_BOTH)) {
        	        //SIMULATE LEFT ARM
        	        if(((ang4L>-99)&&(ang4L<-15))&&((ang5L>9)&&(ang5L<100))) {

        	            if((sqrt(pow(X_posL[0]-finL[0],2)+ pow(X_posL[1]-finL[1],2)+ pow(X_posL[2]-finL[2],2))>=70))  {

        	                retvalue=0;
        	                printf("\n\n Target Unreachable");
        	            }
        	            if((sqrt(pow(X_posL[0]-finL[0],2)+ pow(X_posL[1]-finL[1],2)+ pow(X_posL[2]-finL[2],2))<70)) {

        	                retvalue=1;
        	            }
        	        }
        	        //MessageDevDriverT();
        	        //MessageDevDriverR();
        	    }
        	}
			
     		if(MSim==MSim_MOVEMENT)	{
        	    if((HandAct==BodyTorsoArm_RIGHT) || (HandAct==BodyTorsoArm_BOTH))  {
        	       // MOVEMENT RIGHT MOVEMENT
					double disMeasure = sqrt(pow(X_pos[0]-prevLoc[0],2)+ pow(X_pos[1]-prevLoc[1],2)+ pow(X_pos[2]-prevLoc[2],2));
        	        if(((time%dividen)==0)&&(time>0)&&(disMeasure>20)) {
					
    					ang1=rad2degree*Jan[0];
    					ang2=rad2degree*Jan[1];
    					ang3=rad2degree*Jan[2];
    					ang4=rad2degree*Jan[3];
    					ang5=rad2degree*Jan[4];
    					ang6=rad2degree*Jan[5];
    					ang7=rad2degree*Jan[6];
    					ang8=Wrist;
    					ang9=rad2degree*Jan[8];
    					ang10=rad2degree*Jan[9];

     					angCup=36;
    					ang4L=rad2degree*JanL[3];
    					ang5L=rad2degree*JanL[4];
    					ang6L=rad2degree*JanL[5];
    					ang7L=rad2degree*JanL[6];
    					ang8L=Wrist;
    					ang9L=rad2degree*JanL[8];
    					ang10L=rad2degree*JanL[9];
     					angCupL=36;
   						printf("\n  %.1f, \t  %.1f, \t %.1f ,\t %.1f ,\t %.1f ,\t %.1f, \t  %.1f, \t %.1f ,\t %.1f ,\t %.1f\n",ang1,ang2,ang3,ang4,ang5, ang6,ang7,ang8,ang9,ang10);
    					//printf("\n  %d, \t  %d, \t %d ,\t %d ,\t %d ,\t %d, \t  %d, \t %d ,\t %d ,\t %d",ang1,ang2,ang3,ang4L,ang5L,ang6L,ang7L,ang8L,ang9L,ang10L);
     					//printf("\n\n FINAL SOLUTION  %f, \t  %f, \t %f \t %f, \t  %f, \t %f ",X_pos[0],X_pos[1],X_pos[2],X_posL[0],X_posL[1],X_posL[2]);
        	            //MessageDevDriverT();
        	            //MessageDevDriverR();
        	            if(((ang4>-99)&&(ang4<-15))&&((ang5>9)&&(ang5<100))) {

        	                retvalue =  1;
        	                /*
        	                MessagePassT();
        	                //Sleep(2000);
        	                MessagePassR();
        	                //Sleep(2000);
        	                */
							cout <<"About to activate right arm motors" <<endl;
							cout <<"Body chain involved (right)\t"<<HandAct<<endl;
							if (cmdInterfacePort.getOutputCount()) {
								gazeControl(X_pos[0],X_pos[1],X_pos[2]);
								cmdInterfacePassR();
								Time::delay(3);
								cmdInterfacePassT();
								Time::delay(2);
							}
							else
							{
								MessagePassT();
                                MessagePassR();
                                printf("WARNING: Sending motor commands with no PostureControl\n");
							}
							
        	                Proprioceptive[0]=X_pos[0];
        	                Proprioceptive[1]=X_pos[1];
							Proprioceptive[2]=X_pos[2];
							printf(" Proprioceptive prediction  %f \t %f \t %f\n",Proprioceptive[0],Proprioceptive[1],Proprioceptive[2]);
							prevLoc[0]=X_pos[0];
							prevLoc[1]=X_pos[1];
							prevLoc[2]=X_pos[2];
        	            }
						else
						{
							printf("\nPose not safe using calculated joint angles\n");
						}
        	            if((time%(3*dividen))==0) {

							//int loopV=V1.colSegMainR();
        	            }
        	            // VisionSystem V2;
        	            //V2.colSegMainR();
        	        }
        	    }

        	    if((HandAct==BodyTorsoArm_LEFT) || (HandAct==BodyTorsoArm_BOTH)) {
        	         //MOVEMENT LEFT MOVEMENT
					double disMeasure = sqrt(pow(X_posL[0]-prevLocL[0],2)+ pow(X_posL[1]-prevLocL[1],2)+ pow(X_posL[2]-prevLocL[2],2));
        	        if(((time%dividen)==0)&&(time>0)&&(disMeasure>20)) {
					
        	            ang1=rad2degree*Jan[0];
        	            ang2=rad2degree*Jan[1];
        	            ang3=rad2degree*Jan[2];
        	            ang4=rad2degree*Jan[3];
        	            ang5=rad2degree*Jan[4];
        	            ang6=rad2degree*Jan[5];
        	            ang7=rad2degree*Jan[6];
        	            ang8=Wrist;
        	            ang9=rad2degree*Jan[8];
        	            ang10=rad2degree*Jan[9];

        	            angCup=36;
        	            ang4L=rad2degree*JanL[3];
        	            ang5L=rad2degree*JanL[4];
        	            ang6L=rad2degree*JanL[5];
        	            ang7L=rad2degree*JanL[6];
        	            ang8L=Wrist;
        	            ang9L=rad2degree*JanL[8];
        	            ang10L=rad2degree*JanL[9];
        	            angCupL=36;
        	            //	printf("\n  %.1f, \t  %d, \t %d ,\t %d ,\t %d ,\t %d, \t  %d, \t %d ,\t %d ,\t %d",ang1,ang2,ang3,ang4,ang5, ang6,ang7,ang8,ang9,ang10);
        	            printf("\n  %.1f, \t  %.1f, \t %.1f ,\t %.1f ,\t %.1f ,\t %.1f, \t  %.1f, \t %.1f ,\t %.1f ,\t %.1f\n",-ang1,-ang2,ang3,ang4L,ang5L,ang6L,ang7L,ang8L,ang9L,ang10L);

        	            //  printf("\n\n FINAL SOLUTION  %f, \t  %f, \t %f \t %f, \t  %f, \t %f ",X_pos[0],X_pos[1],X_pos[2],X_posL[0],X_posL[1],X_posL[2]);
        	            //MessageDevDriverT();
        	            //MessageDevDriverL();
            	        if(((ang4L>-99)&&(ang4L<-15))&&((ang5L>9)&&(ang5L<100))) {
            	            retvalue = 1;
            	            //leftMove = true;
            	            /*
            	            MessagePassT();
            	            //Sleep(2000);
            	            MessagePassL();
            	            //Sleep(2000);
            	            */
							cout <<"About to activate left arm motors" <<endl;
							cout <<"Body chain involved (left)\t"<<HandAct<<endl;
							if (cmdInterfacePort.getOutputCount()) {
								gazeControl(X_posL[0],-X_posL[1],X_posL[2]);
								cmdInterfacePassL();
								Time::delay(3);
								cmdInterfacePassT();
								Time::delay(2);
							}
							else
							{
                                MessagePassT();
                                MessagePassL();
								printf("WARNING: Sending motor commands with no PostureControl\n");
							}
            	            Proprioceptive[0]=X_posL[0];
            	            Proprioceptive[1]=-X_posL[1];
							Proprioceptive[2]=X_posL[2];
            	            printf("Proprioceptive prediction  %f \t %f \t %f \n",Proprioceptive[0],Proprioceptive[1],Proprioceptive[2]);
							prevLocL[0]=X_posL[0];
							prevLocL[1]=X_posL[1];
							prevLocL[2]=X_posL[2];
            	        }
						else
						{
							printf("\nPose not safe using calculated joint angles\n");
						}

            	        if((time%(3*dividen))==0) {
            	            {
							//int loopvL=V1.colSegMainR();
            	            }
            	        }
            	    }
           	 	}
        	}

    	}

    }
	//code with motor commands to robot was here


    return retvalue;
};

void PMPThread::Interpret(int CCode,int PtCode,double AmplificationX,double AmplificationZ)
        {
                if(CCode==1) //B
                {
                x_off1=(-60)*AmplificationX;z_off1=(-50)*AmplificationZ;x_off2=0*AmplificationX;z_off2=(-100)*AmplificationZ;
                KXA=10;	KXB=1; 	KYA=1; 	KYB=10; 	TSEC=500; STARTini=1;
                }

                if(CCode==2)//Bump
                {
                x_off1=(-30)*AmplificationX;z_off1=(100)*AmplificationZ;x_off2=(-60)*AmplificationX;z_off2=0*AmplificationZ;
                KXA=1;	KXB=10; 	KYA=1; 	KYB=10; TSEC=1000;STARTini=2;dividen=300; KXAL=1;	KXBL=10; 	KYAL=1; 	KYBL=10;
                }

                if(CCode==3)//U
                {
                x_off1=(-30)*AmplificationX;z_off1=(-100)*AmplificationZ;x_off2=(-60)*AmplificationX;z_off2=0*AmplificationZ;
                KXA=1;	KXB=5; 	KYA=10; KYB=5; TSEC=300;STARTini=1;
                }

                if(CCode==4)//C
                {
                x_off1=(60)*AmplificationX;z_off1=(-50)*AmplificationZ;x_off2=0*AmplificationX;z_off2=(-100)*AmplificationZ;
                //KXA=10;	KXB=1; 	KYA=1; 	KYB=10; TSEC=1000; STARTini=1;dividen=300; KXAL=10;	KXBL=1; 	KYAL=1; 	KYBL=10;// this was original..bump in x y in iCub's old frame of ref: Ajaz
				//KXA=1;	KXB=10; 	KYA=10; 	KYB=1; TSEC=1000; STARTini=1;dividen=300; KXAL=1;	KXBL=10; 	KYAL=10; 	KYBL=1;//KXAL=2;	KXBL=6; 	KYAL=6; 	KYBL=2; //Ajaz
                //KXA=8;	KXB=10; 	KYA=3; 	KYB=5; TSEC=1000; STARTini=1;dividen=30; KXAL=4;	KXBL=4; 	KYAL=8; 	KYBL=10;//me
				 KXA=8;	KXB=10; 	KYA=3; 	KYB=5; TSEC=1000; STARTini=1;dividen=5; KXAL=3;	KXBL=5; 	KYAL=8; 	KYBL=8;//with vishuu
				}
                if(CCode==11)//St Line
                {
                x_off1=0;z_off1=(-100)*AmplificationZ;x_off2=0;z_off2=(-100)*AmplificationZ;
                KXA=1;	KXB=1; 	KYA=1; 	KYB=1; 	TSEC=1500; STARTini=1;
                }
                //for the time being not dealing with startini ...

        };

void PMPThread::GraspR()
             {
                      //Sleep(4000);
                      CubGrazp3();
                       if(((ang4>-99)&&(ang4<-15))&&((ang5>5)&&(ang5<100)))
                        {
                            MessagePassR();
                        }
                           //Sleep(4000);
                           CubGrazp4();
                       if(((ang4>-99)&&(ang4<-15))&&((ang5>5)&&(ang5<100)))
                        {
                            MessagePassR();
                        }
                       //may add grasp detect vision here to confim if the object is grasped
                 };

void PMPThread::GraspL()
             {
                      //Sleep(4000);
                      CubGrazpL3();
                      if(((ang4L>-99)&&(ang4L<-15))&&((ang5L>=0)&&(ang5L<100)))
                       {
                            MessagePassL();
                        }
                           //Sleep(4000);
                           CubGrazpL4();
                       if(((ang4L>-99)&&(ang4L<-15))&&((ang5L>=0)&&(ang5L<100)))
                        {
                            MessagePassL();
                        }
                       //Sleep(10000);
                       // may add grasp detect vision to confirm if the object was grasped
                 };

void PMPThread::MotCon(double T1, double T2, double T3,double TL1, double TL2, double TL3, int time, double Gam, int HAct)	{
 // cout<< HAct<< endl;

  /*  double *ang = Jan;
    double *angL = JanL;
    int len=1,i;
    double *topmp, *topmpL,*force=0,*forceL=0;
//===============================================================
   	if((HAct==BodyTorsoArm_RIGHT) || (HAct==BodyTorsoArm_BOTH)) {

        double *nFK = forward_Kinematics(ang,len); // Joint Angles to Positions 3>>>>>2

        for(i=0;i<3;i++)  {

            X_pos[i]=*(nFK+i);
        }
        if((HAct==BodyTorsoArm_RIGHT)) {
            X_posL[0]=x_iniL;
            X_posL[1]=y_iniL;
            X_posL[2]=z_iniL;
        }
        double *po=X_pos;
        //===================================================================
        target[0]=T1;
        target[1]=T2;
        target[2]=T3;

        double *ta=target;
        //if(HAct==BodyTorsoArm_LEFT) {
        //    ta=po;
        //}
        force=forcefield(po,ta); // Position to Force 3>>>>>3
        for(i=0;i<3;i++)	{
            ffield[i]=*(force+i);
            //ffieldL[i]=0;
        }
        //topmp= ffield;
        //topmpL= ffieldL;
    }

    if((HAct==BodyTorsoArm_LEFT) || (HAct==BodyTorsoArm_BOTH))  {

        double *nLeft = forward_KinematicsL(angL,len); // Joint Angles to Positions 3>>>>>2

        for(i=0;i<3;i++) {
        	X_posL[i]=*(nLeft+i);
        	//cout <<"X positions"<<X_posL[i]<<endl;
        }
        if((HAct==BodyTorsoArm_LEFT)) {

            X_pos[0]=x_ini;
            X_pos[1]=y_ini;
            X_pos[2]=z_ini;
        }

        double *poL=X_posL;
        targetL[0]=TL1;
        targetL[1]=TL2;
        targetL[2]=TL3;

        double *taL=targetL;
        //if(HAct==BodyTorsoArm_RIGHT)  {
        //    taL=poL;
        //}
        forceL=forcefieldL(poL,taL); // Position to Force 3>>>>>3
        for(i=0;i<3;i++) {

            //ffield[i]=0;
            ffieldL[i]=*(forceL+i);
            //cout <<"forcefieldsL"<<ffieldL[i]<<endl;
        }

    }
    topmp= ffield;
    topmpL= ffieldL;
//===================================================================
//                  	PASSIVE MOTION PARADIGM
//===================================================================
    double *Q_Dot=PMP(topmp,topmpL);  //Force to Torque to Q_dot 2>>>>>3
//===================================================================
//===================================================================

    for(i=0;i<20;i++)	{      //MODIFY
        JoVel[i]=(*(Q_Dot+i))*Gam;
    }
//===================================================================
    // Q1-Q3 :W , JAN 0 - JAN 2 : W = JANL 0 - JANL 2;   JAN 3-JAN 9 RIGHT ARM; JANL 13-JANL19 LEFT ARM
//===================================================================
    // From Q_dots to Q >>>>>
    q1[time]=JoVel[0];
    double *j1=q1;
    double joi1=Gamma_Int(j1,time);
    Jan[0]=joi1+janini0;
    JanL[0]=joi1+janini0;

    q2[time]=JoVel[1];
    double *j2=q2;
    double joi2=Gamma_Int(j2,time);
    Jan[1]=joi2;
    JanL[1]=joi2;

    q3[time]=JoVel[2];
    double *j3=q3;
    double joi3=Gamma_Int(j3,time);
    Jan[2]=joi3+janini2;
    JanL[2]=joi3+janini2;

    if((HAct==BodyTorsoArm_RIGHT)||(HAct==BodyTorsoArm_BOTH))  {

        q4[time]=JoVel[3];
        double *j4=q4;
        double joi4=Gamma_Int(j4,time);
        Jan[3]=joi4+janini3;

        q5[time]=JoVel[4];
        double *j5=q5;
        double joi5=Gamma_Int(j5,time);
        Jan[4]=joi5+janini4;

        q6[time]=JoVel[5];
        double *j6=q6;
        double joi6=Gamma_Int(j6,time);
        Jan[5]=joi6+janini5;

        q7[time]=JoVel[6];
        double *j7=q7;
        double joi7=Gamma_Int(j7,time);
        Jan[6]=joi7+janini6;

        q8[time]=JoVel[7];
        double *j8=q8;
        double joi8=Gamma_Int(j8,time);
        Jan[7]=joi8+janini7;

        q9[time]=JoVel[8];
        double *j9=q9;
        double joi9=Gamma_Int(j9,time);
        Jan[8]=joi9+janini8;

        q10[time]=JoVel[9];
        double *j10=q10;
        double joi10=Gamma_Int(j10,time);
        Jan[9]=joi10+janini9;

        if(HAct==BodyTorsoArm_RIGHT) {

            JanL[3]=-1.74;// -0.6981
            JanL[4]=0.78;
            JanL[5]=0;
            JanL[6]=1.3;
            JanL[7]=0;
            JanL[8]=0;
            JanL[9]=0;
        }
    }

    // for LEFT HAND
    if((HAct==BodyTorsoArm_LEFT)||(HAct==BodyTorsoArm_BOTH)) {

        q4L[time]=JoVel[13];
        double *j4L=q4L;
        double joi4L=Gamma_Int(j4L,time);
        JanL[3]=joi4L+janini3L;

        q5L[time]=JoVel[14];
        double *j5L=q5L;
        double joi5L=Gamma_Int(j5L,time);
        JanL[4]=joi5L+janini4L;

        q6L[time]=JoVel[15];
        double *j6L=q6L;
        double joi6L=Gamma_Int(j6L,time);
        JanL[5]=joi6L+janini5L;

        q7L[time]=JoVel[16];
        double *j7L=q7L;
        double joi7L=Gamma_Int(j7L,time);
        JanL[6]=joi7L+janini6L;

        q8L[time]=JoVel[17];
        double *j8L=q8L;
        double joi8L=Gamma_Int(j8L,time);
        JanL[7]=joi8L+janini7L;

        q9L[time]=JoVel[18];
        double *j9L=q9L;
        double joi9L=Gamma_Int(j9L,time);
        JanL[8]=joi9L+janini8L;

        q10L[time]=JoVel[19];
        double *j10L=q10L;
        double joi10L=Gamma_Int(j10L,time);
        JanL[9]=joi10L+janini9L;

        if(HAct==BodyTorsoArm_LEFT) {

            Jan[3]=-1.74;// -0.6981
            Jan[4]=0.78;
            Jan[5]=0;
            Jan[6]=1.3;
            Jan[7]=0;
            Jan[8]=0;
            Jan[9]=0;
        }
    }
    // From Q >> X > Force > Torques >Q_dots >Q ....................
//=================================================================== */
    double *ang = Jan;
	double *angL = JanL;
	int len=1,i;
	double *topmp =0;
    double *topmpL =0;
//===============================================================
   if((HAct==0)||(HAct==2))
   {
		double *nFK = forward_Kinematics(ang,len); // Joint Angles to Positions 3>>>>>2

		for(i=0;i<3;i++)
		{
		X_pos[i]=*(nFK+i);
		}

		if((HAct==0))
		{
		X_posL[0]=x_iniL;
		X_posL[1]=-y_iniL;
		X_posL[2]=z_iniL;
		}
		double *po=X_pos;
		//===================================================================
		target[0]=T1;
		target[1]=T2;
		target[2]=T3;

		double *ta=target;
		if(HAct==1)
		{
		ta=po;
		}
		double *force=forcefield(po,ta); // Position to Force 3>>>>>3
		 for(i=0;i<3;i++)
		{
		ffield[i]=*(force+i);
		//ffieldL[i]=0;
		}
		topmp= ffield;
		topmpL= ffieldL;




   }

   if((HAct==1)||(HAct==2))
   {
		double *nLeft = forward_Kinematics(angL,len); // Joint Angles to Positions 3>>>>>2

		for(i=0;i<3;i++)
		{
		X_posL[i]=*(nLeft+i);
		}
   		if((HAct==1))
		{
		X_pos[0]=x_ini;
		X_pos[1]=y_ini;
		X_pos[2]=z_ini;
		}

		double *poL=X_posL;
		targetL[0]=TL1;
		targetL[1]=TL2;
		targetL[2]=TL3;

		double *taL=targetL;
		if(HAct==0)
		{
		taL=poL;
		}
		double *forceL=forcefield(poL,taL); // Position to Force 3>>>>>3
		 for(i=0;i<3;i++)
		{
        //ffield[i]=0;
        ffieldL[i]=*(forceL+i);
		}
		topmp= ffield;
		topmpL= ffieldL;
   }
//===================================================================
	//                  PASSIVE MOTION PARADIGM
//===================================================================
   	double *Q_Dot=PMP(topmp,topmpL);  //Force to Torque to Q_dot 2>>>>>3
//===================================================================
//===================================================================

	for(i=0;i<20;i++)          //MODIFY
		{
		JoVel[i]=Jvel[i]*Gam;
        //cout<<JoVel[i]<<endl;
		}
//===================================================================
	// Q1-Q3 :W , JAN 0 - JAN 2 : W = JANL 0 - JANL 2;   JAN 3-JAN 9 RIGHT ARM; JANL 13-JANL19 LEFT ARM
//===================================================================
	// From Q_dots to Q >>>>>


 // Right or Both
if((HAct==0)||(HAct==2))
   {
	q1[time]=JoVel[0];
   	double *j1=q1;
	double joi1=Gamma_Int(j1,time);
	Jan[0]=joi1+janini0;
	JanL[0]=Jan[0];

	q2[time]=JoVel[1];
    double *j2=q2;
	double joi2=Gamma_Int(j2,time);
	Jan[1]=joi2+janini1;
	JanL[1]=Jan[1];

	q3[time]=JoVel[2];
    double *j3=q3;
	double joi3=Gamma_Int(j3,time);
	Jan[2]=joi3+janini2;
	JanL[2]=Jan[2];

	q4[time]=JoVel[3];
    double *j4=q4;
	double joi4=Gamma_Int(j4,time);
	Jan[3]=joi4+janini3;

	q5[time]=JoVel[4];
    double *j5=q5;
	double joi5=Gamma_Int(j5,time);
	Jan[4]=joi5+janini4;

	q6[time]=JoVel[5];
    double *j6=q6;
	double joi6=Gamma_Int(j6,time);
	Jan[5]=joi6+janini5;

	q7[time]=JoVel[6];
    double *j7=q7;
	double joi7=Gamma_Int(j7,time);
	Jan[6]=joi7+janini6;

	q8[time]=JoVel[7];
    double *j8=q8;
	double joi8=Gamma_Int(j8,time);
	Jan[7]=joi8+janini7;

	q9[time]=JoVel[8];
    double *j9=q9;
	double joi9=Gamma_Int(j9,time);
	Jan[8]=joi9+janini8;

	q10[time]=JoVel[9];
    double *j10=q10;
	double joi10=Gamma_Int(j10,time);
	Jan[9]=joi10+janini9;

/*	        JanL[3]=-1.74;// -0.6981
	        JanL[4]=0.78;
			JanL[5]=0;
			JanL[6]=1.3;
			JanL[7]=0;
			JanL[8]=0;
			JanL[9]=0;
*/
}

	// for LEFT HAND OR BOTH
if((HAct==1)||(HAct==2))
   {

	q1L[time]=JoVel[10];
    double *j1L=q1L;
	double joi1L=Gamma_Int(j1L,time);
	JanL[0]=joi1L+janini0;
	Jan[0]=JanL[0];

	q2L[time]=JoVel[11];
    double *j2L=q2L;
	double joi2L=Gamma_Int(j2L,time);
	JanL[1]=joi2L+janini1;
	Jan[1]=JanL[1];

	q3L[time]=JoVel[12];
    double *j3L=q3L;
	double joi3L=Gamma_Int(j3L,time);
	JanL[2]=joi3L+janini2;
	Jan[2]=JanL[2];

	q4L[time]=JoVel[13];
    double *j4L=q4L;
	double joi4L=Gamma_Int(j4L,time);
	JanL[3]=joi4L+janini3L;

	q5L[time]=JoVel[14];
    double *j5L=q5L;
	double joi5L=Gamma_Int(j5L,time);
	JanL[4]=joi5L+janini4L;

	q6L[time]=JoVel[15];
    double *j6L=q6L;
	double joi6L=Gamma_Int(j6L,time);
	JanL[5]=joi6L+janini5L;

	q7L[time]=JoVel[16];
    double *j7L=q7L;
	double joi7L=Gamma_Int(j7L,time);
	JanL[6]=joi7L+janini6L;

	q8L[time]=JoVel[17];
    double *j8L=q8L;
	double joi8L=Gamma_Int(j8L,time);
	JanL[7]=joi8L+janini7L;

	q9L[time]=JoVel[18];
    double *j9L=q9L;
	double joi9L=Gamma_Int(j9L,time);
	JanL[8]=joi9L+janini8L;

	q10L[time]=JoVel[19];
    double *j10L=q10L;
	double joi10L=Gamma_Int(j10L,time);
	JanL[9]=joi10L+janini9L;

/*	        Jan[3]=-1.74;// -0.6981
	        Jan[4]=0.78;
			Jan[5]=0;
			Jan[6]=1.3;
			Jan[7]=0;
			Jan[8]=0;
			Jan[9]=0;
*/
}
	// From Q >> X > Force > Torques >Q_dots >Q ....................
//===================================================================
}


void PMPThread::MessageDevDriverR()
     {

        Property options;
        options.put("device", "remote_controlboard");
        options.put("local", "/test/client");   //local port names
        std::string p = "/";
        p.append(robot.c_str());
        p.append("/right_arm");
        options.put("remote", p.c_str());         //where we connect to
		//options.put("remote", "/icubSim/right_arm");         //where we connect to

        // create a device
        PolyDriver robotDevice(options);
        if (!robotDevice.isValid()) {
            printf("Device not available.  Here are the known devices:\n");
            printf("%s", Drivers::factory().toString().c_str());
            //return 0;
        }

        IPositionControl *pos;
        IEncoders *encs;

        bool ok;
        ok = robotDevice.view(pos);
        ok = ok && robotDevice.view(encs);

        if (!ok) {
            printf("Problems acquiring RIGHT ARM interfaces\n");
            //return 0;
        }

        int nj=0;
        pos->getAxes(&nj);
        //printf("Joints %d \n", nj);

        Vector encoders;
        Vector command;
        Vector tmp;
        encoders.resize(nj);
        tmp.resize(nj);
        command.resize(nj);

        int i;
        for (i = 0; i < nj; i++) {
             tmp[i] = 60.0;
        }
        pos->setRefAccelerations(tmp.data());

        /*
        if needed we can check the position of our axes by doing:

            \code
            enc->getEncoders(encoders.data());
            \endcode

            int count=50;
            while(count--)
                {
                    Time::delay(0.1);
                    encs->getEncoders(encoders.data());
                    printf("%.1lf %.1lf %.1lf %.1lf\n", encoders[0], encoders[1], encoders[2], encoders[3]);
                }
        */

        for (i = 0; i < nj; i++) {
            tmp[i] = 30.0;
            pos->setRefSpeed(i, tmp[i]);
        }
        command=0;

                command[0]=ang4;
                command[1]=ang5;
                command[2]=ang6;
                command[3]=ang7;
                command[4]=ang8;
                command[5]=ang9;
                command[6]=ang10;
                command[7]=angCup;
                command[8]=angT1;
                command[9]=angT2;
                command[10]=angT3;
                command[11]=angI1;
                command[12]=angI2;
                command[13]=angM1;
                command[14]=angM2;
                command[15]=angRP;


                pos->positionMove(command.data());

                bool done=false;

                while(!done)
                {
                    pos->checkMotionDone(&done);
                    Time::delay(0.1);
                }


        robotDevice.close();
    };


void PMPThread::MessageDevDriverL()
 {

    int i;
    Property optionsL;
    optionsL.put("device", "remote_controlboard");
    optionsL.put("local", "/test/clientL");   //local port names
    std::string p = "/";
    p.append(robot.c_str());
    p.append("/left_arm");
    optionsL.put("remote", p.c_str());         //where we connect to
    //optionsL.put("remote", "/icubSim/left_arm");         //where we connect to

    // create a device
    PolyDriver robotDeviceL(optionsL);
    if (!robotDeviceL.isValid()) {
        printf("Device not available.  Here are the known devices:\n");
        printf("%s", Drivers::factory().toString().c_str());
        //return 0;
    }


    IPositionControl *posL;
    IEncoders *encsL;

    bool okL;
    okL = robotDeviceL.view(posL);
    okL = okL && robotDeviceL.view(encsL);

    if (!okL) {
        printf("Problems acquiring LEFT ARM interfaces\n");
        //return 0;
    }

    int njL=0;
    posL->getAxes(&njL);
    //printf("Joints Left %d \n", njL);

    Vector encodersL;
    Vector commandL;
    Vector tmpL;
    encodersL.resize(njL);
    tmpL.resize(njL);
    commandL.resize(njL);


    for (i = 0; i < njL; i++) {
         tmpL[i] = 60.0;
    }
    posL->setRefAccelerations(tmpL.data());


     for (i = 0; i < njL; i++) {
        tmpL[i] = 30.0;
        posL->setRefSpeed(i, tmpL[i]);
    }


    commandL=0;

            commandL[0]=ang4L;
            commandL[1]=ang5L;
            commandL[2]=ang6L;
            commandL[3]=ang7L;
            commandL[4]=ang8L;
            commandL[5]=ang9L;
            commandL[6]=ang10L;
            commandL[7]=angCupL;
            commandL[8]=angTL1;
            commandL[9]=angTL2;
            commandL[10]=angTL3;
            commandL[11]=angIL1;
            commandL[12]=angIL2;
            commandL[13]=angML1;
            commandL[14]=angML2;
            commandL[15]=angRPL;

            posL->positionMove(commandL.data());


            bool doneL=false;

            while(!doneL)
            {
                posL->checkMotionDone(&doneL);
                Time::delay(0.1);
            }

    robotDeviceL.close();
 };

void PMPThread::MessageDevDriverT()
 {
   int i;
    Property optionsT;
    optionsT.put("device", "remote_controlboard");
    optionsT.put("local", "/test/clientT");   //local port names
	std::string p = "/";
    p.append(robot.c_str());
    p.append("/torso");
    optionsT.put("remote", p.c_str());         //where we connect to
    //optionsT.put("remote", "/icubSim/torso");         //where we connect to

    // create a device
    PolyDriver robotDeviceT(optionsT);
    if (!robotDeviceT.isValid()) {
        printf("Device not available.  Here are the known devices:\n");
        printf("%s", Drivers::factory().toString().c_str());
        //return 0;
    }

    IPositionControl *posT;
    IEncoders *encsT;

    bool okT;
    okT = robotDeviceT.view(posT);
    okT = okT && robotDeviceT.view(encsT);

 if (!okT) {
        printf("Problems acquiring Torso interfaces\n");
        //return 0;
    }

    int njT=0;
    posT->getAxes(&njT);
    //printf("Joints Torso %d \n", njT);

   Vector encodersT;
    Vector commandT;
    Vector tmpT;
    encodersT.resize(njT);
    tmpT.resize(njT);
    commandT.resize(njT);

    for (i = 0; i < njT; i++) {
         tmpT[i] = 60.0;
    }
    posT->setRefAccelerations(tmpT.data());


     for (i = 0; i < njT; i++) {
        tmpT[i] = 30.0;
        posT->setRefSpeed(i, tmpT[i]);
    }

    commandT=0;
    commandT[0]=(-1*ang3);
    commandT[1]=(1*ang2);
    commandT[2]=(1*ang1);

    posT->positionMove(commandT.data());


    bool doneT=false;
    while(!doneT)
        {
            posT->checkMotionDone(&doneT);
            Time::delay(0.1);
        }

    robotDeviceT.close();

    //return 0;

 };

 void PMPThread::MessagePassR()
 {

    Bottle& outBot1 = cmdRight_armPort.prepare();   // Get the object
    outBot1.clear();
    outBot1.addString("set"); // put "set" command in the bottle
    outBot1.addString("poss"); // put "pos" command in the bottle
    Bottle& listBot = outBot1.addList();
    listBot.addDouble(ang4);
    listBot.addDouble(ang5+3);
    listBot.addDouble(ang6);
    listBot.addDouble(ang7);
    listBot.addDouble(ang8);
    listBot.addDouble(ang9);
    listBot.addDouble(ang10);
    listBot.addDouble(angCup);
    listBot.addDouble(angT1);
    listBot.addDouble(angT2);
    listBot.addDouble(angT3);
    listBot.addDouble(angI1);
    listBot.addDouble(angI2);
    listBot.addDouble(angM1);
    listBot.addDouble(angM2);
    listBot.addDouble(angRP);
    printf("Writing bottle right arm (%s)\n",outBot1.toString().c_str());
    cmdRight_armPort.write();
    Time::delay(5);
};

void PMPThread::MessagePassL()
 {

     Bottle& outBot2 = cmdLeft_armPort.prepare();   // Get the object
     outBot2.clear();
     outBot2.addString("set"); // put "set" command in the bottle
     outBot2.addString("poss"); // put "pos" command in the bottle
     Bottle& listBot1 = outBot2.addList();
     listBot1.addDouble(ang4L);
     listBot1.addDouble(ang5L);
     listBot1.addDouble(ang6L);
     listBot1.addDouble(ang7L);
     listBot1.addDouble(ang8L);
     listBot1.addDouble(ang9L);
     listBot1.addDouble(ang10L);
     listBot1.addDouble(angCupL);
     listBot1.addDouble(angTL1);
     listBot1.addDouble(angTL2);
     listBot1.addDouble(angTL3);
     listBot1.addDouble(angIL1);
     listBot1.addDouble(angIL2);
     listBot1.addDouble(angML1);
     listBot1.addDouble(angML2);
     listBot1.addDouble(angRPL);
     printf("Writing bottle left arm (%s)\n",outBot2.toString().c_str());
     cmdLeft_armPort.write();
     Time::delay(5);

 };

void PMPThread::MessagePassT()
 {

    Bottle& outBot3 = cmdTorsoPort.prepare();   // Get the object
    outBot3.clear();
    outBot3.addString("set"); // put "set" command in the bottle
    outBot3.addString("poss"); // put "pos" command in the bottle
    Bottle& listBot2 = outBot3.addList();
    listBot2.addDouble(ang1);
    listBot2.addDouble(1*ang2);
    listBot2.addDouble(1*ang3);

    printf("\n\n Writing bottle torso (%s)\n",outBot3.toString().c_str());
    cmdTorsoPort.write();                       // Now send i
    Time::delay(5);
  };




 void PMPThread::cmdInterfacePassR()
 {

    Bottle& outBot = cmdInterfacePort.prepare();   // Get the object
    outBot.clear();
    outBot.addVocab(CMD_RIGHT_ARM); // put "pos" command in the bottle
    Bottle& listBot = outBot.addList();
    listBot.addDouble(ang4);
    listBot.addDouble(ang5); //ang5+3
    listBot.addDouble(ang6);
    listBot.addDouble(ang7);
    listBot.addDouble(ang8);
    listBot.addDouble(ang9);
    listBot.addDouble(ang10);
    printf("Sending bottle right arm (%s)\n",outBot.toString().c_str());
    cmdInterfacePort.writeStrict();
    //Time::delay(2);
};

void PMPThread::cmdInterfacePassL()
 {

   Bottle& outBot = cmdInterfacePort.prepare();   // Get the object
   outBot.clear();
   outBot.addVocab(CMD_LEFT_ARM); // put "pos" command in the bottle
   Bottle& listBot1 = outBot.addList();
   listBot1.addDouble(ang4L);
   listBot1.addDouble(ang5L);
   listBot1.addDouble(ang6L);
   listBot1.addDouble(ang7L);
   listBot1.addDouble(ang8L);
   listBot1.addDouble(ang9L);
   listBot1.addDouble(ang10L);
   printf("Sending bottle left arm (%s)\n",outBot.toString().c_str());
   cmdInterfacePort.writeStrict();
   //Time::delay(2);
 };

void PMPThread::cmdInterfacePassT(){
    Bottle& outBot = cmdInterfacePort.prepare();   // Get the object
    outBot.clear();
    outBot.addVocab(CMD_TORSO); // put "pos" command in the bottle
    Bottle& listBot2 = outBot.addList();
	if(BodyChain==BodyTorsoArm_LEFT)
	{
		listBot2.addDouble(-ang1);
		listBot2.addDouble(-ang2);
	}
	else 
	{
		listBot2.addDouble(ang1);
		listBot2.addDouble(ang2);
	}
    listBot2.addDouble(ang3);

    printf("\n\nSending bottle torso (%s)\n",outBot.toString().c_str());
    cmdInterfacePort.writeStrict();                       // Now send i
    //Time::delay(2);
}


void PMPThread::cmdInterfacePassRhand()
 {

    Bottle& outBot = cmdInterfacePort.prepare();   // Get the object
    outBot.clear();
    outBot.addVocab(CMD_RIGHT_HAND); // put "pos" command in the bottle
    Bottle& listBot = outBot.addList();
    listBot.addDouble(angCup);
    listBot.addDouble(angT1);
    listBot.addDouble(angT2);
    listBot.addDouble(angT3);
    listBot.addDouble(angI1);
    listBot.addDouble(angI2);
    listBot.addDouble(angM1);
    listBot.addDouble(angM2);
    listBot.addDouble(angRP);
    printf("Sending bottle right hand (%s)\n",outBot.toString().c_str());
    cmdInterfacePort.writeStrict();
    //Time::delay(2);
};


void PMPThread::cmdInterfacePassLhand()
 {

   Bottle& outBot = cmdInterfacePort.prepare();   // Get the object
   outBot.clear();
   outBot.addVocab(CMD_LEFT_HAND); // put "pos" command in the bottle
   Bottle& listBot = outBot.addList();
   listBot.addDouble(angCupL);
   listBot.addDouble(angTL1);
   listBot.addDouble(angTL2);
   listBot.addDouble(angTL3);
   listBot.addDouble(angIL1);
   listBot.addDouble(angIL2);
   listBot.addDouble(angML1);
   listBot.addDouble(angML2);
   listBot.addDouble(angRPL);
   printf("Sending bottle left hand (%s)\n",outBot.toString().c_str());
   cmdInterfacePort.writeStrict();
   //Time::delay(2);
    // Sleep(3000);
 };

void PMPThread::gazeControl(double x, double y, double z)
 {

    Bottle& gazeTarget = gazePort.prepare();   // Get the object
    gazeTarget.clear();
    gazeTarget.addDouble(x/1000);
    gazeTarget.addDouble(y/1000);
    gazeTarget.addDouble(z/1000);
    printf("\n\n Writing bottle to gaze Control (%s)\n",gazeTarget.toString().c_str());
    gazePort.write();                      
    Time::delay(4);
    //Sleep(1000);
  };




void PMPThread::initiCubUp(double wrio) {
 	ang1   = 0;
 	ang2   = 0;
 	ang3   = 0;

	ang4   = -52;
 	ang5   = 64;
 	ang6   = 14;
 	ang7   = 40;
 	ang8   = wrio;
 	ang9   = 0;
 	ang10  = 0;
	
	ang4L=-52;
 	ang5L=64;
 	ang6L=14;
 	ang7L=40;
 	ang8L=wrio;
 	ang9L=0;
 	ang10L=0;
 	//angCupL=48;

 	/*ang4   = -75;
 	ang5   = 77;
 	ang6   = 14;
 	ang7   = 53;
 	ang8   = 76;
 	ang9   = 0;
 	ang10  = 0;*/
 	/*angCup=36;
    angT1=0;
    angT2=9;
    angT3=0;
    angI1=0;
    angI2=0;
    angM1=0;
    angM2=0;
    angRP=0;*/

 	/*angT1=-15;
 	angT2=0;
 	angT3=0;
 	angI1=0;
 	angI2=0;
 	angM1=0;
	angM2=0;
 	angRP=0;*/

 	/*ang4L=-90;
 	ang5L=45;
 	ang6L=0;
 	ang7L=70;
 	ang8L=0;
 	ang9L=0;
 	ang10L=0;
 	angCupL=48;*/

 	/*angTL1=-15;
 	angTL2=0;
 	angTL3=0;
 	angIL1=0;
 	angIL2=0;
 	angML1=0;
 	angML2=0;
 	angRPL=0;*/
};

void PMPThread::CubGrazp3(){
    angCup=36;
    angT1=77;
    angT2=14;
    angT3=0;
    angI1=76;
    angI2=55;
    angM1=76;
    angM2=55;
    angRP=2;
};

void PMPThread::CubGrazp4(){
    /* angCup=51;
       angT1=90;
       angT2=34;
       angT3=25;
       angI1=67;
       angI2=27;
       angM1=50;
       angM2=25;
       angRP=2;*/

    angCup=51;
    angT1=90;
    angT2=54;
    angT3=55;
    angI1=77;
    angI2=67;
    angM1=60;
    angM2=55;
    angRP=2;
};

void PMPThread::CubGrazpL3(){
    angCupL=48;
    angTL1=70;
    angTL2=20;
    angTL3=0;
    angIL1=5;
    angIL2=0;
    angML1=10;
    angML2=0;
    angRPL=2;
};

void PMPThread::CubGrazpL4(){
    /*angCupL=48;
      angTL1=87;
      angTL2=64;
      angTL3=55;
      angIL1=67;
      angIL2=57;
      angML1=50;
      angML2=35;
      angRPL=0; */

    angCupL=48;
    angTL1=87;
    angTL2=64;
    angTL3=55;
    angIL1=77;
    angIL2=67;
    angML1=60;
    angML2=55;
    angRPL=0;
};

void PMPThread::CubGrazp1(){
    /*ang4=ang4;
      ang5=ang5;
      ang6=ang6;
      ang7=ang7;
      ang8=ang8;
      ang9=ang9;
      ang10=ang10;*/
    //ang9=0;
    //ang8=0;
    angCup=36;
    angT1=77;
    angT2=0;
    angT3=0;
    angI1=11;
    angI2=0;
    angM1=14;
    angM2=0;
    angRP=2;

    /*ang4L=ang4;
      ang5L=ang5;
      ang6L=ang6;
      ang7L=ang7;
      ang8L=ang8;
      ang9L=ang9;
      ang10L=ang10;
      ang8L=0;
      angCupL=36;
      angTL1=90;
      angTL2=37;
      angTL3=10;
      angIL1=33;
      angIL2=0;
      angML1=33;
      angML2=0;
      angRPL=0;*/
    //ang9L=0;

    printf("Angles of Solution \n  %f, \t  %f, \t %f ,\t %f ,\t %f ,\t %f, \t  %f, \t %f ,\t %f ,\t %f",ang4,ang5, ang6,ang7,ang8,ang9,ang10,angT1,angI1,angM1);


};

void PMPThread::CubGrazp2()
 {
     angCup=36;
    angT1=77;
    angT2=0;
    angT3=0;
    angI1=76;
    angI2=0;
    angM1=76;
    angM2=0;
    angRP=2;
    /* angCup=36;
     angT1=90;
     angT2=44;
     angT3=25;
     angI1=67;
     angI2=23;
     angM1=60;
     angM2=25;
     angRP=2;*/

     /*ang4L=ang4;
       ang5L=ang5;
       ang6L=ang6;
       ang7L=ang7;
       ang8L=ang8;
       ang9L=ang9;
       ang10L=ang10;
       //ang9L=0;
       angCupL=36;
       angTL1=90;
       angTL2=45;
       angTL3=20;
       angIL1=67;
       angIL2=9;
       angML1=67;
       angML2=0;
       angRPL=0;*/

     printf("Angles of Solution \n  %f, \t  %f, \t %f ,\t %f ,\t %f ,\t %f, \t  %f, \t %f ,\t %f ,\t %f",ang4,ang5, ang6,ang7,ang8,ang9,ang10,angT1,angI1,angM1);

 };


void PMPThread::CubUp(int HandUp) {
  if(HandUp==0)
      {
          ang4=ang4+15;
          angCup=51;
          angT1=90;
          angT2=34;
          angT3=25;
          angI1=67;
          angI2=23;
          angM1=50;
          angM2=25;
          angRP=2;
      }
  if(HandUp==1)
      {
         ang4L=ang4L+15;
         angCupL=48;
         angTL1=87;
         angTL2=40;
         angTL3=25;
         angIL1=67;
         angIL2=23;
         angML1=60;
         angML2=25;
         angRPL=0;
      }
  printf("Angles of Solution \n  %f, \t  %f, \t %f ,\t %f ,\t %f ,\t %f, \t  %f, \t %f ,\t %f ,\t %f",ang4,ang5, ang6,ang7,ang8,ang9,ang10,angT1,angI1,angM1);

 };


void PMPThread::CubRelease() {
    /*ang4=ang4;
      ang5=ang5;
      ang6=ang6;
      ang7=ang7;
      ang8=ang8;
      ang9=ang9;
      ang10=ang10;*/
    //ang9=0;
    angCup=0;
    angT1=0;
    angT2=0;
    angT3=0;
    angI1=0;
    angI2=0;
    angM1=0;
    angM2=0;
    angRP=0;

    /*ang4L=ang4;
      ang5L=ang5;
      ang6L=ang6;
      ang7L=ang7;
      ang8L=ang8;
      ang9L=ang9;
      ang10L=ang10;*/
    //ang9L=0;
    angCupL=0;
    angTL1=0;
    angTL2=0;
    angTL3=0;
    angIL1=0;
    angIL2=0;
    angML1=0;
    angML2=0;
    angRPL=0;

};

void PMPThread::CubReleaseSoft() {
    /*ang4=ang4;
      ang5=ang5;
      ang6=ang6;
      ang7=ang7;
      ang8=ang8;
      ang9=ang9;
      ang10=ang10;*/
    //ang9=0;
    angCup=51;
    angT1=90;
    angT2=15;
    angT3=5;
    angI1=30;
    angI2=10;
    angM1=10;
    angM2=0;
    angRP=2;

    /*ang4L=ang4;
      ang5L=ang5;
      ang6L=ang6;
      ang7L=ang7;
      ang8L=ang8;
      ang9L=ang9;
      ang10L=ang10;*/
    //ang9L=0;
    angCupL=51;
    angTL1=90;
    angTL2=15;
    angTL3=5;
    angIL1=30;
    angIL2=10;
    angML1=10;
    angML2=0;
    angRPL=2;

};

void PMPThread::Grab(){
    ang4=0;
    ang5=90;
    ang6=0;
    ang7=0;
    ang8=0;
    ang9=0;
    ang10=0;
    angCup=0;
    angT1=30;
    angT2=20;
    angT3=20;
    angI1=20;
    angI2=20;
    angM1=20;
    angM2=20;
    angRP=20;

    ang4L=-38;
    ang5L=-8;
    ang6L=0;
    ang7L=21;
    ang8L=120;
    ang9L=5;
    ang10L=-40;
    angCupL=0;
    angTL1=90;
    angTL2=90;
    angTL3=90;
    angIL1=90;
    angIL2=90;
    angML1=90;
    angML2=90;
    angRPL=90;
};

void PMPThread::Kompliance(int TagK)	{
 	// Close Space********************************** (0,-230,590)
    if (TagK==0) {
        /*t_dur=5;
       	printf ("Adjusting Compliances 0 \n");
       	KFORCE        = 0.006;
       	ITERATION     = 1000;
        RAMP_KONSTANT = 0.005;
        KOMP_JANG     = 0.006;
        // low values of KOMP (admittance) of the torso reduces the contribution of the torso
        KOMP_WAISZT   = 0.000003;
        KOMP_WAISZT2  = 0.000003;

        J0H = 0.041;
        J2H = 0.000005;
        J7H = 0.041;
        J8H = 0.041;
        J9H = 0.041;
        //J7H=10;
        //J8H=10;
        //J9H=10;

        printf ("\n Initiating System Dynamics \n");*/
		//COPIED FROM tagK==1 change in ITERATION only
		printf ("Adjusting Compliances 0 \n");
        KFORCE=0.094;//0.06;//0.005 // 0.0094
		ITERATION=4000;
		RAMP_KONSTANT=0.0015;
		t_dur=5;
		KOMP_JANG=0.0009;
		KOMP_WAISZT=0.0005;
		KOMP_WAISZT1=0.0001;
		//KOMP_WAISZT3=0.00002
		KOMP_WAISZT2=0.0009;
		 //KOMP_WAISZT   = 0.000003;
        //KOMP_WAISZT2  = 0.000003;
		J0H=800;//200; //400 //800
        J1H=0.52;
		J2H=400; //1 //400;
		J3H=20; //4.5 //1 //20;
        J4H=1; //4.5
        J5H=1;//200; //1
        J6H=0.41;//200; //0.041;
		J7H=1;//400; //1
		J8H=500; //1
		J9H=500; //1
        // Far Space *****************************
		//printf ("\n Initiating System Dynamics \n");

    }


// Close Space ends **********************

// Far Space *****************************
     if (TagK==1)
	      {
        printf ("Adjusting Compliances 1 \n");
        KFORCE=0.0094;//0.06;//0.005 // 0.0094
		ITERATION=1000;
		RAMP_KONSTANT=0.005;
		t_dur=5;
		KOMP_JANG=0.0009;
		KOMP_WAISZT=0.001;//0.0001;
		KOMP_WAISZT1=0;
		//KOMP_WAISZT3=0.00002
		KOMP_WAISZT2=0.0009;//0.0009;
		J0H=1;//200; //400 //800
        J1H=0.52;
		J2H=400; //1 //400;
		J3H=20; //4.5 //1 //20;
        J4H=1; //4.5
        J5H=50;//200; //1
        J6H=50;//200; //0.041;
		J7H=1;//400; //1
		J8H=500; //1
		J9H=500; //1
        // Far Space *****************************
		//printf ("\n Initiating System Dynamics \n");
	 }


    if (TagK==2)  {
        // toy crane experimental set-up
        printf ("Adjusting Compliances 2 \n");
        KFORCE=0.009; //0.005
        ITERATION=1000;
        RAMP_KONSTANT=0.0015;
        t_dur=3;
        KOMP_JANG=0.002;
        KOMP_WAISZT=0.0001;
        //KOMP_WAISZT3=0.00002;
        KOMP_WAISZT2=0.0006;

        J0H=50; //800
        J2H=0.2;
        J7H=50;
        J8H=0.041;
        J9H=0.041;

        // Far Space *****************************
        printf ("\n Initiating System Dynamics \n");
    }

    if (TagK==3)  {
        t_dur=5;
        printf ("Adjusting Compliances 3 \n");
        KFORCE=0.01; //0.005
        ITERATION=1000;
        RAMP_KONSTANT=0.005;

        KOMP_JANG=0.002;
        KOMP_WAISZT=0.0001;
        //KOMP_WAISZT3=0.00002;
        KOMP_WAISZT2=0.0011;

        J0H=50; //800
        J2H=0.2;
        J7H=50;
        J8H=0.041;
        J9H=0.041;

        // Far Space *****************************
        printf ("\n Initiating System Dynamics \n");
    }

};


