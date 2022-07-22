@echo off

where /q cl || (
  echo ERROR: "cl" not found - please run this from the MSVC x86 native tools command prompt
  exit /b 1
)

@echo on

call cl /DUNICODE /std:c++17 /O2 /EHsc /nologo /Fe:freedom.exe injector.cpp /link /MACHINE:x86
call cl /DUNICODE /std:c++17 /DIMGUI_USE_STB_SPRINTF /O2 /EHsc /nologo /Iinclude /Iimgui /Iimgui/backends freedom/*.cpp imgui/*.cpp imgui/backends/*.cpp /link /DLL /MACHINE:x86 /OUT:freedom.dll
call csc /nologo /target:library /out:prejit.dll freedom/prejit.cs
