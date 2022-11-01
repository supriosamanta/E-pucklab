#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "ch.h"
#include "hal.h"
#include "selector.h"
#include "memory_protection.h"
#include <main.h>
#include <stdbool.h>
#include "leds.h"
#include "spi_comm.h"
#include "sensors/proximity.h"
#include "epuck1x/uart/e_uart_char.h"
#include "stdio.h"
#include "serial_comm.h"
#include "motors.h"
#include "sensors/VL53L0X/VL53L0X.h"

int motor_speed = 500;
int motor_speed_r = -500;
int dirArr[8];
int orArr[8];
_Bool BB_flag;
_Bool OR_flag;

int sw = 0;

int F_flag = 0;
int R_flag = 0;
int L_flag = 0;
int FO_flag = 0;
int RO_flag = 0;
int LO_flag = 0;
int n=0;
int dist = 0;

void move_forward(void){
	left_motor_set_speed(motor_speed);
	right_motor_set_speed(motor_speed);
}

void move_back(void){
	left_motor_set_speed(motor_speed_r);
	right_motor_set_speed(motor_speed_r);
}

void stop(void){
	left_motor_set_speed(0);
	right_motor_set_speed(0);
}

void turn_left(void){
	left_motor_set_speed(motor_speed_r);
	right_motor_set_speed(motor_speed);
}

void move_left(void){
	left_motor_set_speed(motor_speed_r/2);
	right_motor_set_speed(motor_speed);
}

void turn_right(void){
	left_motor_set_speed(motor_speed);
	right_motor_set_speed(motor_speed_r);
}

void move_right(void){
	left_motor_set_speed(motor_speed);
	right_motor_set_speed(motor_speed_r/2);
}

void ran_number(void)   //function to generate a random number in the range of 100 to 500
{
    int lower = 100, upper = 500;
    n = (rand() %(upper - lower + 1)) + lower;
}

int *readBB(int prox[8]){
	for(int j = 0; j < 8 ; j++){
		if (prox[j] > 200){
			dirArr[j] = 1;  //object is close to the e-puck robot
		    set_rgb_led( j+1, 10, 0, 0);
		}
		else{
			dirArr[j] = 0;  //object is far from the e-puck robot
		    set_rgb_led( j+1, 0, 0, 0);
		}
		F_flag = dirArr[0] + dirArr[7];
		R_flag = dirArr[1] + dirArr[2];
		L_flag = dirArr[6] + dirArr[5];
	}
	return dirArr;
}

int *readOR(int prox[8]){
	for(int j = 0; j < 8 ; j++){
		if (prox[j] < 200){
			orArr[j] = 1;  //object is close to the e-puck robot
		    set_rgb_led( j+1, 0, 0, 10);
		}
		else{
			orArr[j] = 0;  //object is far from the e-puck robot
		    set_rgb_led( j+1, 0, 0, 0);
		}
		FO_flag = orArr[0] + orArr[7];
		RO_flag = orArr[1] + orArr[2] + orArr[3];
		LO_flag = orArr[6] + orArr[5] + orArr[4];
	}
	return orArr;
}

void set_fire(void){
	set_led(2, 1);
	set_led(3, 1);
	set_led(4, 1);
	set_led(6, 1);
	set_led(7, 1);
	set_led(8, 1);
}

void BB_contains(void){
    for (int i = 0; i < 8; i++) {
        if (dirArr[i] == 1) {
        	BB_flag = true;
            break;
        }
        else{
        	BB_flag = false;
        	break;
        }
    }
}

void OR_contains(void){
    for (int i = 0; i < 8; i++) {
        if (orArr[i] == 1) {
        	OR_flag = true;
            break;
        }
        else{
        	OR_flag = false;
        	break;
        }
    }
}
void mode_switch(void){
	if(OR_flag){sw = 1;}
	else if(BB_flag){sw = 2;}
	else{sw = 0;}
}

messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);


int main(void)
{
	messagebus_init(&bus, &bus_lock, &bus_condvar);
    halInit();
    chSysInit();
    mpu_init();
    VL53L0X_start();
    clear_leds();
    spi_comm_start();
    motors_init();
    proximity_start();
    calibrate_ir();

    serial_start();

    int prox[8];
    int *dir;
    int dist = 0;

    /* Infinite loop. */
    while (1) {
    	for (int i = 0; i < 8; i++){prox[i] = get_prox(i);} // Read proximity sensor values
    	dir = readBB(prox);
    	for(int k=0;k<8;k++){set_led( k+1 , *(dir+k));}  // Set LED values
    	set_fire();
    	dist = VL53L0X_get_dist_mm();
		int sw_val = get_selector();
    	switch(sw_val) {
    	    case 0: // Avoider
        	    set_body_led(1);
    	    		if(dir[0]==0 && dir[7]==0){     //we already have a condition to check any obstacle ahead(readBB), why do we need d>30 condition??
    	    			move_forward();//Move forward
    	    		}
    	    		else if(dir[0]>0 && dir[7]==0) //if there is an object at front right side but nothing on the front left
                    {
                        turn_left();
                    }
                    else if(dir[7]>0 && dir[0]==0) //if there is an object at the front left side but nothing on the front right
                    {
                        turn_right();
                    }
    	    		else if(dir[0]==1 && dir[7]==1) //if there is a wall in front of the robot
                    {
                        turn_right();
                    }
                    else if(L_flag>0 && F_flag>0) //if the robot is stuck in a corner(left)
                    {
                         while(dir[7]>0)
                        {
                        turn_right();
                        }

                    }
                    else if(R_flag>0 && F_flag>0) //if the robot is stuck in a corner(right)
                    {
                        while(dir[0]>0) //turn left till the object is in the front i.e facing sensor 0
                        {
                        turn_left();
                        }
                    }
    	    		else{
    	    			if(R_flag>0 && L_flag==0){ // if obstacle on right
    	    				turn_left();
    	    			}
    	    			else if(L_flag>0 && R_flag==0){turn_right();}// if obstacle on left
    	    			else if(L_flag>0 && R_flag>0 && F_flag > 0){
    	    				move_back();
    	    				chThdSleepMilliseconds(100);
    	    				ran_number();               //random speed to turn, so that we get an angled turn
                            left_motor_set_speed(n);
                            right_motor_set_speed(-n);
    	    			}
    	    				//wait for a full turn
    	    			else{
    	    				ran_number();               // I am using a random speed to turn it.
                            left_motor_set_speed(-n);
                            right_motor_set_speed(n);
    	    			}
    	    			}
    	    		//break;
    	    	case 9: // pursuer///////////////////////////////////////////////////////////////////////////////////////

	    			if(OR_flag && dist>30){move_forward();}
	    			else if(LO_flag>0 && RO_flag==0){turn_left();}
	    			else if(RO_flag>0 && LO_flag==0){turn_right();}
	    			else if(R_flag>0 && L_flag==0){move_left();}
	    			else if(L_flag>0 && R_flag==0){move_right();}
	    			else if(dist<30){move_back();}
	    			else if(F_flag != 0){stop();}

    	    default :
    	    	break;
    	}
    }
    chThdSleepMilliseconds(100);
}


#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
