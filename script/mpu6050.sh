#!/bin/bash

#	registri per taratura giroscopio
XG_OFFS_USERH=0x13
XG_OFFS_USERL=0x14
YG_OFFS_USERH=0x15
YG_OFFS_USERL=0x16
ZG_OFFS_USERH=0x17
ZG_OFFS_USERL=0x18

#	registri per taratura accelerometro
XA_OFFS_USERH=0x06
XA_OFFS_USERL=0x07
YA_OFFS_USERH=0x08
YA_OFFS_USERL=0x09
ZA_OFFS_USERH=0x0A
ZA_OFFS_USERL=0x0B

#
SMPLRT_DIV=0x19

#
CONFIG=0x1A

#	sensibilitá giroscopio
GYRO_CONFIG=0x1B
GMASK_250=0x00
GMASK_500=0x08
GMASK_1000=0x10
GMASK_2000=0x18
FS_SEL_250=131
FS_SEL_500=65.5
FS_SEL_1000=32.8
FS_SEL_2000=16.4

#
ACCEL_CONFIG=0x1C
AMASK_2=0x00
AMASK_4=0x08
AMASK_8=0x10
AMASK_16=0x18
AFS_SEL_2=16384
AFS_SEL_4=8192
AFS_SEL_8=4096
AFS_SEL_16=2048

#
FIFO_EN=0x23

#	lettura accelerometro
ACCEL_XOUT_H=0X3B
ACCEL_XOUT_L=0X3C
ACCEL_YOUT_H=0X3D
ACCEL_YOUT_L=0X3E
ACCEL_ZOUT_H=0X3F
ACCEL_ZOUT_L=0X40

#	letture temperatura
TEMP_OUT_H=0X41
TEMP_OUT_L=0X42

#	lettura giroscopio
GYRO_XOUT_H=0X43
GYRO_XOUT_L=0X44
GYRO_YOUT_H=0X45
GYRO_YOUT_L=0X46
GYRO_ZOUT_H=0X47
GYRO_ZOUT_L=0X48

#
PWR_MGMT_1=0X6B

#
PWR_MGMT_2=0X6C

#
WHO_AM_I=0X75

BUS=0
I2C=0
GAMASK_3=$((AMASK_16))
GAMASK_2=$((AMASK_8))
GAMASK_1=$((AMASK_4))
GAMASK_0=$((AMASK_2))

#init. param: bus, i2c addr
function init_mpu(){
MESS="usage: $0 bus_address i2c_address"
	if [[ $# -ne 2 || $2 -ne `i2cget -y $1 $2 $WHO_AM_I` ]];then
		echo $MESS
		exit 1
	else
		echo "power on sensor"
		BUS=$1
		I2C=$2
		i2cset -y $BUS $I2C $PWR_MGMT_1 0
		i2cset -y 1 0x68 0x1b 0x10
		i2cset -y 1 0x68 0x1c 0x10
		if [ $? -ne 0 ];then
			echo "error on init"
			exit 1
		else
			echo "sensor on"
		fi
	fi
}

#read n bytes
#param: address, num bytes
#ret hex values
function readnbyte (){
		addr=`echo $1|tr a-z A-Z|cut -d"X" -f2`
		addrD=`echo "ibase=16;$addr"|bc -l`
	if [ $addrD -gt 0 ];then
		if [ $2 -le 0 ];then
			$2=1
		fi
		#declare -a ret
		ind=0
		ret[0]=0
		while [ $ind -lt $2 ];do
			ret[$ind]=`i2cget -y $BUS $I2C 0x$addr| cut -d"x" -f2|tr a-z A-Z`
			ind=$((ind+1))
			addr=`echo "ibase=16;obase=10;$addr+1"|bc -l`
		done
		echo ${ret[@]}
	else
		echo "usage: $0 address num_bytes"
		exit 1
	fi
}

#convert an hexadecimal number in signed decimal
#param: number to convert with or without 0x prefix, case insensitive
function hexca2dec(){

	if [[ $# -lt '1' ]];then
		echo "error on hexca2dec. missing argument"
		exit 1
	fi
	x=`echo $1|tr a-z A-Z|cut -d"X" -f2`
	if [[ $((0x$x>>15)) -gt '0' ]];then
		x=`echo "obase=10;ibase=16;-((FFFF-$x)+1)"|bc -l`
	else
		x=`echo "obase=10;ibase=16;$x"|bc -l`
	fi
	
	echo `echo $x|sed -e 's/^\./0./' -e 's/^-\./-0./'`

}

function dec2hexca2(){

	if [[ $# -lt '1' ]];then
		echo "error on dec2hexca2. missing argument"
		exit 1
	fi
	x=`echo $1|sed -e 's/^\./0./' -e 's/^-\./-0./'`
	x=`echo "define r(a){if (a%1>0.5){return (a/1)+1}else return a/1};r($x)"|bc`
	if [[ $(echo $x|sed s/\\./,/) -lt '0' ]];then
		x=`echo $x|cut -d'-' -f2`
		x=`echo "obase=16;$x"|bc`
		x=`echo "obase=16;ibase=16;((FFFF-$x)+1)"|bc`
	else
		x=`echo "obase=16;$x"|bc`
	fi
	echo $x
}

#get temperature value
function gettemp(){
	rawhex=`readnbyte $TEMP_OUT_H 2|sed 's/ //g'`
#echo $rawhex
	rawdec=`echo "ibase=16;-((FFFF-$rawhex))+1"|bc -l`
#	echo "$rawhex $rawdec"
	echo "scale=3;($rawdec/340)+36.53"|bc
}

#get gyroscop/accelometer values
#ret decimal values
function getvalues(){
#echo	"----------------$# $0 $1 $2 $3"
	addr=0
	cfgaddr=0
#	mask=0
	fs_3=0
	fs_2=0
	fs_1=0
	fs_0=0
	if [[ $1 = 'g' ]];then
		addr=$GYRO_XOUT_H
		cfgaddr=$GYRO_CONFIG
#		mask=$GMASK_2000
		fs_3=$FS_SEL_2000
		fs_2=$FS_SEL_1000
		fs_1=$FS_SEL_500
		fs_0=$FS_SEL_250
		elif [[ $1 = 'a' ]];then
			addr=$ACCEL_XOUT_H
			cfgaddr=$ACCEL_CONFIG
	#		mask=$GMASK_2000
			fs_3=$AFS_SEL_16
			fs_2=$AFS_SEL_8
			fs_1=$AFS_SEL_4
			fs_0=$AFS_SEL_2
	else
		echo -e "usage $0 a|g [o]\n a=accelerometer\n g=gyroscope\n o=raw values useful for calibration"
		exit 1
	fi
	raw=`readnbyte $addr 6`
	all=(0 0 0 0 0 0)
	ind=0
	for i in $raw;do
		all[$ind]=$i
		ind=$((ind+1))
	done
	cfg=`i2cget -y $BUS $I2C $cfgaddr`
	case $(( cfg & GAMASK_3 )) in
		$GAMASK_3)
			cfg=$fs_3
#			echo "3"
		;;
		$GAMASK_2)
			cfg=$fs_2
#			echo "2"
		;;
		$GAMASK_1)
			cfg=$fs_1
#			echo "1"
		;;
		$GAMASK_0)
			cfg=$fs_0
#			echo "0"
		;;
	esac
#	echo "$cfg $GAMASK_3"
	rawx=`echo ${all[0]}${all[1]}`
	rawy=`echo ${all[2]}${all[3]}`
	rawz=`echo ${all[4]}${all[5]}`
	if [[ $# -eq 2 && $2 = 'o' ]];then
		g=($rawx $rawy $rawz $cfg)
		echo ${g[@]}
		exit 0
	fi
#	if [ $((0x$rawx>>15)) -gt '0' ];then
#		rawx=`echo "ibase=16;-((FFFF-$rawx)+1)"|bc -l`
#	else
#		rawx=`echo "ibase=16;$rawx"|bc -l`
##rawx=$((0x$rawx))
#	fi
#	if [ $((0x$rawy>>15)) -gt '0' ];then
#		rawy=`echo "ibase=16;-((FFFF-$rawy)+1)"|bc -l`
#	else
#		rawy=`echo "ibase=16;$rawy"|bc -l`
##rawy=$((0x$rawy))
#	fi
#	if [ $((0x$rawz>>15)) -gt '0' ];then
#		rawz=`echo "ibase=16;-((FFFF-$rawz)+1)"|bc -l`
#	else
#		rawz=`echo "ibase=16;$rawz"|bc -l`
##rawz=$((0x$rawz))
#	fi
rawx=`hexca2dec $rawx`
rawy=`hexca2dec $rawy`
rawz=`hexca2dec $rawz`
	g[0]=`echo "scale=3;$rawx/$cfg"|bc -l`
	g[1]=`echo "scale=3;$rawy/$cfg"|bc -l`
	g[2]=`echo "scale=3;$rawz/$cfg"|bc -l`
	g[0]=`echo ${g[0]}|sed -e 's/^\./0./' -e 's/^-\./-0./'`
	g[1]=`echo ${g[1]}|sed -e 's/^\./0./' -e 's/^-\./-0./'`
	g[2]=`echo ${g[2]}|sed -e 's/^\./0./' -e 's/^-\./-0./'`
#	if [[ $# -eq 2 && $2 = 'o' ]];then
#	g[3]=$cfg
#	fi
	echo ${g[@]}
}

#set accelerometer and gyroscope offset register for noise cancellation
#param: a=accelerometer; g=gyroscope
function setoffset(){
	div=0
	adxh=0
	adxl=0
	adyh=0
	adyl=0
	adzh=0
	adzl=0
	all=0
	xh=0
	xl=0
	yh=0
	yl=0
	zh=0
	zl=0
	if [[ $1 = 'a' ]];then
	echo "set accelerometer cancellation offset"
		div=$AFS_SEL_8
		adxh=$XA_OFFS_USERH
		adxl=$XA_OFFS_USERL
		adyh=$YA_OFFS_USERH
		adyl=$YA_OFFS_USERL
		adzh=$ZA_OFFS_USERH
		adzl=$ZA_OFFS_USERL
		#hexadecimal values
		all=`getvalues a o`
		elif [[ $1 = 'g' ]];then
		echo "set gyroscope cancellation offset"
			div=$FS_SEL_1000
			adxh=$XG_OFFS_USERH
			adxl=$XG_OFFS_USERL
			adyh=$YG_OFFS_USERH
			adyl=$YG_OFFS_USERL
			adzh=$ZG_OFFS_USERH
			adzl=$ZG_OFFS_USERL
			#hexadecimale values
			all=`getvalues g o`
	fi
	
	x=`echo $all|cut -d" " -f1`
	y=`echo $all|cut -d" " -f2`
	z=`echo $all|cut -d" " -f3`
	cfg=`echo $all|cut -d" " -f4`

	off=0
	temp_comp[0]=0
	temp_comp[1]=0
	temp_comp[2]=0
	if [[ $1 = 'a' ]];then
		off=`readnbyte $adxh 6`
		#hex values
		for i in 2 4 6;do
			temp_comp[$(( i / 3 ))]=$(( 0x`echo $off|cut -d" " -f$i` & 0x01))
		done
		#hex values
		xh=`echo $off|cut -d" " -f1`
		xl=`echo $off|cut -d" " -f2`
		yh=`echo $off|cut -d" " -f3`
		yl=`echo $off|cut -d" " -f4`
		zh=`echo $off|cut -d" " -f5`
		xl=`echo $off|cut -d" " -f6`
echo "sssssssssssss $xh"
#		x=$(( $((0x$xh+x)) | 0x${temp_comp[0]} ))
#		y=$(( $((0x$yh+y)) | 0x${temp_comp[1]} ))
	fi
#=====================================================================
echo "first $x $y $z"
x=`hexca2dec $x`
y=`hexca2dec $y`
z=`hexca2dec $z`
echo "second $x $y $z"
#valori con fondoscala CFG convertiti (visualizzabili ma senza decimali)
x=`echo "$x/$cfg"|bc`
y=`echo "$y/$cfg"|bc`
z=`echo "$z/$cfg"|bc`
echo "third $x $y $z /$cfg"
#cambio del fondoscala in DIV
x=`echo "$x*$div"|bc`
y=`echo "$y*$div"|bc`
z=`echo "$z*$div"|bc`
echo "before $xh $xl $yh $yl $zh $zl"
echo "before $x $y $z"
#conversione vecchi valori offset, inversione letture, somma
#vecchi valori giá con fondoscale DIV
x=`echo "($(hexca2dec $xh$xl))+(($x)*(-1))"|bc`
y=`echo "($(hexca2dec $yh$yl))+(($y)*(-1))"|bc`
z=`echo "($(hexca2dec $zh$zl))+(($z)*(-1))"|bc`

#conversione nuovi offset in hex in complemento a 2
x=`dec2hexca2 $x`
y=`dec2hexca2 $y`
z=`dec2hexca2 $z`

#	echo "$x $y $z"
#	echo "$xh $xl $yh $yl $zh $zl"
#	x=`echo "obase=16;ibase=16;$x+$xh$xl"|bc`
#	y=`echo "obase=16;ibase=16;$y+$yh$yl"|bc`
#	z=`echo "obase=16;ibase=16;$z+$zh$zl"|bc`
	echo "after $x $y $z"
	echo "after $xh $xl $yh $yl $zh $zl"
	#divisione word in byte
	xh=`echo "ibase=10;obase=16;$(( 0x$x>>8 ))"|bc`
	xl=`echo "ibase=10;obase=16;$(( 0x$x&0x00ff ))+${temp_comp[0]}"|bc`
	yh=`echo "ibase=10;obase=16;$(( 0x$y>>8 ))"|bc`
	yl=`echo "ibase=10;obase=16;$(( 0x$y&0x00ff ))+${temp_comp[1]}"|bc`
	zh=`echo "ibase=10;obase=16;$(( 0x$z>>8 ))"|bc`
	zl=`echo "ibase=10;obase=16;$(( 0x$z&0x00ff ))+${temp_comp[2]}"|bc`
	
	echo "last $xh $xl $yh $yl $zh $zl"
#=====================================================================
echo "3"
echo "$BUS $I2C $adxh $xh"
	i2cset -y $BUS $I2C $adxh 0x$xh
	i2cset -y $BUS $I2C $adxl 0x$xl
	i2cset -y $BUS $I2C $adyh 0x$yh
	i2cset -y $BUS $I2C $adyl 0x$yl
	i2cset -y $BUS $I2C $adzh 0x$zh
	i2cset -y $BUS $I2C $adzl 0x$zl
}
#----------------executable start------------------

#param: bus, i2c addr
init_mpu $1 $2
if [ $? -ne '0' ];then
	echo "error on init"
	exit 1
fi
clear
while true;do
echo -e "Temperature:	`gettemp` °\nGyroscpe:	`getvalues g` °/s\nAccelerometer:	`getvalues a` g"
	read -s -n1 -p "type 'c' if you want to update the offset registers
type 'r' if you want to reset the device" -t1 cal
	echo -e "\n"
	if [[ $cal = "c" ]];then
		setoffset a
		setoffset g
	elif [[ $cal = "r" ]];then
	echo -e "reset\n"
	clear
		i2cset -y $BUS $I2C $PWR_MGMT_1 0x80
		i2cset -y $BUS $I2C $PWR_MGMT_1 0x00
#		i2cset -y 1 0x68 0x1b 0x10
#		i2cset -y 1 0x68 0x1c 0x10
#	else
#		echo "wrong character"
	fi
	
tput cup 0 0
done
exit
