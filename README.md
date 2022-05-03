# MiniUI

_MiniUI is a custom launcher and integrated in-game menu for the Miyoo Mini._

MiniUI is simple, some might say to a fault.
That's okay.
I think it's one of those things where, if you like it, even just a little bit, you'll love it.

MiniUI is defined by no.
No boxart.
No video previews.
No background menu music.
No custom themes.
No alsorans.
MiniUI is too full of self-loathing for all that.
It doesn't like launchers.
It doesn't like being one.
It wants to disappear and speed you on your way.
MiniUI is unapologetically opinionated software.
Sorry not sorry.

## Screenshots

<img src="github/main.png" width=320 /> <img src="github/menu.png" width=320 />

## Installing

You can download the latest release of MiniUI from
[here](https://github.com/shauninman/MiniUI/releases/latest).

If you want more info before downloading, the readme included in every release is also available
[here](https://github.com/shauninman/MiniUI/tree/main/skeleton).

MiniUI may be simple, but it's also extensible.
See the [Extras readme](https://github.com/shauninman/MiniUI/tree/main/extras) for info on
adding additional paks (tools and emulators) or creating your own.

## F.A.Q.

### What happened to Union?

The Miyoo Mini happened.
The idea behind [Union](https://github.com/shauninman/Union) was the result of having a few of
these handhelds but  never being fully happy with any one of them in every context.
One had a beautiful screen but was too heavy to carry casually.
One was perfectly pocketable and had amazingly clicky buttons but the tiny screen wasn't great
for long play sessions.
So Union allowed easily moving games and save data between devices depending on context.
Literally just swapping the SD card from one device to the other.
But the Miyoo Mini is pocketable enough, its screen gloriously 640 by 480 enough,
that I stopped using the other devices supported by Union.
In a past life I learned not to develop features I won't use.
Maybe I'll revisit the Union idea in the future or maybe the Mini will remain enough for a long while.

### How do I enable fast forwarding (FF)?

Enter a game, press `Menu`, then go to `Advanced -> Options -> Emulator controls -> Toggle FF`
and bind it to a button or button combination you like.
FF works as a toggle, which means you only need to press once to enable it and then once to
disable it, no need to keep holding the button.

### I've transferred my saves from RetroArch, but when launching a game it doesn't load the save.

You might have mistaken saves for states.
States are saved by games themselves, usually represented by a floppy disk and accompanied by a
warning to not turn off the device while it's saving.
States are saved by emulators and are essentially snapshots of specific moments in the game.

**Saves are usually portable between emulators, while states are not!**

If you've manually saved the game before moving it to MiniUI or if your game supports autosaving,
then you can try starting it up and finding your save game inside the game itself.

Miyoo Mini's stock package and Onion use RetroArch for emulation, while MiniUI uses PicoArch,
so you won't be able to transfer states between them, **but you can still transfer saves**.

### I can't see any apps, how can I add them?

Since MiniUI prefers a minimalistic approach, apps (or rather tools) aren't included by default,
but they can still be added, as mentioned, by following the
[extras package's readme](https://github.com/shauninman/MiniUI/tree/main/extras).

### Why are there so few emulators, how do I add more?

As mentioned, MiniUI likes to be minimalistic, so it only includes most often used emulators by
default.

Read the [extras package's readme](https://github.com/shauninman/MiniUI/tree/main/extras) to
learn how to add additional emulators or package your own.
