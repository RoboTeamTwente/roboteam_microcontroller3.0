/*
 * vel_control.c
 *
 *  Created on: Jan 23, 2019
 *      Author: simen
 */

#include "vel_control.h"
#include "stdbool.h"

///////////////////////////////////////////////////// DEFINITIONS



///////////////////////////////////////////////////// VARIABLES

static PID_states velc_state = off;
static PIDvariables velK[3];
static float ref[3] = {0.0f};
static float wheel_ref[4] = {0.0f};
static float state[3] = {0.0f};

///////////////////////////////////////////////////// PRIVATE FUNCTION DECLARATIONS

//transfer body velocity to necessary wheel speed
static void body2Wheels(float wheelspeed[4], float velocity[3]);

//transfer global coordinate frame to local coordinate frame
static void global2Local(float input[3], float output[2], float  yaw);

void translationVelControl(float State[3], float vel_ref[3], bool use_global_ref, float translationalRef[4]);

float angleControl(float angleRef, float angle);

static void initPID(body_handles body, float kP, float kI, float kD);

///////////////////////////////////////////////////// PUBLIC FUNCTION IMPLEMENTATIONS

int vel_control_Init(){
	velc_state = on;
	initPID(body_x, 1.0, 0.0, 0.0);
	initPID(body_y, 2.0, 0.0, 0.0);
	initPID(body_w, 20.0, 1.5, 0.0);
	return 0;
}

int vel_control_DeInit(){
	velc_state = off;
	return 0;
}


void vel_control_Callback(){
	if (velc_state == on){
		float translationalRef[4] = {0.0f};
		translationVelControl(state, ref, translationalRef);

		float angularRef = angleControl(ref[body_w], state[body_w]);

		for (wheel_names wheel=wheels_RF; wheel<wheels_LF; wheel++){
			wheel_ref[wheel] = translationalRef[wheel] + angularRef;
		}
	}
}

void setRef(float input[3]){
	ref[body_x] = input[body_x];
	ref[body_y] = input[body_y];
	ref[body_w] = input[body_w];
}

void getwheelRef(float output[4]){
	for (wheel_names wheel=wheels_RF; wheel<wheels_LF; wheel++){
		output[wheel] = wheel_ref[wheel];
	}
}

void setState(float input[3]){
	state[body_x] = input[body_x];
	state[body_y] = input[body_y];
	state[body_w] = input[body_w];
}

///////////////////////////////////////////////////// PRIVATE FUNCTION IMPLEMENTATIONS

void translationVelControl(float State[3], float vel_ref[3], float translationalRef[4]){
	float velLocalRef[3] = {0, 0, 0};
	global2Local(vel_ref, velLocalRef, State[body_w]); //transfer global to local

	// Manually adjusting velocity command
	//     Explanation: see Velocity Difference file on drive (https://docs.google.com/document/d/1pGKysiwpu19DKLpAZ4GpluMV7UBhBQZ65YMTtI7bd_8/)
	velLocalRef[body_x] = 1.063 * velLocalRef[body_x];
	velLocalRef[body_y] = 1.308 * velLocalRef[body_y];

	// Local control
	float velxErr = (velLocalRef[body_x] - State[body_x]);
	float velyErr = (velLocalRef[body_y] - State[body_y]);
	velLocalRef[body_x] += PID(velxErr, velK[body_x]);
	velLocalRef[body_y] += PID(velyErr, velK[body_y]);

	body2Wheels(translationalRef, velLocalRef); //translate velocity to wheel speed
}

float angleControl(float angleRef, float angle){
	float angleErr = constrainAngle(angleRef - angle);//constrain it to one circle turn
	if (fabs(angleErr) < YAW_MARGIN) { // reset the I to zero everytime the target has been reached
		angleErr = 0;
		velK[body_w].I = 0;
	}
	return PID(angleErr, velK[body_w]);// PID control from control_util.h
}


static void body2Wheels(float wheelspeed[4], float velocity[3]){
	//mixing matrix
	float velx2wheel = (velocity[body_x]*sin60/rad_wheel);
	float vely2wheel = (velocity[body_y]*cos60/rad_wheel);
	//float rot2wheel =  (rad_robot*velocity[body_w]/rad_wheel);
	wheelspeed[wheels_RF] = (velx2wheel + vely2wheel);
	wheelspeed[wheels_RB] = (velx2wheel - vely2wheel);
	wheelspeed[wheels_LB] = (-velx2wheel - vely2wheel);
	wheelspeed[wheels_LF] = (-velx2wheel + vely2wheel);
}

static void global2Local(float global[3], float local[3], float  yaw){
	//trigonometry
	local[body_x] = cosf(yaw)*global[body_x]+sinf(yaw)*global[body_y];
	local[body_y] = -sinf(yaw)*global[body_x]+cosf(yaw)*global[body_y];
	local[body_w] = global[body_w];
}

static void initPID(body_handles body, float kP, float kI, float kD){
	velK[body] = PIDdefault;
	velK[body].kP = kP;
	velK[body].kI = kI;
	velK[body].kD = kD;
}

