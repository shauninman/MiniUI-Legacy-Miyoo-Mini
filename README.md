# MiniUI

_MiniUI is a custom launcher and integrated in-game menu for the [Miyoo Mini](https://lemiyoo.cn/product/143.html) handheld emulator_

<img src="github/main.png" width=320 /> <img src="github/menu.png" width=320 />

MiniUI is simple, some might say to a fault. That's okay. I think it's one of those things where, if you like it, even just a little bit, you'll love it. (For the uninitiated, MiniUI is a direct descendent of [MinUI](https://github.com/shauninman/MinUI) which itself is a distillation of the stock launcher and menu of the [Trimui Model S](http://www.trimui.com).)

MiniUI is defined by no. No boxart. No video previews. No background menu music. No custom themes. No runners-up. MiniUI wants to disappear and speed you on your way. MiniUI is unapologetically opinionated software. Sorry not sorry. 

Check the [Releases](https://github.com/shauninman/MiniUI/releases) for the latest and if you want more info about features and setup before downloading, the readme included in every release is also available [here](https://github.com/shauninman/MiniUI/tree/main/skeleton) (without formatting).

MiniUI maybe be simple but it's also extensible. See the [Extras readme](https://github.com/shauninman/MiniUI/tree/main/extras) for info on adding additional paks or creating your own.

## Key features

- Simple launcher, simple SD card
- No launcher settings or configuration
- Automatically hides extension and region/version cruft in display names
- Consistent in-emulator menu with quick access to save states, disc changing, and emulator options
- Automatically sleeps after 30 seconds or press POWER to sleep (and later, wake)
- Automatically powers off while asleep after two minutes or hold POWER for one second
- Automatically resumes right where you left off if powered off while in-game
- Resume from manually created, last used save state by pressing X in the launcher instead of A
- Streamlined emulator frontend (picoarch + libretro cores)

## Stock systems

- Game Boy
- Game Boy Color
- Game Boy Advance
- Nintendo Entertainment System
- Super Nintendo Entertainment System
- Sega Genesis
- Sony PlayStation

## Extras

- Sega Game Gear
- Sega Master System
- Pokémon mini
- TurboGrafx-16

## Extra Extras

The community has created a bunch of additional paks, some tools and utilities, others paks based on cores created for Onion. I do not support these paks, nor have I done any sort of quality checks on them. They may lack the polish, features, and performance of my official stock and extras paks.

- [tiduscrying](https://github.com/tiduscrying/MiniUI-Extra-Extras)
- [westoncampbell](https://github.com/westoncampbell/MiyooMini/releases/tag/MiniUI-OnionPAKs)
- [eggs](https://www.dropbox.com/sh/hqcsr1h1d7f8nr3/AABtSOygIX_e4mio3rkLetWTa?preview=MiniUI_Tools.zip)

## What happened to Union?

The Miyoo Mini happened. The idea behind [Union](https://github.com/shauninman/Union) was the result of having a few of these handhelds but never being fully happy with any one of them in every context. One had a beautiful screen but was too heavy to carry casually. One was perfectly pocketable and had amazingly clicky buttons but the tiny screen wasn't great for long play sessions. So Union allowed easily moving games and save data between devices depending on context. Literally just swapping the SD card from one device to the other. But the Miyoo Mini is pocketable enough, its screen gloriously 640 by 480 enough, that I stopped using the other devices supported by Union. In a past life I learned not to develop features I won't use. Maybe I'll revisit the Union idea in the future or maybe the Mini will remain enough for a long while.
