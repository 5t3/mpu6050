CC=gcc
CFLAGS=-ggdb -c
CLIBRARY=ncurses
LD=ld
LDFLAGS=
all: i2c-utils.o mpu6050.o
	$(CC) -ggdb -o main i2c-utils.o mpu6050.o main.c -l$(CLIBRARY)
	$(CC) -ggdb -o main-dbg i2c-utils.o mpu6050.o main-dbg.c -l$(CLIBRARY)

i2c-utils.o:
	$(CC) $(CFLAGS) i2c-utils.c

mpu6050.o:
	$(CC) $(CFLAGS) mpu6050.c

clean:
	rm -fr *.o *.*~ main
