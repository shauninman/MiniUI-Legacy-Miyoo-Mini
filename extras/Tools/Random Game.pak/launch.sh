#!/bin/sh

# TODO: write a daemon that binds this to Y?

say "Finding a random game..."
sleep 1

cd $SDCARD_PATH
ROMS="$SDCARD_PATH/Roms"
FILE="$ROMS"
LOOPS=0
while :; do
	if [[ ! -d "$FILE" ]]; then
		break
	fi
	
	LINES=`find "$FILE" -maxdepth 1 -not -path '*/.*'`
	TOTAL=`echo -n "$LINES" | grep -c '^'`
	if [[ "$TOTAL" -le "1" ]]; then # the current directory seems to be listed first
		FILE="$ROMS" # take it again from the top?
		LOOPS=`expr ${LOOPS} + 1`
		if [[ "$LOOPS" -ge "16" ]]; then
			FILE=""
			break
		fi
	fi
	
	R=`expr ${RANDOM} % ${TOTAL} + 1`
	FILE=`echo "$LINES" | head -n $R | tail -1`
	
	if [[ -f "$FILE" ]]; then
		break
	fi
	
	BASE=$(basename "$FILE")
	if [[ -f "$FILE/$BASE.m3u" ]]; then
		break
	fi
	
	if [[ -f "$FILE/$BASE.cue" ]]; then
		break
	fi
done

if [[ -z "$FILE" ]]; then
	show okay.png
	say "Could not find any games."$'\n'"(Too many empty folders?)"$'\n'
	sleep 0.1
	confirm only
	exit
fi

# the shell isn't liking parameter expansion 
# when the var contains spaces so we have to 
# pop the directories manually
TMP="$FILE"
EMU_TAG="$TMP"
while :; do
	TMP=$(dirname "$EMU_TAG")
	if [ "$TMP" = "$ROMS" ]; then
		EMU_TAG=$(basename "$EMU_TAG")
		break
	fi
	EMU_TAG="$TMP"
done

# see above
# EMU_TAG="$FILE"
# EMU_TAG="${EMU_TAG//\/mnt\/SDCARD\/Roms\//}"
# EMU_TAG="${EMU_TAG//\/*/}"
EMU_TAG="${EMU_TAG//*(/}"
EMU_TAG="${EMU_TAG//)*/}"

echo -n "$FILE" > "/tmp/last.txt"
# echo -n "${EMU_TAG}.pak $FILE" > "$SDCARD_PATH/random.txt"

if [[ -f "$SDCARD_PATH/Emus/${EMU_TAG}.pak/launch.sh" ]]; then
	"$SDCARD_PATH/Emus/${EMU_TAG}.pak/launch.sh" "$FILE"
elif [[ -f "$SDCARD_PATH/.system/paks/Emus/${EMU_TAG}.pak/launch.sh" ]]; then
	"$SDCARD_PATH/.system/paks/Emus/${EMU_TAG}.pak/launch.sh" "$FILE"
fi