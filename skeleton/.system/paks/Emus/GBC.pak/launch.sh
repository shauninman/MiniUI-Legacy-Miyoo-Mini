#!/bin/sh

EMU_TAG=GBC
EMU_EXE=gambatte

ROM="$1"

EMU_DIR=$(dirname "$0")
mkdir -p "$BIOS_PATH/$EMU_TAG"
mkdir -p "$SAVES_PATH/$EMU_TAG"
HOME="$USERDATA_PATH"
cd "$HOME"
picoarch "$CORES_PATH/${EMU_EXE}_libretro.so" "$ROM" LCD &> "$LOGS_PATH/$EMU_TAG.txt"
