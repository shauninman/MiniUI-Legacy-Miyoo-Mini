#!/bin/sh

show confirm.png && say "Once enabled Simple Mode can only"$'\n'"be disabled by using another device"$'\n'"to delete a file from this SD card."$'\n\n'"Please see the README included"$'\n'"in this pak for details."$'\n\n'

if confirm; then
	touch /mnt/SDCARD/.userdata/enable-simple-mode
fi