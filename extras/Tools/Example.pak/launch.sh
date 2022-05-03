#!/bin/sh

DIR=$(dirname "$0")
cd "$DIR"

# TODO: something about this causes the next show to not actually show anything :thinking_face:

# progressui &
#
# progress 0 "Sorry to make you wait"
# sleep 0.5
#
# progress 33 "Practice makes progress"
# sleep 0.5
#
# progress 66 "Not much longer now"
# sleep 0.5
#
# progress 100 "That wasn't so bad"
# sleep 1
#
# progress quit

show confirm.png
say "So, do you want to do the thing?"$'\n'

if confirm; then
	blank
	say "Yeah man, I want to do it"
	sleep 2
	show "$DIR/fam.png"
	confirm any
else
	show okay.png
	say "Nah, I'm good"$'\n'
	sleep 0.1
	confirm only
fi