/* i2c-utils.c */
void i2cFuncs(int dev);
int i2cinit(char *filename, unsigned char hexAddr);
unsigned int i2cRead(int dev, unsigned char reg);
void i2cWrite(int dev, unsigned char reg, unsigned char val);
void i2cReadN(int dev, unsigned char *buf, unsigned char reg, int nbyte);
void i2cWriteN(int dev, unsigned char reg, unsigned char *vals, int nbyte);
int i2cBurstRead(int dev, unsigned char reg, unsigned char len, unsigned char *data);
