#include <stdio.h>
#include <stdlib.h>
#include "h/i2c-utils.h"
#include "h/mpu6050.h"
#include "h/mpu6050_const.h"
#include <time.h>

/*sensors offsets*/
static double offsets[6]={0};

/*return sensors offsets, ag=0 -> accelerometer, ag=1 -> gyroscope*/
inline double getoffsets(int ag, double *data){
	data[0]-=offsets[ag*3];
	data[1]-=offsets[(ag*3)+1];
	data[2]-=offsets[(ag*3)+2];
}

/*switch on the chip*/
void startMpu(int dev){

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
/*wait 100ms for a stable output*/
	struct timespec wait;
	wait.tv_sec=(long)0;
	wait.tv_nsec=(long)100000000;
	nanosleep(&wait,NULL);
}

/*switch off the chip*/
void stopMpu(int dev){

/*power off*/
	i2cWrite(dev,PWR_MGMT_1,0x80);
/*reset offsets array*/
	int i=0;
	for(i;i<6;i++){
		offsets[i]=0;
	}
}

/*simple busy-wait on data ready interrupt*/
int bWaitDataReady(int dev){

/*read interrupt state*/
	unsigned char dr=i2cRead(dev,INT_STATUS)&0x01;
/*busy-wait, do nothing until data ready bit is up*/
	while(!dr){;
		dr=i2cRead(dev,INT_STATUS);
	}
	return 1;
}

/*return temperature in celsius*/
double getTemp(unsigned char *buf){

/*build raw data*/
	short int raw=(buf[0]<<8)|(buf[1]);
/*conversion in celsius*/
	double temp=((double)raw/340)+36.53;
	return temp;
}

/*return accelerometer full-scale range*/
/*usefull to convert raw data in m/s*/
double getAccelFs(int dev){

	double fs=0.0;
/*read accelerometer configuration register*/
	int cfg=i2cRead(dev,ACCEL_CONFIG);
/*find actual full-scale*/
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

/*return gyroscope full-scale range*/
double getGyroFs(dev){
	
	double fs=0.0;
/*read gyroscope configuration register*/	
	int cfg=i2cRead(dev,GYRO_CONFIG);
/*find actual full-scale*/
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

/*convert raw data in human readable format, m/s or °/s*/
/*dev = i2c device file descriptor
raw = array containing raw values
ag = accelerometer/gyroscope selector 0/1
data = array to return converted values*/
void buildAGdata(int dev, unsigned char *raw, int ag, double *data){

	double fs=0.0;
/*build raw data*/
	short int rawx=(raw[0]<<8)|raw[1];
	short int rawy=(raw[2]<<8)|raw[3];
	short int rawz=(raw[4]<<8)|raw[5];
/*accelerometer 0, gyroscope 1*/
	if(!ag){
	/*get full-scale*/
		fs=getAccelFs(dev);
	/*add offset decimal part*/
		data[0]=-offsets[0];
		data[1]=-offsets[1];
		data[2]=-offsets[2];
	}
	else{
		fs=getGyroFs(dev);
		data[0]=-offsets[3];
		data[1]=-offsets[4];
		data[2]=-offsets[5];
	}
/*convert data in m/s or °/s*/
	data[0]+=rawx/fs;
	data[1]+=rawy/fs;
	data[2]+=rawz/fs;
	data[3]+=fs;
}

/*fallback function to set offsets without using the registers*/
/*ag = 0 -> accelerometer, ag = 1 -> gyroscope, off = array of converted values*/
void setOffsets(int ag, double *off){
	offsets[ag*3]=off[0];
	offsets[(ag*3)+1]=off[1];
	offsets[(ag*3)+2]=off[2];
}

/*TODO ... ISSUE = increase error*/
/*@param: device*/
/*@param: accelerometer values in m/s*/
void setAccelOffset(int dev, double *accel_val){

/*get acceleration values and conversion*/
	double actualx=accel_val[0]*AFS_SEL_8;
	double actualy=accel_val[1]*AFS_SEL_8;
	double actualz=accel_val[2]*AFS_SEL_8;
/*printf("actual_vals %x %x %x\n",(int)actualx,(int)actualy,(int)actualz);	*/
/*read factory offsets*/
	unsigned char off[6]={0};
	i2cReadN(dev,off,XA_OFFS_USERH,6);
	
/*get temperature compesation bit*/
	unsigned char tempComp[3]={0};
	unsigned char mask=0x01;
	tempComp[0]=off[1]&mask;
	tempComp[1]=off[3]&mask;
	tempComp[2]=off[5]&mask;

/*build offsets*/
	short int xoff=(off[0]<<8)|off[1];
	short int yoff=(off[2]<<8)|off[3];
	short int zoff=(off[4]<<8)|off[5];
/*printf("factory offsets %hx %hx %hx\n",xoff,yoff,zoff);*/
/*add actual data to factory offsets*/
	xoff-=(int)actualx;
	yoff-=(int)actualy;
	zoff-=(int)actualz;
	
/*save offsets decimal parts*/
	offsets[0]=(actualx-(int)actualx)/AFS_SEL_8;
	offsets[1]=(actualy-(int)actualy)/AFS_SEL_8;
	offsets[2]=(actualz-(int)actualz)/AFS_SEL_8;
/*printf("offsets array %.5f %.5f %.5f\n",offsets[0],offsets[1],offsets[2]);*/

/*split data in bytes and set temperature compensation to lower part*/
	unsigned char noff[6]={0};
	noff[0]=(xoff>>8)&0xff;
	noff[1]=(xoff&0xfe)|tempComp[0];	
	noff[2]=(yoff>>8)&0xff;
	noff[3]=(yoff&0xfe)|tempComp[1];
	noff[4]=(zoff>>8)&0xff;
	noff[5]=(zoff&0xfe)|tempComp[2];
	
/*write new offset*/
	i2cWriteN(dev, XA_OFFS_USERH, noff, 6);
}

/*TODO ... ISSUE = not enought*/
/*@param: device*/
/*@param: gyroscope values in °/s*/
void setGyroOffset(int dev, double *gyro_val){

/*conversion*/
	double newx=-(gyro_val[0]*FS_SEL_1000);
	double newy=-(gyro_val[1]*FS_SEL_1000);
	double newz=-(gyro_val[2]*FS_SEL_1000);
/*save offsets decimal parts*/
	offsets[3]=(newx-(int)newx)/FS_SEL_1000;
	offsets[4]=(newy-(int)newy)/FS_SEL_1000;
	offsets[5]=(newz-(int)newz)/FS_SEL_1000;
/*printf("offsets array %.5f %.5f %.5f\n",offsets[0],offsets[1],offsets[2]);*/
/*split data in byte*/
	unsigned char off[6]={0};
	off[0]=(((short int)newx)>>8)&0xff;
	off[1]=((short int)newx)&0xff;
	off[2]=(((short int)newy)>>8)&0xff;
	off[3]=((short int)newy)&0xff;
	off[4]=(((short int)newz)>>8)&0xff;
	off[5]=((short int)newz)&0xff;
	
	i2cWriteN(dev,XG_OFFS_USERH,off,6);
}

/*return fifo bytes count*/
int getFifoCount(int dev){

	unsigned char buf[2]={0};
/*read fifo count registers*/
	i2cReadN(dev,buf,FIFO_COUNT_H,2);
/*build data*/
	int c=((buf[0]<<8)&0x0000ff00)|(buf[1]&0x000000ff);
	return c;
}

/*fill buffer with fifo data*/
/*param: dev=device file descriptor*/
/*param: buf=buffer for fifo data*/
int getFifoData(int dev, unsigned char *buf){
/*packet lenght and byte count*/
	int packet_l=0;
	int byte_count=0;
/*check if fifo is enabled*/
	if(!(i2cRead(dev,USER_CTRL)>>6)){
		return 0;
	}
/*count how many bytes in a packet*/
/*read fifo configuration register, which sensor write his data to the fifo*/
	unsigned char mask=i2cRead(dev,FIFO_EN);
	/*temperature*/
	packet_l=FIFO_PACKET_LEN*(mask>>7);
	/*gyroscope x,y,z axis*/
	packet_l+=FIFO_PACKET_LEN*((mask&FIFO_GYRO_X_OUT)>>6);
	packet_l+=FIFO_PACKET_LEN*((mask&FIFO_GYRO_Y_OUT)>>5);
	packet_l+=FIFO_PACKET_LEN*((mask&FIFO_GYRO_Z_OUT)>>4);
	/*accelerometer*/
	packet_l+=FIFO_PACKET_LEN*((mask&FIFO_ACC_OUT)>>3)*3;

/*fifo packet count*/
	int c=getFifoCount(dev);
	
/*check for fifo overflow, fifo thrashold (50%) or odd byte count --> reset fifo*/
	if( (i2cRead(dev,INT_STATUS) >> 4) || c>(FIFO_MAX_PACKET/2) || c&1 ){
	/*save configuration register*/
		unsigned char s=i2cRead(dev,USER_CTRL) & 0x00ff;
	/*reset fifo*/
		i2cWrite(dev,USER_CTRL,FIFO_RESET);
	/*reconfigure fifo*/
		i2cWrite(dev,USER_CTRL,s);
	/*get new byte count*/
		c=getFifoCount(dev);
	}
/*fill buffer with fifo data*/
	int i;
	/*for step depends on how many bytes in a packet*/
	for(i=0;i<c;i+=packet_l){
	/*read a packet with a single i2c command*/
		byte_count+=i2cBurstRead(dev,FIFO_R_W,packet_l,&buf[i]);
	/*wait for register refresh*/
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
	/*get data from fifo*/
	double tempSum, gyroXsum, gyroYsum, gyroZsum, accXsum, accYsum, accZsum;
	int s=0;
	int count=0;
	while(s<nsample){
		double tmp[4]={0};
		unsigned char fifo[1024]={0};
		int c=0;
		count=getFifoData(dev,fifo);
		/*sum data for al packets only if packet is complete*/
		while(c<count && (count-c)>=packet_l){
		
			if(acc_flag){
				buildAGdata(dev,&fifo[c],0,tmp);
				accXsum+=tmp[0];
				accYsum+=tmp[1];
				accZsum+=tmp[2];
				/*move fifo index*/
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
		/*read all packets, reset fifo*/
		i2cWrite(dev,USER_CTRL,0x44);
	}
	int div=s;
	int i=0;
	if(acc_flag){
		av[i]=accXsum/div;
		av[i+1]=accYsum/div;
		av[i+2]=accZsum/div;
/*		printf("acc X average= %.3f\n",av[i]);*/
/*		printf("acc Y average= %.3f\n",av[i+1]);*/
/*		printf("acc Z average= %.3f\n",av[i+2]);*/
		i+=3;
	}
	if(temp_flag){
		av[i]=tempSum/div;
/*		printf("temp average= %.3f, summ=%.3f, count=%d\n",av[i],tempSum,div);*/
		i++;
	}
	if(gyrox_flag){
		av[i]=gyroXsum/div;
/*		printf("gyro X average= %.3f\n",av[i]);*/
		i++;
	}
	if(gyroy_flag){
		av[i]=gyroYsum/div;
/*		printf("gyro Y average= %.3f\n",av[i]);*/
		i++;
	}
	if(gyroz_flag){
		av[i]=gyroZsum/div;
/*		printf("gyro Z average= %.3f\n",av[i]);*/
		i++;
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
