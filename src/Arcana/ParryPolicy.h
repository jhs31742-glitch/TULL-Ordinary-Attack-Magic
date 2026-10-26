#pragma once
#include <cmath>
namespace Arcana {
struct ParryGate {
 double ends=0,cooldown=0,grace=0;unsigned last=0;bool active=false;
 bool Arm(double now){if(active||now<cooldown)return false;active=true;ends=now+parryWindowSeconds;cooldown=now+parryCooldownSeconds;return true;}
 bool Block(double now,unsigned source,bool frontal,bool healthDamage){
  if(!source||!frontal||!healthDamage)return false;
  if(active&&now<=ends){active=false;last=source;grace=now+.18;return true;}
  return source==last&&now<=grace;
 }
 void Close(){active=false;}
};
}
