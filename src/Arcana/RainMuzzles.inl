struct RainShot {ArtSlot circle{};RE::NiPoint3 origin{};RE::NiMatrix3 rotation{};float length=0,width=0,phase=0,power=0,age=0,wait=0;bool active=false,launched=false;};
std::array<RainShot,192> rainShotsPending{};std::array<unsigned,2> muzzleCursor{};
void StopRainShots(int side,bool tail,RE::Actor* actor){for(int i=side*rainMuzzlesPerHand;i<(side+1)*rainMuzzlesPerHand;i++){auto& shot=rainShotsPending[i];if(shot.active){RetireCircle(shot.circle,side,tail,actor);shot=RainShot{};}}}
int FreeMuzzleSlots(int side){int count=0;for(int i=side*rainMuzzlesPerHand;i<(side+1)*rainMuzzlesPerHand;i++)if(!rainShotsPending[i].active)count++;return count;}
void QueueRainShot(Hand& h,int side,RE::Actor* actor,RE::NiAVObject* root,const RE::NiPoint3& origin,const RE::NiMatrix3& rotation,float length,float width){
 for(int n=0;n<rainMuzzlesPerHand;n++){int k=side*rainMuzzlesPerHand+(muzzleCursor[side]+n)%rainMuzzlesPerHand;auto& shot=rainShotsPending[k];if(shot.active)continue;
  muzzleCursor[side]=(k-side*rainMuzzlesPerHand+1)%rainMuzzlesPerHand;shot=RainShot{};shot.active=true;shot.origin=origin;shot.rotation=rotation;shot.length=length;shot.width=width;shot.phase=h.cadence.elapsed;shot.power=RainShake(h.rainQueued);
  shot.circle.tint.color=h.circleColor;EnsureCircle(shot.circle,forms.rainMuzzle[k],actor,root);return;
 }
}
void UpdateRainShots(RE::ActorMagicCaster* c,Hand& h,int side,RE::Actor* actor,RE::NiAVObject* root,float dt){
 int launchedThisUpdate=0;
 for(int i=side*rainMuzzlesPerHand;i<(side+1)*rainMuzzlesPerHand;i++){
  auto& shot=rainShotsPending[i];if(!shot.active)continue;shot.wait+=dt;EnsureCircle(shot.circle,forms.rainMuzzle[i],actor,root);
  if(shot.wait>2.f&&!shot.launched){logger::warn("RAIN_MUZZLE_TIMEOUT slot={}",i);h.failed++;RetireCircle(shot.circle,side,false,actor);shot=RainShot{};continue;}
  if(!CircleReady(shot.circle))continue;shot.age+=dt;auto direction=shot.rotation*RE::NiPoint3{0,1,0};
  PoseCircle(shot.circle,shot.origin-direction*8,shot.rotation,rainMuzzleScale*(shot.launched?1.f:Smooth(shot.age/rainMuzzlePrepare)),1);
  if(!shot.launched&&shot.age>=rainMuzzlePrepare&&FreePulseSlots(side)>0&&launchedThisUpdate<2){
   if(h.rainFired+1==rainShots){auto ground=RE::NiPoint3{shot.origin.x,shot.origin.y,h.field.z+4};shot.rotation=DirectionRotation(ground-shot.origin);shot.length=std::min(4096.f,Length(ground-shot.origin)+60);}
   Launch(h,side,c,actor,root,shot.origin,shot.rotation,Rain,shot.length,shot.width,h.cadence.elapsed,shot.power);h.rainFired++;launchedThisUpdate++;Sound(h.shots[h.soundSlot++%16],forms.sound[4],root,.34f,TOAM::Presentation::Event::TowerShot);
   if(h.cadence.elapsed>=6&&h.cadence.elapsed>=h.nextShake){Shake(actor,.28f+.48f*RainPower(h.rainFired),.25f);h.nextShake=h.cadence.elapsed+.12f;}
   shot.launched=true;shot.age=0;
  }else if(shot.launched&&shot.age>=rainMuzzleHold){RetireCircle(shot.circle,side,true,actor);shot=RainShot{};}
 }
}
bool RainShotBusy(int side){for(int i=side*rainMuzzlesPerHand;i<(side+1)*rainMuzzlesPerHand;i++)if(rainShotsPending[i].active)return true;return false;}
