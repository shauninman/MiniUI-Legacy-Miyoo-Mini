#!/bin/sh

cd "$(dirname "$0")"
a=`ps | grep screenshot | grep -v grep`
if [ "$a" == "" ] ; then
    ./screenshot &
    say "Enabled screenshots"$'\n'"Press L2 + R2 to take"
	sleep 2
else
    killall screenshot
    say "Disabled screenshots"
	sleep 1
fi

