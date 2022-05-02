#!/bin/sh

DIR=$(dirname "$0")
cd "$DIR"

BOOT="gpio output 85 1; bootlogo 0 0 0 0 0; mw 1f001cc0 11; gpio out 8 0; sf probe 0;sf read 0x22000000 \${sf_kernel_start} \${sf_kernel_size}; gpio out 8 1; gpio output 4 1; bootm 0x22000000"

show confirm.png
say "Prevents the flash of"$'\n'"static that may occur"$'\n'"before the boot logo"$'\n'

if confirm; then
	blank
	/etc/fw_setenv bootcmd $BOOT
	show okay.png
	say "Patch applied"$'\n'
	sleep 0.1
	confirm only
else
	blank
	say "Aborted"
	sleep 1
fi