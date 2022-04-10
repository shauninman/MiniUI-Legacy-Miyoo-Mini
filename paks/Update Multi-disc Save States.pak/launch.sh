#!/bin/sh

cd "$(dirname "$0")"

# TODO: confirm

if [ -f "./.fixed" ]; then
	echo "Already fixed."
else
	MMENU_PATH="$USERDATA_PATH/.mmenu"

	find "$ROMS_PATH" -type f -name '*.m3u' | while read M3U_PATH ; do
		ROM_PATH=$(dirname "$M3U_PATH")
		TAG_PATH=$(dirname "$ROM_PATH")
		TAG_NAME=${TAG_PATH#*(}
		TAG_NAME=${TAG_NAME%)*}
		M3U_FILE=$(basename "$M3U_PATH")
		while IFS= read -r ROM_FILE || [[ -n "$ROM_FILE" ]]; do
			find "$MMENU_PATH/$TAG_NAME" -type f -name "$ROM_FILE.*" | while read SRC_PATH ; do
				DST_PATH=${SRC_PATH/$ROM_FILE/$M3U_FILE}
				SRC_EXT="${SRC_PATH##*.}"
				mv -f "$SRC_PATH" "$DST_PATH" # later disc saves overwrite earlier disc saves
				if [ "$SRC_EXT" = "bmp" ]; then
					TXT_PATH=${DST_PATH/bmp/txt}
					echo -n "$ROM_PATH/$ROM_FILE" > "$TXT_PATH"
				fi
			done
		done < "$M3U_PATH"
	done
	echo "Fixed!"
	touch "./.fixed"
	# TODO: option to delete this pak
fi