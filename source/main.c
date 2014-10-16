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

/*	initscr();*/
/*	cbreak();*/
/*	noecho();*/
/*	timeout(0);*/
/*	mvaddstr(2,16,"X");*/
/*	mvaddstr(2,26,"Y");*/
/*	mvaddstr(2,36,"Z");*/
/*	mvaddstr(0,0,"Temperatura:");*/
/*	mvaddstr(1,0,"------------------------------------------");*/
/*	mvaddstr(3,0,"Accelerometro:");*/
/*	mvaddstr(4,0,"Giroscopio:");*/
/*	mvaddstr(5,0,"Type 'c' to update offset registers:");*/
/*	mvaddstr(6,0,"Type 'r' to reset the device:");*/
/*	refresh();*/
	
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
/*	i2cFuncs(dev);*/
/*start mpu6050*/
	startMpu(dev);
/*enable fifo*/
/*	i2cWrite(dev,USER_CTRL,0x40);*/
/*output of temperature,accelerometer and gyroscop to fifo*/
/*	i2cWrite(dev,FIFO_EN,0x78);*/
/*	nanosleep(&wait,NULL);*/
/*enable interrupt register*/
/*i2cWrite(dev,INT_ENABLE,INT_FIFO_OFLOW_EN|INT_DATA_READY_EN);*/
/*ready interrupt status*/
	i=i2cRead(dev,INT_STATUS);
/*	while(1){*/
/*		temp=getTemp(dev);*/
/*		i2cReadN(dev,buf,ACCEL_XOUT_H,6);*/
/*		buildAGdata(dev,buf,0,val);*/
/*		ax=val[0];*/
/*		ay=val[1];*/
/*		az=val[2];*/
/*		i2cReadN(dev,buf,GYRO_XOUT_H,6);*/
/*		buildAGdata(dev,buf,1,val);*/
/*		gx=val[0];*/
/*		gy=val[1];*/
/*		gz=val[2];*/
/*		mvprintw(0,16,"%.3f",temp);*/
/*		move(3,16);*/
/*		clrtoeol();*/
/*		mvprintw(3,16,"%.3f",ax);*/
/*		mvprintw(3,26,"%.3f",ay);*/
/*		mvprintw(3,36,"%.3f",az);*/
/*		move(4,16);*/
/*		clrtoeol();*/
/*		mvprintw(4,16,"%.3f",gx);*/
/*		mvprintw(4,26,"%.3f",gy);*/
/*		mvprintw(4,36,"%.3f",gz);*/
/*		move(7,0);*/
/*		ch=getch();*/
/*		switch(ch){*/
/*			case 'c':*/
/*				averageFullFifo(dev,FIFO_ACC_OUT,val);*/
/*				setAccelOffset(dev,val);*/
/*				averageFullFifo(dev,FIFO_GYRO_X_OUT|FIFO_GYRO_Y_OUT|FIFO_GYRO_Z_OUT,val);*/
/*				setGyroOffset(dev,val);*/
/*				setAccelOffset(dev,buf);*/
/*				setGyroOffset(dev,buf);*/
/*				break;*/
/*			case 'r':*/
/*				stopMpu(dev);*/
/*				startMpu(dev);*/
/*				break;*/
/*			default:*/
/*				break;*/
/*		}*/
/*		refresh();*/
/*		i=i2cRead(dev,INT_STATUS);*/
/*	}*/
/*	endwin();*/
/*	exit(0);*/
/*	while(nanosleep(&wait,NULL)==0){*/
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
/*				averageFullFifo(dev,FIFO_ACC_OUT,val);*/
/*				setAccelOffset(dev,val);*/
/*				averageFullFifo(dev,FIFO_GYRO_X_OUT|FIFO_GYRO_Y_OUT|FIFO_GYRO_Z_OUT,val);*/
/*				setGyroOffset(dev,val);*/
double av[7];
averageFifo(dev,av,1000);
setAccelOffset(dev,av);
setGyroOffset(dev,&av[3]);
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
/*			if(getFifoCount(dev)!=1024){*/
/*				printf("fifo count= %d\n",getFifoCount(dev));*/
/*				int c=getFifoData(dev,fifoBuf);*/
/*				printf("fifo packets= %f\n",(double)getFifoCount(dev)/14);*/
/*				int cc=0;*/
/*				for(cc;cc<c;cc++){*/
/*					printf("%x ",cc,fifoBuf[cc]);*/
/*				}*/
/*			}*/
		}
		else{
			printf("not data ready... %x\n",i);
		}
		i=i2cRead(dev,INT_STATUS);
	}
}
