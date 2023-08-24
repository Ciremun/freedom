# Freedom

freedom - internal osu! mod, realtime difficulty changer for any beatmap, works online.  

[![Preview!](https://github-production-user-asset-6210df.s3.amazonaws.com/88152063/262791922-4de9ad96-1700-4a85-8a75-f14880519ac7.png)](https://github-production-user-asset-6210df.s3.amazonaws.com/88152063/262784473-398fc187-9d04-4ca4-935f-595b97aa3ed6.mp4)

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

- Timewarp:
    * Scale

- Replay Copy (*a bit buggy!*):
    * Add/Remove Hard Rock (HR)
    * Replay Keys Only
    * Replay Aim Only
    * Leaderboard Replay Download

- Aimbot (*a bit buggy!*):
    * Cursor Speed
    * Spins Per Minute

- Relax (*unstable rate beta!*):
    * SingleTap
    * Alternate
    * Variable Unstable Rate

- Mods:
    * Score Multiplier Changer
    * Unmod Flashlight
    * Unmod Hidden

- Misc:
    * Set Font Size
    * Automatically Detects Beatmap, Saves Mod Settings

## Build

### Required Visual Studio Components For Building

* MSVC x64/x86 build tools
* Windows SDK
* .NET Framework SDK

![installer_preview](https://user-images.githubusercontent.com/38132413/199610177-89f05acc-c1ff-4656-9839-2abf66ffd126.png)  

open command prompt (cmd.exe) and run:  

    vcvarsall x86
    cl nobuild.c && nobuild.exe
