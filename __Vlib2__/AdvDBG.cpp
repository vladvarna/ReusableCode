#ifdef _DEBUG //compile only for debug builds
#pragma once
#define V_ADVANCED_DEBUG

#include <mmed.cpp>

Chronos dbgcronom; //debug stop watch
MSTimer killtime; //auto kill after
FPS dbgfpscnt; //debug frames per second counter

#define CM_BEG dbgcronom.Abs();
#define CM_END printbox("Cronometru: %u ns",dbgcronom.Sec(1000000.));
#define LIMIT_FPS(mult) if(dbgcronom.Sec(mult)) dbgcronom.Abs(); else return;
#define SHOW_FPS if(dbgfpscnt.Count()) status("%u",dbgfpscnt.fps);
#define STAT_FPS if(dbgfpscnt.Count()) wsb.stat(0,"%u",dbgfpscnt.fps);
#define AUTO_KILL(secs) if(killtime.Rel()>secs*1000) exit(0);

#endif