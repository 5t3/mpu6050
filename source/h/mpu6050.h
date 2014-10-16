/* mpu6050.c */
void startMpu(int dev);
void stopMpu(int dev);
double getTemp(unsigned char *buf);
double getAccelFs(int dev);
double getGyroFs(int dev);
void buildAGdata(int dev, unsigned char *raw, int ag, double *data);
void setAccelOffset(int dev, double *accel_val);
void setGyroOffset(int dev, double *gyro_val);
double averageFullFifo(int dev, unsigned char mask, double *av);
int getFifoCount(int dev);
int getFifoData(int dev, unsigned char *buf);
int averageFifo(int dev, double *av, int nsample);
void setDLPF(int dev, int val);
void setSampleRateDiv(int dev, int val);
