Freedom - internal osu! mod, realtime difficulty changer for any beatmap, works online.  

![Platform](https://img.shields.io/badge/Windows_8.1+-0078D6?style=for-the-badge&logo=windows&logoColor=white)
![Mode](https://img.shields.io/badge/Mode%20--%20osu!-E3619B?style=for-the-badge&logo=osu&logoColor=white)

![preview](https://user-images.githubusercontent.com/38132413/199610571-ea5dc5df-5b5e-40d1-89b7-3b9c6955c4e0.png)  

## Download

[GitHub Actions Artifact](https://github.com/Ciremun/freedom/actions) - most recent build.  
[Releases](https://github.com/Ciremun/freedom/releases/latest) - probably outdated build.  

## Usage

run osu!, then run your favorite injector, you may use the supplied one (freedom_injector.exe).  
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

- Replay Copy:
    * Add/Remove Hard Rock (HR)
    * Replay Keys Only
    * Replay Aim Only
    * Leaderboard Replay Download

- Aimbot:
    * Cursor Speed
    * Spins Per Minute

- Relax:
    * SingleTap
    * Alternate
    * Variable Unstable Rate

- Mods:
    * Score Multiplier Changer
    * Unmod Flashlight
    * Unmod Hidden

- Misc:
    * Set Font Size
    * Set Discord RPC Status Text
    * Unload DLL

## Build

### Requirements

* MSVC x64/x86 build tools
* Windows SDK
* .NET Framework SDK

![installer_preview](https://user-images.githubusercontent.com/38132413/199610177-89f05acc-c1ff-4656-9839-2abf66ffd126.png)  

### Execute nobuild

    cmd.exe
    nobuild.exe

Alternatively, bootstrap nobuild

    cmd.exe
    vcvarsall x86
    cl nobuild.c && nobuild.exe

### Optional nobuild flags

rebuild and inject freedom with debug symbols and console log:

    nobuild.exe rebuild debug console inject

|    Flag     |          Description          |
|:-----------:|:-----------------------------:|
|             | Freedom only                  |
| rebuild     | update headers / rebuild all  |
| debug       | symbols, disable optimizations|
| console     | print logs to console         |
| inject      | run injector after build      |
| standalone  | build standalone only         |
| all         | build standalone and internal |
| run         | run standalone after build    |
