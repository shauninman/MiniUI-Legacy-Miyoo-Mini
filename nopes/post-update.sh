#!/bin/sh

VERSION=`cat "/mnt/SDCARD/.system/version.txt"` # this is now multiline

progress 100 $VERSION
sleep 2
