#!/bin/sh
# MiniUI.pak

# init_lcd
cat /proc/ls
sleep 0.5

# init backlight
echo 0 > /sys/class/pwm/pwmchip0/export
echo 800 > /sys/class/pwm/pwmchip0/pwm0/period
echo 6 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle
echo 1 > /sys/class/pwm/pwmchip0/pwm0/enable

# init charger detection
if [ ! -f /sys/devices/gpiochip0/gpio/gpio59/direction ]; then
	echo 59 > /sys/class/gpio/export
	echo in > /sys/devices/gpiochip0/gpio/gpio59/direction
fi

export SDCARD_PATH=/mnt/SDCARD
export BIOS_PATH=/mnt/SDCARD/Bios
export ROMS_PATH=/mnt/SDCARD/Roms
export SAVES_PATH=/mnt/SDCARD/Saves
export USERDATA_PATH=/mnt/SDCARD/.userdata
export LOGS_PATH=/mnt/SDCARD/.userdata/logs
export CORES_PATH=/mnt/SDCARD/.system/cores
export RES_PATH=/mnt/SDCARD/.system/res

killall tee
rm -f "$SDCARD_PATH/update.log"

export LD_LIBRARY_PATH="/mnt/SDCARD/.system/lib:$LD_LIBRARY_PATH"
export PATH="/mnt/SDCARD/.system/bin:$PATH"

lumon & # adjust lcd luma and saturation

CHARGING=`cat /sys/devices/gpiochip0/gpio/gpio59/value`
if [ "$CHARGING" == "1" ]; then
	batmon
fi

keymon &

mkdir -p "$LOGS_PATH"
mkdir -p "$USERDATA_PATH/.mmenu"
mkdir -p "$USERDATA_PATH/.miniui"

# init datetime
DATETIME_PATH=$USERDATA_PATH/.miniui/datetime.txt
if [ -f "$DATETIME_PATH" ]; then
	DATETIME=`cat "$DATETIME_PATH"`
	date +'%F %T' -s "$DATETIME"
	DATETIME=`date +'%s'`
	DATETIME=$((DATETIME + 6 * 60 * 60))
	date -s "@$DATETIME"
fi

cd $(dirname "$0")

EXEC_PATH=/tmp/minui_exec
touch "$EXEC_PATH"  && sync

CPU_PATH=/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
while [ -f "$EXEC_PATH" ]; do
	echo ondemand > "$CPU_PATH"

	./MiniUI &> "$LOGS_PATH/MiniUI.txt"
	
	echo `date +'%F %T'` > "$DATETIME_PATH"
	echo performance > "$CPU_PATH"
	sync

	NEXT="/tmp/next"
	if [ -f $NEXT ]; then
		CMD=`cat $NEXT`
		rm -f $NEXT
		eval $CMD
		if [ -f "/tmp/using-swap" ]; then
			rm -f "/tmp/using-swap"
			swapoff -a
		fi
		
		echo `date +'%F %T'` > "$DATETIME_PATH"
		sync
	fi
done

reboot # just in case
