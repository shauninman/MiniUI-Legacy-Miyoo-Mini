#!/bin/sh

DIR="$(dirname "$0")"
HOME="$USERDATA_PATH"

cd "$DIR"
if [ -f Doukutsu.exe ] && [ -d data ]; then
	cd "$HOME"
	picoarch "$DIR/nxengine_libretro.so" "$DIR/Doukutsu.exe"
else
	show "okay.png"
	say "Missing exe and data folder!"$'\n\n'"Please see readme.txt"$'\n'"in the Cave Story folder"$'\n'"on your SD card."$'\n'
	confirm only
fi
