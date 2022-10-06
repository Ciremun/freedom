# Freedom

freedom - internal osu! mod, realtime difficulty changer for any beatmap, works online.  

![freedom](https://user-images.githubusercontent.com/38132413/189542546-2f701100-2690-470f-b9cc-63fe6faa8181.png)  

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

 - Singletap Relax (*no hit timing randomization!*)  

 - Aimbot (*reverse sliders are not implemented!*):
    * Cursor Speed
    * Spins Per Minute

- Misc:
    * Set Font Size
    * Hide Mod Menu
    * Automatically Detects Beatmap, Saves Mod Settings

## Build

### Required Visual Studio Components For Building

* MSVC x64/x86 build tools
* Windows SDK
* .NET Framework SDK

open command prompt (cmd.exe) and run:  

    vcvarsall x86
    build.bat

Makefile provides debug build:  

    vcvarsall x86
    make -B -j16
