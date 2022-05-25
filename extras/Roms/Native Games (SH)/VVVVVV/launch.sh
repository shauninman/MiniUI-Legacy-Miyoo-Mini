#!/bin/sh

cd "$(dirname "$0")"
HOME="$USERDATA_PATH"

if [ -f data.zip ]; then
	warn
	./vvvvvv
else
	show "okay.png"
	say "Missing data.zip!"$'\n\n'"Please see readme.txt"$'\n'"in the VVVVVV folder"$'\n'"on your SD card."$'\n'
	confirm only
fi

