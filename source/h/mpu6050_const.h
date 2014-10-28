/*	registri per taratura giroscopio*/
#define XG_OFFS_USERH 0x13
#define XG_OFFS_USERL 0x14
#define YG_OFFS_USERH 0x15
#define YG_OFFS_USERL 0x16
#define ZG_OFFS_USERH 0x17
#define ZG_OFFS_USERL 0x18

/*	registri per taratura accelerometro*/
#define XA_OFFS_USERH 0x06
#define XA_OFFS_USERL 0x07
#define YA_OFFS_USERH 0x08
#define YA_OFFS_USERL 0x09
#define ZA_OFFS_USERH 0x0A
#define ZA_OFFS_USERL 0x0B

/**/
#define SMPLRT_DIV 0x19

/**/
#define CONFIG 0x1A
#define DLPF_1 0x00
#define DLPF_2 0x01
#define DLPF_3 0x02
#define DLPF_4 0x03
#define DLPF_5 0x04
#define DLPF_6 0x05
#define DLPF_7 0x06
#define DLPF_MASK 0x07

/*	sensibilitá giroscopio*/
#define GYRO_CONFIG 0x1B
#define GMASK_250 0x00
#define GMASK_500 0x08
#define GMASK_1000 0x10
#define GMASK_2000 0x18
#define FS_SEL_250 131.0
#define FS_SEL_500 65.5
#define FS_SEL_1000 32.8
#define FS_SEL_2000 16.4

/**/
#define ACCEL_CONFIG 0x1C
#define AMASK_2 0x00
#define AMASK_4 0x08
#define AMASK_8 0x10
#define AMASK_16 0x18
#define AFS_SEL_2 16384.0
#define AFS_SEL_4 8192.0
#define AFS_SEL_8 4096.0
#define AFS_SEL_16 2048.0

/**/
#define USER_CTRL 0x6a
#define FIFO_EN_CTRL 0x40
#define FIFO_RESET 0x04
/**/
#define FIFO_EN 0x23
#define FIFO_TEMP_OUT 0x80
#define FIFO_GYRO_X_OUT 0x40
#define FIFO_GYRO_Y_OUT 0x20
#define FIFO_GYRO_Z_OUT 0x10
#define FIFO_GYRO_OUT 0x70
#define FIFO_ACC_OUT 0x08

#define FIFO_PACKET_LEN 2
#define FIFO_MAX_PACKET 1024

#define FIFO_COUNT_H 0x72
#define FIFO_COUNT_L	0x73

#define FIFO_R_W 0x74

/*	lettura accelerometro*/
#define ACCEL_XOUT_H 0X3B
#define ACCEL_XOUT_L 0X3C
#define ACCEL_YOUT_H 0X3D
#define ACCEL_YOUT_L 0X3E
#define ACCEL_ZOUT_H 0X3F
#define ACCEL_ZOUT_L 0X40

/*	letture temperatura*/
#define TEMP_OUT_H 0X41
#define TEMP_OUT_L 0X42

/*	lettura giroscopio*/
#define GYRO_XOUT_H 0X43
#define GYRO_XOUT_L 0X44
#define GYRO_YOUT_H 0X45
#define GYRO_YOUT_L 0X46
#define GYRO_ZOUT_H 0X47
#define GYRO_ZOUT_L 0X48

/**/
#define PWR_MGMT_1 0X6B

/**/
#define PWR_MGMT_2 0X6C

/**/
#define WHO_AM_I 0X75

/**/
#define INT_ENABLE 0x38
#define INT_FIFO_OFLOW_EN 0x10
#define INT_DATA_READY_EN 0x01
#define INT_STATUS 0x3a
#define INT_FIFO_OFLOW INT_FIFO_OFLOW_EN
#define INT_DATA_READY INT_DATA_READY_EN

#define BUS 0
#define I2C 0
#define GAMASK_3 AMASK_16
#define GAMASK_2 AMASK_8
#define GAMASK_1 AMASK_4
#define GAMASK_0 AMASK_2

static const char *DLPF_TABLE="VAL   BANDWIDTH   DELAY   FREQ_CAMP   BANDWIDTH   DELAY   FREQ_CAMP\n0     260Hz       0ms       1kHz      256Hz       0.98ms  8kHz\n1     184Hz       2ms       1kHz      188Hz       1.9ms   1kHz\n2     94Hz        3ms       1kHz      98Hz        2.8ms   1kHz\n3     44Hz        4.9ms     1kHz      42Hz        4.8ms   1kHz\n4     21Hz        8.5ms     1kHz      20Hz        8.3ms   1kHz\n5     10Hz        13.8ms    1kHz      10Hz        13.4ms  1kHz\n6     5Hz         19ms      1kHz      5Hz         18.8ms  1kHz";
