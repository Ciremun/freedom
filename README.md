# Freedom
Internal osu! mod, realtime difficulty changer for any beatmap, works online.  

![Platform](https://img.shields.io/badge/Windows_8.1+-0078D6?style=for-the-badge&logo=windows&logoColor=white)
![Mode](https://img.shields.io/badge/Mode%20--%20osu!-E3619B?style=for-the-badge&logo=osu&logoColor=white)

> [!WARNING]  
> Status: Unknown. Use at your own risk.  

![preview](https://user-images.githubusercontent.com/38132413/199610571-ea5dc5df-5b5e-40d1-89b7-3b9c6955c4e0.png)  

## Download

[GitHub Actions Artifact](https://github.com/Ciremun/freedom/actions) - most recent build.  
[Releases](https://github.com/Ciremun/freedom/releases/latest) - probably outdated build.  

## Usage

run osu!, then run your favorite manual map injector, you may use the supplied one (freedom_injector.exe).  
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
    * Cursor Delay
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
    * Set UI Font Size
    * Set Discord RPC Status Text

## Build

### Requirements

* MSVC x64/x86 build tools
* Windows SDK
* .NET Framework SDK

![installer_preview](https://user-images.githubusercontent.com/38132413/199610177-89f05acc-c1ff-4656-9839-2abf66ffd126.png)  

### Compile and run build

    cl.exe nob.c && .\nob.exe

### Build options

    nob.exe -help

## Thank

[Dear ImGui](https://github.com/ocornut/imgui)  
[stb](https://github.com/nothings/stb)  
[nob.h](https://github.com/tsoding/nob.h)  
[nobuild](https://github.com/tsoding/nobuild)  
[pattern.h](https://github.com/Ciremun/freedom/blob/6c0ebbc374dc34ff8925b1758d01ed5911e9fc47/vendor/pattern.h)  
