#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include <main.h>
#include "leds.h"
#include "spi_comm.h"
#include "sensors/proximity.h"
#include "epuck1x/uart/e_uart_char.h"
#include "stdio.h"
#include "serial_comm.h"
#include "motors.h"

//int dirArr[8];

int *readBB(int prox[8]){
	// color the LEDs red
	int *r;
	static int dirArr[8];
	for(int j = 0; j < 8 ; j++){
		if (prox[j] > 500)
			dirArr[j] = 1;
		else
			dirArr[j] = 0;
		p = dirArr[0];
	}
	return r;
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

    clear_leds();
    spi_comm_start();
    motors_init();

    proximity_start();
    calibrate_ir();
    serial_start();
    //int x=0;
    int prox[8];
    int *dir;

    //char str[100];

    /* Infinite loop. */
    while (1) {
    	//waits 1 second
    	for (int i = 0; i < 8; i++){
    		prox[i] = get_prox(i);
    	}

    	//int str_length = sprintf(str, "Proximity sensor value %d!\n",prox);
    	//e_send_uart1_char(str, str_length);
    	dir = readBB(prox);

    	/*if (x>500)
		{

    		set_front_led(1);
    		left_motor_set_speed(10);
    		right_motor_set_speed(-10);

		}
    	else
    	{
    		set_front_led(0);
    		left_motor_set_speed(0);
    		right_motor_set_speed(0);
    	}
    	//set_front_led(1);
        //chThdSleepMilliseconds(5000);
        //set_front_led(0);
        //chThdSleepMilliseconds(5000);*/
    	for(int k=0;k<8;k++)
    	{
    		set_led( k , *(dir+k));
    	}

    }
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
