#pragma once

#include "pattern.h"

//
// osu!lazer
//

// 16-byte aligned function prologues
constexpr auto on_beatmap_changed_sig { pattern::build<"55 48 83 EC 60 48 8D 6C 24 60 C5 D8 57 E4 C5 F9 7F 65 C0 C5 F9 7F 65 D0 C5 F9 7F 65 E0 C5 F9 7F 65 F0 48 89 4D 10 48 89 55 18 48 8B 4D 10 FF 15 . . . . 85 C0 . . FF 15"> };

// 55                    - push rbp
// 48 83 EC 60           - sub rsp,60
// 48 8D 6C 24 60        - lea rbp,[rsp+60]
// C5 D8 57 E4              - vxorps xmm4,xmm4,xmm4
// C5 F9 7F 65 C0           - vmovdqa [rbp-40],xmm4
// C5 F9 7F 65 D0           - vmovdqa [rbp-30],xmm4
// C5 F9 7F 65 E0           - vmovdqa [rbp-20],xmm4
// C5 F9 7F 65 F0           - vmovdqa [rbp-10],xmm4
// 48 89 4D 10           - mov [rbp+10],rcx
// 48 89 55 18           - mov [rbp+18],rdx
// 48 8B 4D 10           - mov rcx,[rbp+10]
// FF 15 . . . .        - call qword ptr [7FFADF174108]
// 85 C0                 - test eax,eax



// 55 48 83 EC 60 48 8D 6C 24 60 C5 D8 57 E4 C5 F9 7F 65 C0 C5 F9 7F 65 D0 C5 F9 7F 65 E0 C5 F9 7F 65 F0 48 89 4D 10 48 89 55 18 48 8B 4D 10 FF 15 . . . . 85 C0 . . FF 15
