#!/bin/sh

EMU_EXE=mednafen_supafaust
CORES_PATH=$(dirname "$0")

###############################

EMU_TAG=$(basename "$(dirname "$0")" .pak)
#------------------------------
# update from unprefixed name
OLD_EXE=supafaust
OLD_DATA_PATH="$USERDATA_PATH/.picoarch-${OLD_EXE}-${EMU_TAG}"
if [ -d "$OLD_DATA_PATH" ]; then
	NEW_DATA_PATH="$USERDATA_PATH/.picoarch-${EMU_EXE}-${EMU_TAG}"
	mv "$OLD_DATA_PATH" "$NEW_DATA_PATH"
fi
#------------------------------
ROM="$1"
mkdir -p "$BIOS_PATH/$EMU_TAG"
mkdir -p "$SAVES_PATH/$EMU_TAG"
HOME="$USERDATA_PATH"
cd "$HOME"
picoarch "$CORES_PATH/${EMU_EXE}_libretro.so" "$ROM" &> "$LOGS_PATH/$EMU_TAG.txt"
