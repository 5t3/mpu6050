#include <stdio.h>
/*open*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
/*ioctl*/
#include <sys/ioctl.h>
/*i2c*/
#include <linux/i2c-dev.h>
/*read,write*/
#include <unistd.h>
/*exit*/
#include <stdlib.h>

void i2cFuncs(int dev){
	unsigned long f=0;
	if (ioctl(dev,I2C_FUNCS,&f)<0){
		/*ERROR*/
		perror("Errore accesso device");
		exit(1);
	}
	else
		printf("functionalities: %0lx", f);
		exit(0);
}

int i2cinit(char *filename,unsigned char hexAddr){
	int dev=0;
/*	char	*file="/dev/i2c-1";*/
	if ( (dev=open(filename,O_RDWR))<0){
		/*ERROR*/
		perror("Errore apertura bus I2C");
		exit(1);
	}
	
	if (ioctl(dev,I2C_SLAVE,hexAddr)<0){
		/*ERROR*/
		perror("Errore accesso device");
		exit(1);
	}
	return dev;
}

unsigned int i2cRead(int dev, unsigned char reg){

	int ret=0;
	ret=i2c_smbus_read_byte_data(dev,reg);
	if(ret>=0){
		return (unsigned int)ret&0x000000ff;
	}
	else
		perror("Errore lettura registro");
		printf("%d %d\n",dev,reg);
		i2cRead(dev,reg);
}

void i2cWrite(int dev, unsigned char reg, unsigned char val){
	int ret=i2c_smbus_write_byte_data(dev,reg,val);
	if(ret<0){
		perror("Errore scrittura registro");
		i2cWrite(dev,reg,val);
	}
/*	unsigned char buf[2]={0};*/
/*	buf[0]=reg;*/
/*	buf[1]=val;*/
/*	printf("reg %hhx val %hhx\n",buf[0],buf[1]);*/
/*	if(write(dev,buf,2)<2){*/
		/*ERROR*/
/*		printf("Errore scrittura dati da %X",reg);*/
/*		perror("");*/
/*		exit(1);*/
/*	}*/
}

void i2cReadN(int dev, unsigned char *buf, unsigned char reg, int nbyte){
	int i;
	for(i=0;i<nbyte;i++){
		buf[i]=(unsigned char)i2cRead(dev,reg+i)&0x000000ff;
/*		i2cRead(dev,(buf+i),(reg+i));*/
/*		printf("%X",reg+i);*/
	}
}

void i2cWriteN(int dev, unsigned char reg, unsigned char *vals, int nbyte){
	int i;
	for(i=0;i<nbyte;i++){
		i2cWrite(dev,reg+i,vals[i]);
/*		i2cWrite(dev,(reg+i),vals[i]);*/
	}
}
/*int val[2]={0};*/
/*int *testsm(int dev, unsigned char reg){*/

/*	val[0]=i2c_smbus_read_byte_data(dev,reg);*/
/*	val[1]=i2c_smbus_read_word_data(dev,reg);*/
/*	return val;*/
/*}*/

int i2cBurstRead(int dev, unsigned char reg, unsigned char len, unsigned char *data){

	int c=i2c_smbus_read_i2c_block_data(dev,reg,len,data);
	if(c<0){
		perror("Burst read error\n");
	}
	return c;
}
