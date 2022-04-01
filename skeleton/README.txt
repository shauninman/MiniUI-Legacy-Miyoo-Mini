MiniUI is a minimal launcher for the Miyoo Mini

Source: https://github.com/shauninman/MiniUI

----------------------------------------
Features

- No settings or configuration
- Simple file browser backd by a
  sensible SD card layout
- No button combinations to remember
- Consistent in-emulator menu with
  quick access to save states, disc
  changing, and emulator options
- Auto-sleep after 30 seconds or
  press POWER to sleep (and wake)
- Auto-power off while sleeping after
  1 minute or hold POWER for 1 second
  to power off manually
- Auto-save state when powering off,
  both auto and manually
- Auto-restore auto-save state the
  next time you power on
- Resume from your last used, manually
  created save state by pressing X in 
  the file browser instead of A
- Streamlined emulator front end (picoarch)
  backed by familiar emulator cores 

----------------------------------------
Roms

Included in this zip is a Roms folder containing folders for each console MiniUI currently supports. You can rename these folders but you must keep the uppercase tag name in parentheses in order to retain the mapping to the correct emulator (eg. "Nintendo Entertainment System (FC)" could be renamed to "Nintendo (FC)", "NES (FC)", or "Famicom (FC)"). You should probably preload these with roms and copy each folder to the Roms folder on your SD card before installing. Or not, I'm not the boss of you.

----------------------------------------
Bios

A lot of emulators require or perform much better with official bios. MiniUI is strictly BYOB. Place the bios for each system in a folder that matches the tag in the corresponding Roms folder name (eg. bios for "Sony PlayStation (PS)" roms gos in "/Bios/PS/").

----------------------------------------
Disc-based games

To streamline launching disc-based games with MiniUI place your bin/cue (and/or iso/wav files) in a folder with the same name as the cue file. MinUI will launch the cue file instead of navigating into the folder. For multi-disc games, put all the the discs in a single folder and create an m3u file (just a text file containing the relative path to each disc's cue file on a separate line) with the same name as the folder. Instead of showing the entire messy contents of the folder, MinUI will just display Disc 1, Disc2, etc.

----------------------------------------
Install

Copy the contents of this archive to the root of a fresh SD card. Don't forget the hidden ".tmp_update" folder and don't unzip the "MiniUI.zip" archive. Insert your SD card into the device and power it on. MiniUI will install automatically and be ready to go in about 20 seconds. It may display the default boot logo or a black screen during this time. That's normal.

----------------------------------------
Update

To install an update copy just the "MiniUI.zip" archive to the root of your SD card. Insert your SD card into the device and power it on. There will be a new "Update" entry at the bottom of the main menu (press up to jump to the bottom of the menu). Open the updater and allow the update to complete.

----------------------------------------
Thanks

To eggs for his example middleware code, neon scalers, and patience in the face of my endless questions.

Check out eggs' releases: https://www.dropbox.com/sh/hqcsr1h1d7f8nr3/AABtSOygIX_e4mio3rkLetWTa

To neonloop for putting together the Trimui toolchain from which I learned everything I know about docker and buildroot and the basis for the Miyoo Mini toolchain I put together, and for picoarch, without which I might have given up entirely.

Check out neonloop's repos: https://git.crowdedwood.com

To Jim Gray (and the entire Onion crew) for taking a joke, running with it, and turning a stop-gap into a beloved launcher and community, it's been amazing to watch.

Check out Onion for a more full-bodied Miyoo Mini experience: https://github.com/jimgraygit/Onion/

And to the entire Retro Game Handhelds Miyoo Mini and dev Discord community for their enthusiasm and encouragement. 

Checkout the channel on the Retro Game Handhelds Discord: https://discord.com/channels/529983248114122762/891336865540620338