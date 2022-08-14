MiniUI is an extensible launcher for the Miyoo Mini

Source: https://github.com/shauninman/MiniUI

----------------------------------------
Adding Paks

A pak is a folder with a pak extension that contains a shell script named launch.sh. 

There are two kinds of paks, emulators and tools. Emulator paks live in the Emus folder. Tool paks live in the Tools folder. Both of those folders live at the root of your SD card. If you're adding paks created by someone else (sharing is caring) just copy each pak into the corresponding folder. That's usually it but they may have additional steps (eg. additions to the Roms or Bios folders). Check for a readme or ask the original author if you're unsure.

----------------------------------------
Creating an emulator pak

There are two kinds of emulator paks, both similar, both super simple. The first re-uses an existing MiniUI libretro core. The second uses a custom libretro core. To illustrate the different approaches, let's create a Sega Game Gear pak (already part of this zip file).

First create an "Emus" folder at the root of your SD card if one doesn't already exist. Then make sure your file browser is showing invisible files for this next step. Copy "/.system/paks/Emus/MD.pak/" into the Emus folder and rename it to GG.pak. GG is your emulator tag. The tag is used to map its roms folder to the pak. That's it. You're done. You just created your first MiniUI emulator pak! Now create the corresponding roms folder "/Roms/Game Gear (GG)/" (see, there's that tag again!) and load it up with a few roms. Boot up MiniUI and give it a shot. Easy, right? Let's do the next one.

First download the smsplus-gx_libretro.so core and put it in your GG.pak. Then open launch.sh in a plain text editor (make sure this editor uses just `\n` for newlines, Linux is picky Windows friends). We need to change two things this time. Replace `EMU_EXE=picodrive` with `EMU_EXE=smsplus-gx` then right below that line add `CORES_PATH=$(dirname "$0")`. Save and you're done. Still pretty damn easy.

There are two things to consider when creating your own emulator pak. If a core doesn't support save states, autoresume will not work. That means if a player manually powers off in game or it falls alseep while on the menu and automatically powers off, they will lose any unsaved progress. Please don't do this to people. It doesn't matter if you mention it in a readme (!), people won't read it and they will be angry at you (or more likely, me). Just don't. The other thing worth noting is that MiniUI doesn't enable swap by default (it can degrade performance of emulators that don't need it). For emulators that require more memory than is physically available on the Miyoo Mini, just call `needs-swap` before launching `picoarch` to enabled a 128MB swap image.

----------------------------------------
Bios files

You'll need to BYOB for these emulator paks included in this zip file.

 MGBA: gba_bios.bin
  PCE: syscard3.pce
  PKM: bios.min
  SGB: sgb_bios.bin
 
----------------------------------------
Creating a tool pak

Tool paks can run a GUI program, setup a daemon, or just script a quick action before exiting. See the Files and Screenshots paks for examples but usually a tool pak will change to the current directory, set any necessary environmental variables, and then launch a binary relative to the folder. 

MiniUI comes with a couple of binaries to simplify providing feedback to users: show, blank, say, and confirm. (If you need to perform a longer action with discrete steps there is also the more advanced progressui.) Check out the Example pak for...um, an example of how those work.
