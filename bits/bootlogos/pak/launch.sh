#!/bin/sh

# used to generate the following paks

# /Tools/Single-use/02) Add MiniUI Boot Logo.pak
# /Tools/Single-use/02) Remove MiniUI Boot Logo.pak

DIR=$(dirname "$0")
cd "$DIR"

{

SUPPORTED_VERSION="202205010000"
EXPECT1_MD5=`cat ./image1.txt`
EXPECT2_MD5=64c9ec14d4c4995cb9681bba1eefaa34
EXPECT3_MD5=6c0f4834b754a19ad578b39b3da03c95
MAX_SUM=129732 # 12KB minus 1340b header

bail()
{
	show okay.png
	say "$1"$'\n'
	sleep 0.1
	confirm only
	exit 1
}

if [ $MIYOO_VERSION -gt $SUPPORTED_VERSION ]; then
	bail "Unknown firmware version"$'\n\n'"Please update this pak"$'\n'"and try again."$'\n'
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
	
	if [[ "$IMAGE1_MD5" != "$EXPECT1_MD5" ]]; then
		bail "Image 1 failed verification"$'\n\n'"Aborted"$'\n'
	elif [[ "$IMAGE2_MD5" != "$EXPECT2_MD5" ]]; then
		bail "Image 2 failed verification"$'\n\n'"Aborted"$'\n'
	elif [[ "$IMAGE3_MD5" != "$EXPECT3_MD5" ]]; then
		bail "Image 3 failed verification"$'\n\n'"Aborted"$'\n'
	fi
	
	IMAGE1_SIZE=$(stat -c%s ./image1.jpg)
	IMAGE2_SIZE=$(stat -c%s ./image2.jpg)
	IMAGE3_SIZE=$(stat -c%s ./image3.jpg)

	SUM=$(expr $IMAGE1_SIZE + $IMAGE2_SIZE + $IMAGE3_SIZE)
	
	if [ $SUM -gt $MAX_SUM ]; then
		bail "Sum of image file sizes"$'\n'"is greater than 128KB"$'\n\n'"Aborted"$'\n'
	fi
	
	blank
	say "Preparing logo"
	sleep 1
	if ! ./logomake; then
		bail "Preparing logo failed"$'\n\n'"Aborted"$'\n'
	fi

	blank
	say "Flashing logo"
	sleep 1
	if ! ./logowrite; then
		bail "Flashing logo failed"$'\n\n'"Aborted"$'\n'
	fi
	
	blank
	say "Done"
	sleep 1
else
	blank
	say "Aborted"
	sleep 1
fi

} &> "$LOGS_PATH/BootLogo.txt"
