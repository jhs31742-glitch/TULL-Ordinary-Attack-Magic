TOAM::Presentation::Values presentationDraft{};bool presentationLoaded=false;std::string presentationStatus;
void PresentationDraft(){if(!presentationLoaded){presentationDraft=TOAM::Presentation::Get();presentationLoaded=true;}}
void PresentationActions(int page){
 using namespace TOAM::Presentation;
 if(button(L("Apply and save###presSave","적용 및 저장###presSave")))presentationStatus=Save(presentationDraft)?L("Saved. New circle owners use the selected colors.","저장했습니다. 다음 마법진 소유자부터 선택한 색상을 사용합니다."):L("Save failed; active values retained.","저장 실패: 기존 유효 설정을 유지합니다.");
 sameLine(0,-1);if(button(L("Reset defaults###presReset","기본값 복원###presReset"))){Values d;if(page==0){presentationDraft.master=d.master;presentationDraft.spellVolume=d.spellVolume;presentationDraft.eventVolume=d.eventVolume;presentationDraft.selected=d.selected;}if(page==1)presentationDraft.shake=d.shake;if(page==2)presentationDraft.colors=d.colors;presentationStatus=L("Defaults restored in draft. Apply to save.","편집값을 원본 기본값으로 복원했습니다. 적용하면 저장됩니다.");}
 sameLine(0,-1);if(button(L("Reload INI###presReload","INI 다시 읽기###presReload"))){if(Reload()){presentationDraft=Get();presentationStatus=L("INI reloaded.","INI를 다시 읽었습니다.");}else presentationStatus=L("Invalid INI; active values retained.","INI 값 오류: 기존 유효 설정을 유지합니다.");}
 if(!presentationStatus.empty())text(presentationStatus.c_str(),nullptr);
}
void __stdcall RenderAudio(){
 using namespace TOAM::Presentation;Language();PresentationDraft();
 text(L("Original multiplier:1.00x. Range:0.00-1.00x;0=mute. Existing gains,voices,timing and fades remain.","원본 배율1.00x. 범위0.00~1.00x;0은 음소거입니다. 기존 Gain·재생 횟수·시점·Fade를 유지합니다."),nullptr);
 slider(L("Master volume###audioMaster","전체 효과음 배율###audioMaster"),&presentationDraft.master,0,1,"%.2fx",0);
 for(std::size_t i=0;i<spellCount;++i){std::string label=std::string(L(spellKeys[i],spellKR[i]))+"###audioSpell"+std::to_string(i);slider(label.c_str(),&presentationDraft.spellVolume[i],0,1,"%.2fx",0);}
 text(L("Event volume and stable sound ID (one-shots and loops are separate). Original IDs shown below.","이벤트별 음량 및 고정 음원 ID입니다. 단발음과 반복음은 호환 범위를 구분합니다. 원본 ID도 표시합니다."),nullptr);
 for(std::size_t i=0;i<eventCount;++i){
  const auto& e=events[i];std::string label=std::string(L(e.key,e.korean))+"###eventVolume"+std::to_string(i);slider(label.c_str(),&presentationDraft.eventVolume[i],0,1,"%.2fx",0);
  auto& id=presentationDraft.selected[i];char state[256]{};std::snprintf(state,sizeof(state),"%03u  %s  [%s %03u]",id,L(sources[id-1].name,sources[id-1].korean),L("Original","원본"),e.original);text(state,nullptr);
  std::string prev=std::string(L("Previous","이전"))+"###audioPrev"+std::to_string(i),next=std::string(L("Next","다음"))+"###audioNext"+std::to_string(i);
  if(button(prev.c_str()))do{id=id==1?unsigned(sources.size()):id-1;}while(!Compatible(Event(i),id));sameLine(0,-1);
  if(button(next.c_str()))do{id=id==sources.size()?1:id+1;}while(!Compatible(Event(i),id));
 }
 text(L("No sound preview. Hover volume also retains the existing Flight/HumVolume multiplier.","미리듣기는 제공하지 않습니다. 부유음은 기존 Flight/HumVolume 배율도 함께 유지합니다."),nullptr);PresentationActions(0);
}
void __stdcall RenderCamera(){
 Language();PresentationDraft();text(L("Original:1.00x;0.00x skips TOAM requests. Duration and event timing remain.","원본1.00x;0.00x이면 TOAM 흔들림 요청을 생략합니다. 지속시간과 발동 시점은 유지합니다."),nullptr);
 if(slider(L("Camera shake master###shakeMaster","화면 흔들림 배율###shakeMaster"),&presentationDraft.shake,0,3,"%.2fx",0))presentationDraft.shake=std::round(presentationDraft.shake*20.F)/20.F;
 text(L("Step:0.05x. Skyrim SE clamps final camera strength to1;high original events can saturate.","간격0.05x. Skyrim SE는 최종 강도를1로 제한하므로 원래 강한 이벤트는 고배율에서 상한에 도달합니다."),nullptr);PresentationActions(1);
}
void __stdcall RenderColors(){
 using namespace TOAM::Presentation;Language();PresentationDraft();text(L("Default preserves each spell's original1.0.0 material. New casts/effect owners snapshot the selection.","Default는 각 마법의1.0.0 원래 재질입니다. 새 시전·효과 소유자가 선택값을 고정합니다."),nullptr);
 for(std::size_t i=0;i<spellCount;++i){std::string name=std::string(L(spellKeys[i],spellKR[i]))+" : "+colorKeys[std::size_t(presentationDraft.colors[i])];text(name.c_str(),nullptr);
  for(std::size_t c=0;c<colorKeys.size();++c){if(c)sameLine(0,-1);std::string key=std::string(colorKeys[c])+"###color"+std::to_string(i)+"_"+std::to_string(c);if(button(key.c_str()))presentationDraft.colors[i]=Color(c);}
 }
 text(L("Unsafe shared geometry or failed material isolation keeps the original appearance;see the Native log.","공유 기하 또는 재질 독립성 검사가 실패하면 원본 외형을 유지하며 Native 로그에 기록합니다."),nullptr);PresentationActions(2);
}
