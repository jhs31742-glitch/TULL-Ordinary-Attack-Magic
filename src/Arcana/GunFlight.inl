// Optional firearm source profiles. Self-owned markers keep absent ESPs out of OAR conditions.
std::array<RE::SpellItem*,17> flightGunMarkers{};
std::array<RE::TESForm*,7> flightGunOptional{};
std::array<RE::TESObjectWEAP*,28> flightGunWeapons{};
bool flightGunAnimations=true,flightGunAuthoredActive=false;
bool FlightGunKeyword(RE::TESObjectWEAP* w,const char* name){
 if(!w||!name)return false;for(auto* k:w->GetKeywords())if(k&&k->formEditorID.c_str()&&_stricmp(k->formEditorID.c_str(),name)==0)return true;return false;
}
bool FlightGunOptionalKeyword(RE::TESObjectWEAP* w,std::size_t i){
 auto* f=flightGunOptional[i];auto* k=f?f->As<RE::BGSKeyword>():nullptr;return w&&k&&w->HasKeyword(k);
}
bool FlightGunOptionalEffect(RE::PlayerCharacter* p,std::size_t i){
 auto* f=flightGunOptional[i];auto* effect=f?f->As<RE::EffectSetting>():nullptr;auto* target=p?p->AsMagicTarget():nullptr;
 return effect&&target&&target->HasMagicEffect(effect);
}
void FlightGunClear(RE::PlayerCharacter* p){
 flightGunAuthoredActive=false;if(!p)return;for(auto* marker:flightGunMarkers)if(marker&&p->HasSpell(marker))p->RemoveSpell(marker);
}
void FlightGunUpdateMarkers(RE::PlayerCharacter* p){
 auto* f=p?p->GetEquippedObject(false):nullptr;auto* w=f?f->As<RE::TESObjectWEAP>():nullptr;
 int family=-1;bool active=flightGunAnimations&&FlightActive()&&p&&w&&w->IsCrossbow()&&!p->IsDead();
 if(active)for(std::size_t i=0;i<flightGunWeapons.size();++i)if(w==flightGunWeapons[i]){family=i<14?0:1;break;}
 std::array<bool,17> match{};
 if(active && family==0)match[0]=FlightGunOptionalKeyword(w,0);
 if(active && family==0)match[1]=FlightGunOptionalKeyword(w,1);
 if(active && family==1)match[2]=((p->GetActorBase() && p->GetActorBase()->GetFormID()==7)) && !(FlightGunKeyword(w,"Shotgun")) && !(FlightGunKeyword(w,"Handgun")) && !(FlightGunKeyword(w,"Revolver")) && FlightGunKeyword(w,"Colt") && FlightGunKeyword(w,"Handgun Auto") && !(FlightGunOptionalKeyword(w,2)) && !(FlightGunOptionalKeyword(w,3)) && !(FlightGunOptionalKeyword(w,4));
 if(active && family==1)match[3]=((p->GetActorBase() && p->GetActorBase()->GetFormID()==7)) && !(FlightGunKeyword(w,"Shotgun")) && FlightGunKeyword(w,"Handgun") && !(FlightGunKeyword(w,"Handgun Auto")) && !(FlightGunKeyword(w,"Revolver")) && !(FlightGunOptionalKeyword(w,2)) && !(FlightGunOptionalKeyword(w,3)) && !(FlightGunOptionalKeyword(w,4));
 if(active && family==1)match[4]=((p->GetActorBase() && p->GetActorBase()->GetFormID()==7)) && !(FlightGunKeyword(w,"Shotgun")) && FlightGunOptionalEffect(p,5) && FlightGunKeyword(w,"Handgun") && !(FlightGunKeyword(w,"Revolver")) && !(FlightGunKeyword(w,"Handgun Auto")) && !(FlightGunOptionalKeyword(w,2)) && !(FlightGunOptionalKeyword(w,3)) && !(FlightGunOptionalKeyword(w,4));
 if(active && family==1)match[5]=((p->GetActorBase() && p->GetActorBase()->GetFormID()==7)) && !(FlightGunKeyword(w,"Shotgun")) && FlightGunKeyword(w,"Handgun") && !(FlightGunKeyword(w,"Revolver")) && !(FlightGunKeyword(w,"Handgun Auto")) && !(FlightGunOptionalKeyword(w,2)) && !(FlightGunOptionalKeyword(w,3)) && !(FlightGunOptionalKeyword(w,4)) && FlightGunOptionalEffect(p,6);
 if(active && family==1)match[6]=((p->GetActorBase() && p->GetActorBase()->GetFormID()==7)) && !(FlightGunKeyword(w,"Shotgun")) && FlightGunKeyword(w,"handgun") && FlightGunKeyword(w,"Infinite") && !(FlightGunKeyword(w,"Handgun Auto")) && !(FlightGunOptionalKeyword(w,2)) && !(FlightGunOptionalKeyword(w,3)) && !(FlightGunOptionalKeyword(w,4));
 if(active && family==1)match[7]=((p->GetActorBase() && p->GetActorBase()->GetFormID()==7)) && !(FlightGunKeyword(w,"Shotgun")) && !(FlightGunKeyword(w,"Handgun")) && !(FlightGunKeyword(w,"Revolver")) && FlightGunKeyword(w,"Handgun Auto") && !(FlightGunKeyword(w,"Colt")) && !(FlightGunOptionalKeyword(w,2)) && !(FlightGunOptionalKeyword(w,3)) && !(FlightGunOptionalKeyword(w,4));
 if(active && family==1)match[8]=((p->GetActorBase() && p->GetActorBase()->GetFormID()==7)) && !(FlightGunKeyword(w,"Shotgun")) && FlightGunOptionalEffect(p,5) && !(FlightGunKeyword(w,"Handgun")) && !(FlightGunKeyword(w,"Colt")) && FlightGunKeyword(w,"Handgun Auto") && !(FlightGunOptionalKeyword(w,2)) && !(FlightGunOptionalKeyword(w,3)) && !(FlightGunOptionalKeyword(w,4));
 if(active && family==1)match[9]=((p->GetActorBase() && p->GetActorBase()->GetFormID()==7)) && FlightGunKeyword(w,"Minigun") && !(FlightGunKeyword(w,"Pump Shotgun")) && !(FlightGunKeyword(w,"Revolver")) && !(FlightGunKeyword(w,"Handgun Auto")) && !(FlightGunOptionalKeyword(w,2)) && !(FlightGunOptionalKeyword(w,3)) && !(FlightGunOptionalKeyword(w,4));
 if(active && family==1)match[10]=((p->GetActorBase() && p->GetActorBase()->GetFormID()==7)) && !(FlightGunKeyword(w,"Shotgun")) && !(FlightGunKeyword(w,"Handgun")) && !(FlightGunKeyword(w,"Revolver")) && FlightGunKeyword(w,"Handgun Auto") && !(FlightGunOptionalKeyword(w,2)) && !(FlightGunOptionalKeyword(w,3)) && !(FlightGunOptionalKeyword(w,4)) && FlightGunOptionalEffect(p,6);
 if(active && family==1)match[11]=((p->GetActorBase() && p->GetActorBase()->GetFormID()==7)) && FlightGunKeyword(w,"Pump Shotgun") && !(FlightGunKeyword(w,"Revolver")) && !(FlightGunKeyword(w,"Handgun Auto")) && !(FlightGunOptionalKeyword(w,2)) && !(FlightGunOptionalKeyword(w,3)) && !(FlightGunOptionalKeyword(w,4));
 if(active && family==1)match[12]=((p->GetActorBase() && p->GetActorBase()->GetFormID()==7)) && !(FlightGunKeyword(w,"Handgun")) && !(FlightGunKeyword(w,"Handgun Auto")) && FlightGunKeyword(w,"Pump Shotgun") && !(FlightGunKeyword(w,"Revolver")) && !(FlightGunOptionalKeyword(w,2)) && !(FlightGunOptionalKeyword(w,3)) && !(FlightGunOptionalKeyword(w,4)) && FlightGunOptionalEffect(p,6);
 if(active && family==1)match[13]=((p->GetActorBase() && p->GetActorBase()->GetFormID()==7)) && !(FlightGunKeyword(w,"Shotgun")) && FlightGunKeyword(w,"Revolver") && !(FlightGunKeyword(w,"Handgun Auto")) && !(FlightGunOptionalKeyword(w,2)) && !(FlightGunOptionalKeyword(w,3)) && !(FlightGunOptionalKeyword(w,4));
 if(active && family==1)match[14]=((p->GetActorBase() && p->GetActorBase()->GetFormID()==7)) && !(FlightGunKeyword(w,"Shotgun")) && FlightGunOptionalEffect(p,5) && FlightGunKeyword(w,"Handgun") && FlightGunKeyword(w,"Revolver") && !(FlightGunKeyword(w,"Handgun Auto")) && !(FlightGunOptionalKeyword(w,2)) && !(FlightGunOptionalKeyword(w,3)) && !(FlightGunOptionalKeyword(w,4));
 if(active && family==1)match[15]=((p->GetActorBase() && p->GetActorBase()->GetFormID()==7)) && !(FlightGunKeyword(w,"Shotgun")) && FlightGunKeyword(w,"Handgun") && !(FlightGunKeyword(w,"Handgun Auto")) && FlightGunKeyword(w,"Revolver") && !(FlightGunOptionalKeyword(w,2)) && !(FlightGunOptionalKeyword(w,3)) && !(FlightGunOptionalKeyword(w,4)) && FlightGunOptionalEffect(p,6);
 if(active && family==1)match[16]=FlightGunKeyword(w,"Shotgun") && !(FlightGunKeyword(w,"Pump Shotgun")) && !(FlightGunKeyword(w,"Revolver")) && !(FlightGunKeyword(w,"Handgun Auto")) && !(FlightGunOptionalKeyword(w,2)) && !(FlightGunOptionalKeyword(w,3)) && !(FlightGunOptionalKeyword(w,4));
 flightGunAuthoredActive=false;std::uint32_t mask=0;
 for(std::size_t i=0;i<match.size();++i){auto* marker=flightGunMarkers[i];if(!marker)continue;bool had=p&&p->HasSpell(marker);
  if(match[i]){mask|=1u<<i;flightGunAuthoredActive=true;if(!had)p->AddSpell(marker);}else if(had)p->RemoveSpell(marker);
 }
 static std::uint32_t previous=0;static unsigned logs=0;
 if(mask!=previous){previous=mask;if(logs++<24)logger::info("FLIGHT_GUN_PROFILE_SELECTED weapon={:08X} family={} markerMask={:05X} [marker dispatch; OAR playback unverified]",w?w->GetFormID():0,family,mask);}
}
void FlightGunLoad(){
 auto* dh=RE::TESDataHandler::GetSingleton();if(!dh)return;
 for(std::size_t i=0;i<flightGunMarkers.size();++i)flightGunMarkers[i]=dh->LookupForm<RE::SpellItem>(ResolveSourceForm(0xC53+std::uint32_t(i)),plugin);
 flightGunOptional[0]=dh->LookupForm(0x923,"Call to Arms.esp");
 flightGunOptional[1]=dh->LookupForm(0x92E,"Call to Arms.esp");
 flightGunOptional[2]=dh->LookupForm(0x801,"DAR Rifles.esp");
 flightGunOptional[3]=dh->LookupForm(0x802,"DAR Rifles.esp");
 flightGunOptional[4]=dh->LookupForm(0x800,"DAR Rifles.esp");
 flightGunOptional[5]=dh->LookupForm(0x5907,"Stances - Dynamic Weapon Movesets SE.esp");
 flightGunOptional[6]=dh->LookupForm(0x16CEA,"Skrub's Gun and Guns.esp");
 flightGunWeapons[0]=dh->LookupForm<RE::TESObjectWEAP>(0x924,"Call to Arms.esp");
 flightGunWeapons[1]=dh->LookupForm<RE::TESObjectWEAP>(0x92B,"Call to Arms.esp");
 flightGunWeapons[2]=dh->LookupForm<RE::TESObjectWEAP>(0x92D,"Call to Arms.esp");
 flightGunWeapons[3]=dh->LookupForm<RE::TESObjectWEAP>(0x936,"Call to Arms.esp");
 flightGunWeapons[4]=dh->LookupForm<RE::TESObjectWEAP>(0x937,"Call to Arms.esp");
 flightGunWeapons[5]=dh->LookupForm<RE::TESObjectWEAP>(0x938,"Call to Arms.esp");
 flightGunWeapons[6]=dh->LookupForm<RE::TESObjectWEAP>(0x939,"Call to Arms.esp");
 flightGunWeapons[7]=dh->LookupForm<RE::TESObjectWEAP>(0x93A,"Call to Arms.esp");
 flightGunWeapons[8]=dh->LookupForm<RE::TESObjectWEAP>(0x940,"Call to Arms.esp");
 flightGunWeapons[9]=dh->LookupForm<RE::TESObjectWEAP>(0x941,"Call to Arms.esp");
 flightGunWeapons[10]=dh->LookupForm<RE::TESObjectWEAP>(0x942,"Call to Arms.esp");
 flightGunWeapons[11]=dh->LookupForm<RE::TESObjectWEAP>(0x943,"Call to Arms.esp");
 flightGunWeapons[12]=dh->LookupForm<RE::TESObjectWEAP>(0x944,"Call to Arms.esp");
 flightGunWeapons[13]=dh->LookupForm<RE::TESObjectWEAP>(0x955,"Call to Arms.esp");
 flightGunWeapons[14]=dh->LookupForm<RE::TESObjectWEAP>(0x2F9A,"Skrub's Gun and Guns.esp");
 flightGunWeapons[15]=dh->LookupForm<RE::TESObjectWEAP>(0x7ED5,"Skrub's Gun and Guns.esp");
 flightGunWeapons[16]=dh->LookupForm<RE::TESObjectWEAP>(0xCE2C,"Skrub's Gun and Guns.esp");
 flightGunWeapons[17]=dh->LookupForm<RE::TESObjectWEAP>(0x16C9A,"Skrub's Gun and Guns.esp");
 flightGunWeapons[18]=dh->LookupForm<RE::TESObjectWEAP>(0x16CA1,"Skrub's Gun and Guns.esp");
 flightGunWeapons[19]=dh->LookupForm<RE::TESObjectWEAP>(0x16CA3,"Skrub's Gun and Guns.esp");
 flightGunWeapons[20]=dh->LookupForm<RE::TESObjectWEAP>(0x16CA7,"Skrub's Gun and Guns.esp");
 flightGunWeapons[21]=dh->LookupForm<RE::TESObjectWEAP>(0x16CA8,"Skrub's Gun and Guns.esp");
 flightGunWeapons[22]=dh->LookupForm<RE::TESObjectWEAP>(0x16CAB,"Skrub's Gun and Guns.esp");
 flightGunWeapons[23]=dh->LookupForm<RE::TESObjectWEAP>(0x16CD4,"Skrub's Gun and Guns.esp");
 flightGunWeapons[24]=dh->LookupForm<RE::TESObjectWEAP>(0x16CDF,"Skrub's Gun and Guns.esp");
 flightGunWeapons[25]=dh->LookupForm<RE::TESObjectWEAP>(0x16CE0,"Skrub's Gun and Guns.esp");
 flightGunWeapons[26]=dh->LookupForm<RE::TESObjectWEAP>(0x19497,"Skrub's Gun and Guns.esp");
 flightGunWeapons[27]=dh->LookupForm<RE::TESObjectWEAP>(0x1949B,"Skrub's Gun and Guns.esp");
 logger::info("FLIGHT_GUN_PROFILES_READY profiles=17 sourceWeapons={} [optional sources; selection/pose unverified]",std::count_if(flightGunWeapons.begin(),flightGunWeapons.end(),[](auto* w){return w!=nullptr;}));
}
