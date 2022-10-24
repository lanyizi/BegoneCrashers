# BegoneCrashers
Fix RA3's wall crash

# How to use
Open `DllInjector.exe` and leave it there, that's it!
Now crashers cannot crash your RA3 anymore~

# How it works

The core part of this fix is inside
[BegoneCrashers/PatchGame.cpp](BegoneCrashers/PatchGame.cpp).
The game crashes because it tries to dereference null pointers
when a player orders some structures (for example, wall) to move.
To fix the crash, two `jmp` instructions are added to the game code,
they will redirect program execution to our code
before RA3 tries to dereference pointers.
Our code contains null checks for those pointers,
so we can avoid the game crash.

You are welcome to use the source code of this project
inside your own projects.
But please try to keep the fix "compatible" with this project,
which means writing two (and only two) 5-byte `jmp` instructions
in the same locations as this project did.
In this way, there won't any issues even if
multiple tools are all going to fix the wall crash.
