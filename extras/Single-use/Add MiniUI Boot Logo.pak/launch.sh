#!/bin/sh

EXPECT1_MD5=32bfc3839b8fa8a0816030554506487f

DIR=$(dirname "$0")
cd "$DIR"

{

EXPECT2_MD5=64c9ec14d4c4995cb9681bba1eefaa34
EXPECT3_MD5=6c0f4834b754a19ad578b39b3da03c95
SUPPORTED_VERSION="202205010000"
MAX_SUM=129732 # 12KB minus 1340b header

if [ $MIYOO_VERSION -gt $SUPPORTED_VERSION ]; then
	show okay.png
	say "Unknown firmware version"$'\n\n'"Please update this pak"$'\n'"and try again."$'\n'
	sleep 0.1
	confirm only
	exit 1
fi

show confirm.png
say "WARNING! Modifying the boot"$'\n'"logo has the potential to brick"$'\n'"your Miyoo Mini. No takebacks!"$'\n'

if confirm; then
	blank
	
	say "Verifying logo"
	sleep 1
	
	IMAGE1_MD5=$(md5sum ./image1.jpg | awk '{print $1}')
	IMAGE2_MD5=$(md5sum ./image2.jpg | awk '{print $1}')
	IMAGE3_MD5=$(md5sum ./image3.jpg | awk '{print $1}')
	
	echo $IMAGE1_MD5
	echo $IMAGE2_MD5
	echo $IMAGE3_MD5
	
	blank
	if [[ "$IMAGE1_MD5" != "$EXPECT1_MD5" ]]; then
		say "Image 1 failed verification"$'\n\n'"Aborted"$'\n'
		sleep 0.1
		confirm only
		exit 1
	elif [[ "$IMAGE2_MD5" != "$EXPECT2_MD5" ]]; then
		say "Image 2 failed verification"$'\n\n'"Aborted"$'\n'
		sleep 0.1
		confirm only
		exit 1
	elif [[ "$IMAGE3_MD5" != "$EXPECT3_MD5" ]]; then
		say "Image 3 failed verification"$'\n\n'"Aborted"$'\n'
		sleep 0.1
		confirm only
		exit 1
	fi
	
	IMAGE1_SIZE=$(stat -c%s ./image1.jpg)
	IMAGE2_SIZE=$(stat -c%s ./image2.jpg)
	IMAGE3_SIZE=$(stat -c%s ./image3.jpg)
	
	echo $IMAGE1_SIZE
	echo $IMAGE2_SIZE
	echo $IMAGE3_SIZE
	
	SUM=$(expr $IMAGE1_SIZE + $IMAGE2_SIZE + $IMAGE3_SIZE)
	
	echo $SUM
	
	if [ $SUM -gt $MAX_SUM ]; then
		show okay.png
		say "Sum of image file sizes"$'\n'"is greater than 128KB"$'\n\n'"Aborted"$'\n'
		sleep 0.1
		confirm only
		exit 1
	fi
	
	say "Preparing logo"
	sleep 1
	./logomake

	blank
	say "Flashing logo"
	sleep 1
	./logowrite
	
	blank
	say "Done"
	sleep 1
else
	blank
	say "Aborted"
	sleep 1
fi

} &> ./log.txt