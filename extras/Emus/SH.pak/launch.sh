#!/bin/sh

# for Native Games or Ports

EMU_TAG=$(basename "$(dirname "$0")" .pak)
mkdir -p "$SAVES_PATH/$EMU_TAG"
"$1" &> "$LOGS_PATH/$EMU_TAG.txt"