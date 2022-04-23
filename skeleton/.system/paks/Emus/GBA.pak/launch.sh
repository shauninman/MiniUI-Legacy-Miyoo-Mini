#!/bin/sh

EMU_TAG=GBA
EMU_EXE=gpsp

#############################

ROM="$1"
mkdir -p "$BIOS_PATH/$EMU_TAG"
mkdir -p "$SAVES_PATH/$EMU_TAG"
HOME="$USERDATA_PATH"
cd "$HOME"
picoarch "$CORES_PATH/${EMU_EXE}_libretro.so" "$ROM" LCD &> "$LOGS_PATH/$EMU_TAG.txt"
