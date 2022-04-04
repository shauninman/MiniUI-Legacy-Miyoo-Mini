#!/bin/sh

# transfer alpha userdata to beta location
if [ -d "/mnt/SDCARD/.userdata/miyoomini" ]; then
	
	# NOTE: progressui isn't working here for some reason...
	mv "/mnt/SDCARD/.userdata" "/mnt/SDCARD/.userdata-old"
	mv "/mnt/SDCARD/.userdata-old/miyoomini" "/mnt/SDCARD/.userdata-new"
	mv "/mnt/SDCARD/.userdata-old/shared/datetime.txt" "/mnt/SDCARD/.userdata-old/shared/.minui/"
	mv "/mnt/SDCARD/.userdata-old/shared/.minui" "/mnt/SDCARD/.userdata-new/.miniui"
	rm -rf "/mnt/SDCARD/.userdata-old"
	mv "/mnt/SDCARD/.userdata-new" "/mnt/SDCARD/.userdata"
fi