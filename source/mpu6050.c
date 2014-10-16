#include <stdio.h>
#include <stdlib.h>
#include "h/i2c-utils.h"
#include "h/mpu6050.h"
#include "h/mpu6050_const.h"
#include <time.h>

static double sub1off[6];

inline double getSub1off(int ag, double *data){
	data[0]-=sub1off[ag*3];
	data[0]-=sub1off[(ag*3)+1];
	data[0]-=sub1off[(ag*3)+2];
}

void startMpu(int dev){
/*	unsigned char buf[2]={0};*/
/*power on*/
	i2cWrite(dev,PWR_MGMT_1,0x00);
/*set sample rate at 50Hz*/
	i2cWrite(dev,SMPLRT_DIV,0x4f);
/*enable fifo*/
	i2cWrite(dev,USER_CTRL,0x40);
/*output of temperature,accelerometer and gyroscop to fifo*/
	i2cWrite(dev,FIFO_EN,0x78);
/*enable interrupt register*/
	i2cWrite(dev,INT_ENABLE,INT_FIFO_OFLOW_EN|INT_DATA_READY_EN);
/*wait 100ms*/
	struct timespec wait;
	wait.tv_sec=(long)0;
	wait.tv_nsec=(long)100000000;
	nanosleep(&wait,NULL);
}

void stopMpu(int dev){
/*	unsigned char buf[2]={0};*/
	i2cWrite(dev,PWR_MGMT_1,0x80);
}

int bWaitDataReady(int dev){

	unsigned char dr=i2cRead(dev,INT_STATUS)&0x01;
	while(!dr){;
/*		printf("waiting data ready %hhx\n",dr);*/
		dr=i2cRead(dev,INT_STATUS);
	}
/*	printf("leaving\n");*/
	return 1;
}

double getTemp(unsigned char *buf){
	short int raw=(buf[0]<<8)|(buf[1]);
	double temp=((double)raw/340)+36.53;
	return temp;
}

double getAccelFs(int dev){
	double fs=0.0;
	int cfg=i2cRead(dev,ACCEL_CONFIG);
	switch(cfg){
		case AMASK_16:
			fs=AFS_SEL_16;
			break;
		case AMASK_8:
			fs=AFS_SEL_8;
			break;
		case AMASK_4:
			fs=AFS_SEL_4;
			break;
		case AMASK_2:
			fs=AFS_SEL_2;
			break;
		default:
/*		ERROR*/
			printf("Error on accel cfg selection\n");
			printf("cfg %.1f %.1f\n",cfg,fs);
/*			exit(1);*/
			break;
	}
	return fs;
}

double getGyroFs(dev){
	double fs=0.0;
	int cfg=i2cRead(dev,GYRO_CONFIG);
	switch(cfg){
		case GMASK_2000:
			fs=FS_SEL_2000;
			break;
		case GMASK_1000:
			fs=FS_SEL_1000;
			break;
		case GMASK_500:
			fs=FS_SEL_500;
			break;
		case GMASK_250:
			fs=FS_SEL_250;
			break;
		default:
/*		ERROR*/
			printf("Error on accel cfg selection\n");
			printf("cfg %.1f %.1f\n",cfg,fs);
/*			exit(1);*/
			break;
	}
	return fs;
}

void buildAGdata(int dev, unsigned char *raw, int ag, double *data){

	double fs=0.0;
	short int rawx=(raw[0]<<8)|raw[1];
	short int rawy=(raw[2]<<8)|raw[3];
	short int rawz=(raw[4]<<8)|raw[5];
	if(!ag){
		fs=getAccelFs(dev);
	}
	else{
		fs=getGyroFs(dev);
	}

	data[0]=rawx/fs;
	data[1]=rawy/fs;
	data[2]=rawz/fs;
	data[3]=fs;
}

/*@param: device*/
/*@param: accelerometer values in current full scale range*/
void setAccelOffset(int dev, double *accel_val){

	unsigned char off[6]={0};
	i2cReadN(dev,off,XA_OFFS_USERH,6);
	/*get acceleration values*/

	double newx=accel_val[0]*AFS_SEL_8;
	double newy=accel_val[1]*AFS_SEL_8;
	double newz=accel_val[2]*AFS_SEL_8;
	
	/*get temperature compesation bit*/
	unsigned char tempComp[3]={0};
	unsigned char mask=0x01;
	tempComp[0]=off[1]&mask;
	tempComp[1]=off[3]&mask;
	tempComp[2]=off[5]&mask;

	/*build new offsets*/
	unsigned int xoff=(off[0]<<8)|off[1];
	unsigned int yoff=(off[2]<<8)|off[3];
	unsigned int zoff=(off[4]<<8)|off[5];
	
	unsigned char noff[6]={0};
	xoff-=newx;
	yoff-=newy;
	zoff-=newz;
	/*set temperature compensation*/

	noff[0]=(xoff>>8)&0xff;
	noff[1]=(xoff&0xff)|tempComp[0];	
	noff[2]=(yoff>>8)&0xff;
	noff[3]=(yoff&0xff)|tempComp[1];
	noff[4]=(zoff>>8)&0xff;
	noff[5]=(zoff&0xff)|tempComp[2];
	
	/*write new offset*/
	i2cWriteN(dev, XA_OFFS_USERH, noff, 6);
}

/*@param: device*/
/*@param: gyroscope values in current full scale range*/
void setGyroOffset(int dev, double *gyro_val){

	double newx=-(gyro_val[0]*FS_SEL_1000);
	double newy=-(gyro_val[1]*FS_SEL_1000);
	double newz=-(gyro_val[2]*FS_SEL_1000);

	unsigned char off[6]={0};

	off[0]=(((unsigned int)newx)>>8)&0xff;
	off[1]=((unsigned int)newx)&0xff;
	off[2]=(((unsigned int)newy)>>8)&0xff;
	off[3]=((unsigned int)newy)&0xff;
	off[4]=(((unsigned int)newz)>>8)&0xff;
	off[5]=((unsigned int)newz)&0xff;
	
	i2cWriteN(dev,XG_OFFS_USERH,off,6);
}

/**/
/*double averageFullFifo(int dev, unsigned char mask, double *av){*/
/*	int savef=i2cRead(dev,FIFO_EN);*/
/*	unsigned char saveu=(unsigned char)i2cRead(dev,USER_CTRL);*/
/*	unsigned char fifo[1024]={0};*/
/*	double data[4];*/
/*	double avx=0;*/
/*	double avy=0;*/
/*	double avz=0;	*/
/*	int i=0;*/
/*	int c=0;*/
/*	int ag=0;*/
/*	double fs=0.0;*/
/*	*/
/*	if (mask==FIFO_GYRO_X_OUT|FIFO_GYRO_Y_OUT|FIFO_GYRO_Z_OUT){*/
/*		i2cWrite(dev,FIFO_EN,FIFO_GYRO_X_OUT|FIFO_GYRO_Y_OUT|FIFO_GYRO_Z_OUT);*/
/*		ag=0;*/
/*	}*/
/*	if (mask==FIFO_ACC_OUT){*/
/*		i2cWrite(dev,FIFO_EN,FIFO_ACC_OUT);*/
/*		ag=1;*/
/*	}*/
/*	*/
/*	i2cWrite(dev,USER_CTRL,FIFO_RESET|saveu);*/
/*	unsigned int st=i2cRead(dev,INT_STATUS);*/
/*while((c=getFifoCount(dev))<1024);*/
/*		*/
/*	st=i2cRead(dev,INT_STATUS);*/

/*while(i<c){*/
/*	i2cBurstRead(dev,FIFO_R_W,6,&fifo[i]);*/
/*	i+=6;*/
/*}*/
/*i=0;	*/
/*	while (i<c){*/
/*		buildAGdata(dev,&fifo[i],ag,data);*/
/*		double x,y,z,f;*/
/*		x=data[0];*/
/*		y=data[1];*/
/*		z=data[2];*/
/*		f=data[3];*/
/*printf("%f %f %f %f/",x,y,z,f);*/
/*		avx+=data[0];*/
/*		avy+=data[1];*/
/*		avy+=data[2];*/
/*		i+=6;*/
/*printf("\n");*/
/*	*/
/*	}*/

/*	av[0]=avx/(i/6);*/
/*	av[1]=avy/(i/6);*/
/*	av[2]=avz/(i/6);*/
/*		*/
/*	printf("av %f %f %f",av[0],av[1],av[2]);*/
/*	i2cWrite(dev,FIFO_EN,(unsigned int)(savef&0xff));*/
/*	i2cWrite(dev,USER_CTRL,FIFO_RESET|saveu);*/
/*}*/

int getFifoCount(int dev){

	unsigned char buf[2]={0};
	i2cReadN(dev,buf,FIFO_COUNT_H,2);
	int c=((buf[0]<<8)&0x0000ff00)|(buf[1]&0x000000ff);
	return c;
}

/*fill buffer with fifo data*/
/*param: dev=device file descriptor*/
/*param: buf=buffer for fifo data*/
int getFifoData(int dev, unsigned char *buf){

	int packet_l=0;
	int byte_count=0;
/*check if fifo is enabled*/
	if(!(i2cRead(dev,USER_CTRL)>>6)){
		return 0;
	}
/*count how many bytes in a packet*/
	unsigned char mask=i2cRead(dev,FIFO_EN);
	packet_l=FIFO_PACKET_LEN*(mask>>7);
	packet_l+=FIFO_PACKET_LEN*((mask&FIFO_GYRO_X_OUT)>>6);
	packet_l+=FIFO_PACKET_LEN*((mask&FIFO_GYRO_Y_OUT)>>5);
	packet_l+=FIFO_PACKET_LEN*((mask&FIFO_GYRO_Z_OUT)>>4);
	packet_l+=FIFO_PACKET_LEN*((mask&FIFO_ACC_OUT)>>3)*3;

/*fifo packet count*/
	int c=getFifoCount(dev);
	
/*check for odd packet count, fifo overflow or fifo thrashold (50%) --> reset fifo*/	
	if( (i2cRead(dev,INT_STATUS) >> 4) || c>(FIFO_MAX_PACKET/2) || c&1 ){
		unsigned char s=i2cRead(dev,USER_CTRL) & 0x00ff;
		i2cWrite(dev,USER_CTRL,FIFO_RESET);
		i2cWrite(dev,USER_CTRL,s);
		c=getFifoCount(dev);
		printf("fifo reset\n");
	}
/*fill buffer with fifo data*/
	int i;
	for(i=0;i<c;i+=packet_l){
		byte_count+=i2cBurstRead(dev,FIFO_R_W,packet_l,&buf[i]);
		bWaitDataReady(dev);
	}
	return byte_count;
}

/*average on nsample sample from fifo*/
/*return order... accelerometer [x,y,z], temperature, gyroscope [x,y,z]*/
int averageFifo(int dev, double *av, int nsample){

/*check if fifo is enabled*/
	if(!(i2cRead(dev,USER_CTRL)>>6)){
		return 0;
	}
/*which sensors are enabled*/	
	int temp_flag, gyro_flag, acc_flag, gyrox_flag, gyroy_flag, gyroz_flag;
	unsigned char fifo_en=i2cRead(dev,FIFO_EN);
	temp_flag=fifo_en>>7;
	gyrox_flag=(fifo_en&FIFO_GYRO_X_OUT)>>6;
	gyroy_flag=(fifo_en&FIFO_GYRO_Y_OUT)>>5;
	gyroz_flag=(fifo_en&FIFO_GYRO_Z_OUT)>>4;
	gyro_flag=gyrox_flag+gyroy_flag+gyroz_flag;
	acc_flag=(fifo_en&FIFO_ACC_OUT)>>3;

/*packet lenght*/	
	int packet_l=((temp_flag+gyro_flag)*FIFO_PACKET_LEN)+acc_flag*3*FIFO_PACKET_LEN;
	printf("packet lenght %d\n",packet_l);
/*get data from fifo*/
	double tempSum, gyroXsum, gyroYsum, gyroZsum, accXsum, accYsum, accZsum;
	int s=0;
	int count=0;
	while(s<nsample){
		double tmp[4]={0};
		unsigned char fifo[1024]={0};
		int c=0;
		count=getFifoData(dev,fifo);
		while(c<count && (count-c)>=packet_l){
		
			if(acc_flag){
				buildAGdata(dev,&fifo[c],0,tmp);
				accXsum+=tmp[0];
				accYsum+=tmp[1];
				accZsum+=tmp[2];
				c+=(FIFO_PACKET_LEN*3);
			}
			if(temp_flag){
				tempSum+=getTemp(&fifo[c]);
				c+=FIFO_PACKET_LEN;
			}
			if(gyro_flag==3){
				buildAGdata(dev,&fifo[c],1,tmp);
				gyroXsum+=tmp[0];
				gyroYsum+=tmp[1];
				gyroZsum+=tmp[2];
				c+=FIFO_PACKET_LEN*gyro_flag;
			}
			else{
				if(gyrox_flag){
					buildAGdata(dev,&fifo[c],1,tmp);
					gyroXsum+=tmp[0];
					c+=FIFO_PACKET_LEN;
				}
				if(gyroy_flag){
					buildAGdata(dev,&fifo[c],1,tmp);
					gyroZsum+=tmp[0];
					c+=FIFO_PACKET_LEN;
				}
				if(gyroz_flag){
					buildAGdata(dev,&fifo[c],1,tmp);
					gyroZsum+=tmp[0];
					c+=FIFO_PACKET_LEN;
				}
			}
		s++;
		}
		i2cWrite(dev,USER_CTRL,0x44);
	}
	int div=s;
	int i=0;
	if(temp_flag){
		av[i]=tempSum/div;
		printf("temp average= %.3f, summ=%.3f, count=%d\n",av[i],tempSum,div);
		i++;
	}
	if(gyrox_flag){
		av[i]=gyroXsum/div;
		printf("gyro X average= %.3f\n",av[i]);
		i++;
	}
	if(gyroy_flag){
		av[i]=gyroYsum/div;
		printf("gyro Y average= %.3f\n",av[i]);
		i++;
	}
	if(gyroz_flag){
		av[i]=gyroZsum/div;
		printf("gyro Z average= %.3f\n",av[i]);
		i++;
	}
	if(acc_flag){
		av[i]=accXsum/div;
		av[i+1]=accYsum/div;
		av[i+2]=accZsum/div;
		printf("acc X average= %.3f\n",av[i]);
		printf("acc Y average= %.3f\n",av[i+1]);
		printf("acc Z average= %.3f\n",av[i+2]);
	}
	return 1;
}

/*TODO*/
void setDLPF(int dev, int val){
	if(val<0){
		val=0;
	}
	if(val>6){
		val=6;
	}
	i2cWrite(dev,CONFIG,(unsigned int)(val&DLPF_MASK));
}

/*TODO*/
void setSampleRateDiv(int dev, int val){
	if(val<0){
		val=0;
	}
	if(val>255){
		val=255;
	}
	i2cWrite(dev,CONFIG,(unsigned int)(val&0x000000ff));
}
