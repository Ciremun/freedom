# Freedom

freedom - internal osu! mod, realtime difficulty changer for any beatmap, works online.  

![preview](https://user-images.githubusercontent.com/38132413/199610571-ea5dc5df-5b5e-40d1-89b7-3b9c6955c4e0.png)  

## Download

[GitHub Actions Artifact](https://github.com/Ciremun/freedom/actions) - most recent build.  
[Releases](https://github.com/Ciremun/freedom/releases/latest) - probably outdated build.  

## Usage

run osu!, then run your favorite injector, you may use the supplied one (freedom.exe).  
mod ui should appear in top left corner of the osu! window, see [controls](#controls).  

### Controls

|    Keys     |   Description  |
|:-----------:|:--------------:|
| Right Click |    Settings    |
| F11         |  Hide Mod Menu |

## Features

- Difficulty Changer:
    * Approach Rate (AR)
    * Circle Size (CS)
    * Overall Difficulty (OD)

- Replay Copy (*a bit buggy!*):
    * Add/Remove Hard Rock (HR)
    * Replay Keys Only
    * Replay Aim Only
    * Leaderboard Replay Download

- Aimbot (*a bit buggy!*):
    * Cursor Speed
    * Spins Per Minute

- Singletap Relax (*no hit timing randomization!*)

- Misc:
    * Set Font Size
    * Hide Mod Menu
    * Automatically Detects Beatmap, Saves Mod Settings

## Build

### Required Visual Studio Components For Building

* MSVC x64/x86 build tools
* Windows SDK
* .NET Framework SDK

![installer_preview](https://user-images.githubusercontent.com/38132413/199610177-89f05acc-c1ff-4656-9839-2abf66ffd126.png)  

open command prompt (cmd.exe) and run:  

    vcvarsall x86
    build.bat

Makefile provides debug build:  

    vcvarsall x86
    make -B -j16
