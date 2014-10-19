#include <stdio.h>
#include <ncurses.h>
#include "h/i2c-utils.h"
#include "h/mpu6050.h"
#include "h/mpu6050_const.h"
#include <time.h>
#include <signal.h>
#include <stdlib.h>

int dev=0;

/*terminal interrupt handler*/
void ctrlcHandler(int s){
	stopMpu(dev);
	close(dev);
	endwin();
	exit(0);
}

void main(){
	/*set handler*/
	struct sigaction sig;
	sig.sa_handler=ctrlcHandler;
	sigaction(SIGINT,&sig,NULL);

	/*300ms*/
	struct timespec wait;
	wait.tv_sec=(long)0;
	wait.tv_nsec=(long)300000000;
	int ch;
	unsigned char buf[6]={0};
	double val[4]={0};
	unsigned int i=0;
	double temp, ax, ay, az, gx, gy, gz;
	
	char *device="/dev/i2c-1";
/*	int dev=0;*/
	unsigned char addr=0x68;
	
	unsigned char fifoBuf[1024]={0};
	/*open device*/	
	dev=i2cinit(device,addr);

/*start mpu6050*/
	startMpu(dev);

/*ready interrupt status*/
	i=i2cRead(dev,INT_STATUS);
		while(1){
			if(i&INT_DATA_READY){
				printf("data ready... %x\n",i);
				i2cReadN(dev,buf,TEMP_OUT_H,2);
				temp=getTemp(buf);
				printf("temp: %.3f\n",temp);
				i2cReadN(dev,buf,ACCEL_XOUT_H,6);
				buildAGdata(dev,buf,0,val);
				printf("accel: %.3f  %.3f  %.3f\n",val[0],val[1],val[2]);
				printf("accel: %x  %x  %x\n",buf[0],buf[1],buf[2]);

				i2cReadN(dev,buf,GYRO_XOUT_H,6);
				buildAGdata(dev,buf,1,val);
				printf("gyro: %.3f  %.3f  %.3f\n",val[0],val[1],val[2]);
				printf("gyro: %x  %x  %x\n",buf[0],buf[1],buf[2]);
				printf("--------------------------\n");
				char ch;
				scanf("%c",&ch);
				printf("pressed: %c\n",ch);
				switch(ch){
					case 'c':
					printf("calibrate\n");
					double av[7];
					averageFifo(dev,av,1000);
					setOffsets(0,av);
					setOffsets(1,&av[3]);
/*					setAccelOffset(dev,av);*/
/*					setGyroOffset(dev,&av[3]);*/
					/*printf("%f %f %f %f %f %f %f\n",av[0],av[1],av[2],av[3],av[4],av[5],av[6]);*/
						break;
					case 'r':
					printf("reset\n");
						stopMpu(dev);
						startMpu(dev);
						break;
					default:
						break;
				}

			}
			else{
				printf("not data ready... %x\n",i);
			}
			i=i2cRead(dev,INT_STATUS);
		}
}
