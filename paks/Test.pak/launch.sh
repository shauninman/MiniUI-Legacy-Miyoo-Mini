#!/bin/sh

DIR=$(dirname "$0")
cd $DIR

# {

show $DIR/confirm.png
say "Do the thing?"$'\n\n'

blank
if confirm ; then
	say "Yeah man, I wanna do it."
else
	say "Nah, I'm good."
fi

sleep 2

# } > ./log.txt 2>&1