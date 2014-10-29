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
wait.tv_nsec=(long)200000000;
int ch;

	initscr();
	cbreak();
	noecho();
	timeout(0);
	mvaddstr(2,16,"X");
	mvaddstr(2,26,"Y");
	mvaddstr(2,36,"Z");
	mvaddstr(0,0,"Temperatura:");
	mvaddstr(1,0,"------------------------------------------");
	mvaddstr(3,0,"Accelerometro:");
	mvaddstr(4,0,"Giroscopio:");
	mvaddstr(5,0,"Type 'c' to update offset registers:");
	mvaddstr(6,0,"Type 'r' to reset the device:");
	mvaddstr(7,0,"Type 'f' to set a low pass filter:");
	mvaddstr(8,0,"Type 's' to set the output data rate:");
	refresh();
	
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

/*ready interrupt status*/
	i=i2cRead(dev,INT_STATUS);
	while(1){
	
	if(i&INT_DATA_READY){
		i2cReadN(dev,buf,TEMP_OUT_H,2);
		temp=getTemp(buf);
		i2cReadN(dev,buf,ACCEL_XOUT_H,6);
		buildAGdata(dev,buf,0,val);
		ax=val[0];
		ay=val[1];
		az=val[2];
		i2cReadN(dev,buf,GYRO_XOUT_H,6);
		buildAGdata(dev,buf,1,val);
		gx=val[0];
		gy=val[1];
		gz=val[2];
		mvprintw(0,16,"%.3f",temp);
		move(3,16);
		clrtoeol();
		mvprintw(3,16,"%.3f",ax);
		mvprintw(3,26,"%.3f",ay);
		mvprintw(3,36,"%.3f",az);
		move(4,16);
		clrtoeol();
		mvprintw(4,16,"%.3f",gx);
		mvprintw(4,26,"%.3f",gy);
		mvprintw(4,36,"%.3f",gz);
		move(9,0);
		ch=getch();
		switch(ch){
				double av[7]={0};
			case 'c':
				averageFifo(dev,av,1000);
				setOffsets(0,av);
				setOffsets(1,&av[3]);
/*				setAccelOffset(dev,av);*/
/*				setGyroOffset(dev,&av[3]);*/
				break;
			case 'r':
				stopMpu(dev);
				startMpu(dev);
				break;
			case 's':
				timeout(-1);
				printw("type a value in HZ and press ENTER\n");
				refresh();
				char str[]={0};
				echo();
				getstr(str);
				setSampleRateDiv(dev,atoi((const char*)str));
				move(9,0);
				timeout(0);
				noecho();
				break;
			case 'f':
				timeout(-1);
				printw("%s\n",DLPF_TABLE);
				printw("select a value between 0 and 6:\n");
				refresh();
				ch=getch();
				setDLPF(dev,atoi((const char*)&ch));
				move(9,0);
				timeout(0);
				break;
			default:
				break;
		}
		clrtobot();
		refresh();
	}
		nanosleep(&wait,NULL);
		i=i2cRead(dev,INT_STATUS);
	}
	endwin();
	exit(0);

}
