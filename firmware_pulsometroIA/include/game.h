#pragma once
#include <stdint.h>
enum class Phase {Idle,Countdown,Running,Won,Lost,Cancelled,Timeout};
struct Game {
 Phase phase=Phase::Idle; uint32_t mark=0,elapsed=0,clearSince=0; unsigned errors=0; bool armed=true;
 bool busy()const{return phase==Phase::Countdown||phase==Phase::Running;}
 bool start(uint32_t now,bool touch,bool finish){if(busy()||touch||finish)return false;errors=0;elapsed=0;mark=now;phase=Phase::Countdown;armed=true;clearSince=now;return true;}
 // Contact event can also be latched by ISR, so short contacts are retained.
 void tick(uint32_t now,bool touch,bool contactEvent,bool finishEdge){
  if(phase==Phase::Countdown){if(touch||contactEvent||finishEdge){phase=Phase::Cancelled;return;}if(uint32_t(now-mark)>=3000){phase=Phase::Running;mark=now;clearSince=now;}else return;}
  if(phase!=Phase::Running)return;
  elapsed=now-mark;
  if(touch||contactEvent){clearSince=now;if(armed){++errors;armed=false;if(errors==3){phase=Phase::Lost;return;}}}
  else if(uint32_t(now-clearSince)>=150)armed=true;
  if(finishEdge){phase=Phase::Won;return;}
  if(elapsed>=600000)phase=Phase::Timeout;
 }
 void cancel(uint32_t now){if(busy()){if(phase==Phase::Running)elapsed=now-mark;phase=Phase::Cancelled;}}
};
