TOAM::Presentation::Values presentationDraft{};
bool presentationLoaded=false;
std::string presentationStatus;
constexpr std::array<const char*,5> colorKR{"원본 (Default)","파랑 (Blue)","빨강 (Red)","금색 (Gold)","보라 (Purple)"};
void PresSeparator(){if(separator)separator();}
void PresentationDraft(){if(!presentationLoaded){presentationDraft=TOAM::Presentation::Get();presentationLoaded=true;}}
TOAM::Presentation::Values PresentationPage(int page){
 auto value=TOAM::Presentation::Get();
 if(page==0){value.master=presentationDraft.master;value.spellVolume=presentationDraft.spellVolume;value.eventVolume=presentationDraft.eventVolume;value.selected=presentationDraft.selected;}
 if(page==1)value.shake=presentationDraft.shake;
 if(page==2)value.colors=presentationDraft.colors;
 return value;
}
void PresentationActions(int page){
 using namespace TOAM::Presentation;PresSeparator();
 if(button(L("Apply page and save###presSave","이 페이지 적용 및 저장###presSave")))presentationStatus=Save(PresentationPage(page))?L("Page saved. Circle changes apply to new casts/effect owners.","이 페이지를 저장했습니다. 색상은 다음 시전·효과 소유자부터 적용됩니다."):L("Save failed; active values retained.","저장 실패: 기존 유효 설정을 유지합니다.");
 sameLine(0,-1);
 if(button(L("Reset page defaults###presReset","이 페이지 기본값###presReset"))){Values d;if(page==0){presentationDraft.master=d.master;presentationDraft.spellVolume=d.spellVolume;presentationDraft.eventVolume=d.eventVolume;presentationDraft.selected=d.selected;}if(page==1)presentationDraft.shake=d.shake;if(page==2)presentationDraft.colors=d.colors;presentationStatus=L("Default draft restored. Apply this page to save.","편집값을 원본 기본값으로 복원했습니다. 적용 및 저장이 필요합니다.");}
 sameLine(0,-1);
 if(button(L("Reload INI###presReload","INI 다시 읽기###presReload"))){if(Reload()){presentationDraft=Get();presentationStatus=L("INI reloaded; drafts refreshed.","INI를 다시 읽고 편집값을 갱신했습니다.");}else presentationStatus=L("Invalid INI; active values retained.","INI 값 오류: 기존 유효 설정을 유지합니다.");}
 if(!presentationStatus.empty())text(presentationStatus.c_str(),nullptr);
}
void PresentationSpell(std::size_t& selected,const char* id){
 using namespace TOAM::Presentation;
 if(selected>=spellCount)selected=0;
 std::array<const char*,spellCount> names{};for(std::size_t i=0;i<spellCount;++i)names[i]=L(spellDisplayEN[i],spellDisplayKR[i]);
 int choice=int(selected);
 if(combo){std::string label=std::string(L("Spell###","마법 선택###"))+id;if(combo(label.c_str(),&choice,names.data(),int(names.size()),13)&&choice>=0&&choice<int(spellCount))selected=std::size_t(choice);}
 std::string prev=std::string(L("Previous spell###","이전 마법###"))+id+"Prev",next=std::string(L("Next spell###","다음 마법###"))+id+"Next";
 if(button(prev.c_str()))selected=(selected+spellCount-1)%spellCount;sameLine(0,-1);
 if(button(next.c_str()))selected=(selected+1)%spellCount;
 text(names[selected],nullptr);PresSeparator();
}
void PresentationSound(std::size_t eventIndex){
 using namespace TOAM::Presentation;const auto event=Event(eventIndex);const auto& e=events[eventIndex];auto& id=presentationDraft.selected[eventIndex];
 if(!Compatible(event,id))id=e.original;
 std::array<std::string,sources.size()> labels{};std::array<const char*,sources.size()> items{};std::array<unsigned,sources.size()> ids{};int count=0,choice=0;
 for(unsigned candidate=1;candidate<=sources.size();++candidate)if(Compatible(event,candidate)){
  char label[256]{};std::snprintf(label,sizeof(label),"%03u  %s%s",candidate,L(sources[candidate-1].name,sources[candidate-1].korean),candidate==e.original?L(" [Original]"," [원본]"):"");
  labels[count]=label;items[count]=labels[count].c_str();ids[count]=candidate;if(candidate==id)choice=count;++count;
 }
 std::string widget=std::string(L("Sound###sound","음원 선택###sound"))+std::to_string(eventIndex);
 if(combo){if(combo(widget.c_str(),&choice,items.data(),count,10)&&choice>=0&&choice<count)id=ids[choice];}
 else {text(items[choice],nullptr);std::string prev="<###soundPrev"+std::to_string(eventIndex),next=">###soundNext"+std::to_string(eventIndex);if(button(prev.c_str()))id=ids[(choice+count-1)%count];sameLine(0,-1);if(button(next.c_str()))id=ids[(choice+1)%count];}
}
void RenderAudioEvents(std::size_t selected,bool levels,bool sounds){
 using namespace TOAM::Presentation;bool found=false;
 for(std::size_t i=0;i<eventCount;++i){if(events[i].spell!=Spell(selected))continue;found=true;
  text(L(eventDisplayEN[i],events[i].korean),nullptr);
  if(levels){std::string key=std::string(L("Event volume###event","이벤트 음량###event"))+std::to_string(i);slider(key.c_str(),&presentationDraft.eventVolume[i],0,1,"%.2fx",0);}
  if(sounds)PresentationSound(i);PresSeparator();
 }
 if(!found)text(L("This spell has no separate TOAM audio event in the catalog.","이 마법에는 카탈로그에 구분된 별도 TOAM 효과음 이벤트가 없습니다."),nullptr);
}
void __stdcall RenderAudio(){
 using namespace TOAM::Presentation;Language();PresentationDraft();
 text(L("AUDIO / Existing TOAM sounds","효과음 / 기존 TOAM 음원"),nullptr);
 text(L("Original 1.00x; range 0.00-1.00x; 0=mute. No sound preview.","원본 1.00x / 범위 0.00~1.00x / 0=음소거 / 미리듣기 없음"),nullptr);
 slider(L("Master volume###audioMaster","전체 효과음###audioMaster"),&presentationDraft.master,0,1,"%.2fx",0);PresSeparator();
 static std::size_t selected=0;PresentationSpell(selected,"audioSpell");
 slider(L("Selected spell volume###spellVolume","선택 마법 음량###spellVolume"),&presentationDraft.spellVolume[selected],0,1,"%.2fx",0);
 char state[160]{};const auto active=Get();std::snprintf(state,sizeof(state),L("Active master %.2fx / spell %.2fx","현재 적용: 전체 %.2fx / 마법 %.2fx"),active.master,active.spellVolume[selected]);text(state,nullptr);
 if(beginTabBar&&endTabBar&&beginTabItem&&endTabItem){
  if(beginTabBar("TOAMAudioEvents",0)){
   if(beginTabItem(L("Event volume","이벤트 음량"),nullptr,0)){RenderAudioEvents(selected,true,false);endTabItem();}
   if(beginTabItem(L("Sound selection","음원 교체"),nullptr,0)){RenderAudioEvents(selected,false,true);endTabItem();}
   endTabBar();
  }
 }else RenderAudioEvents(selected,true,true);
 text(L("Original timing, voice limits and fades remain. Hover sound also uses Flight/HumVolume.","기존 재생 시점·동시 재생 제한·Fade를 유지합니다. 부유음은 Flight/HumVolume도 반영합니다."),nullptr);
 PresentationActions(0);
}
void __stdcall RenderCamera(){
 Language();PresentationDraft();text(L("CAMERA SHAKE","화면 흔들림"),nullptr);PresSeparator();
 text(L("Original 1.00x; 0.00x skips TOAM shake requests.","원본 1.00x / 0.00x는 TOAM 흔들림 요청 생략"),nullptr);
 if(slider(L("Intensity###shakeMaster","흔들림 강도###shakeMaster"),&presentationDraft.shake,0,3,"%.2fx",0))presentationDraft.shake=std::round(presentationDraft.shake*20.F)/20.F;
 char state[160]{};std::snprintf(state,sizeof(state),L("Active %.2fx / draft %.2fx","현재 적용 %.2fx / 편집 %.2fx"),TOAM::Presentation::Get().shake,presentationDraft.shake);text(state,nullptr);
 text(L("Step 0.05x. Duration/timing unchanged. Skyrim caps final strength at 1, so high values can saturate.","간격 0.05x. 지속시간·시점 유지. 게임의 최종 강도 상한으로 고배율 체감이 포화될 수 있습니다."),nullptr);PresentationActions(1);
}
bool ChooseCircleColor(TOAM::Presentation::Color& color,const std::string& owner){
 using namespace TOAM::Presentation;bool changed=false;
 if(color>=Color::Count)color=Color::Default;
 for(std::size_t c=0;c<colorKeys.size();++c){
  if(c)sameLine(0,-1);
  const auto rgb=colorRGB[c];const std::string suffix=owner+std::to_string(c);
  if(c&&colorButton){
   const std::string id=std::string(colorKeys[c])+"###circleSwatch"+suffix;
   if(colorButton(id.c_str(),MenuVec4{rgb.red,rgb.green,rgb.blue,1.F},0,MenuVec2{30.F,24.F})){color=Color(c);changed=true;}
   sameLine(0,-1);
  }
  const std::string label=(color==Color(c)?"[x] ":"")+std::string(L(colorKeys[c],colorKR[c]))+"###circleColor"+suffix;
  if(button(label.c_str())){color=Color(c);changed=true;}
 }
 return changed;
}
void RenderAllCircleColors(){
 using namespace TOAM::Presentation;static Color all=Color::Default;
 text(L("Select a swatch to set the draft color for all 13 spells.","색을 선택하면 13종 마법의 편집값에 일괄 반영합니다."),nullptr);
 if(ChooseCircleColor(all,"All"))presentationDraft.colors.fill(all);
 const auto active=Get();bool same=true;for(auto c:active.colors)if(c!=active.colors[0])same=false;
 std::string state=std::string(L("Active: ","현재 적용: "))+(same?L(colorKeys[std::size_t(active.colors[0])],colorKR[std::size_t(active.colors[0])]):L("Different per spell","마법마다 다름"));
 text(state.c_str(),nullptr);
 text(L("Default restores each spell's own original appearance.","원본(Default)은 모든 마법을 각자의 원래 외형으로 복원합니다."),nullptr);
}
void RenderIndividualCircleColors(){
 using namespace TOAM::Presentation;static std::size_t page=0;constexpr std::size_t perPage=5,pages=(spellCount+perPage-1)/perPage;
 text(L("Five spells per page. Choose each spell's color below.","한 페이지에 최대 5개 마법을 표시합니다. 각 마법의 색을 선택하십시오."),nullptr);
 for(std::size_t p=0;p<pages;++p){if(p)sameLine(0,-1);const auto end=(p+1)*perPage<spellCount?(p+1)*perPage:spellCount;
  std::string label=(page==p?"[x] ":"")+std::to_string(p*perPage+1)+" - "+std::to_string(end)+"###colorPage"+std::to_string(p);
  if(button(label.c_str()))page=p;
 }
 PresSeparator();const auto active=Get();
 for(std::size_t i=page*perPage;i<spellCount&&i<(page+1)*perPage;++i){
  auto& c=presentationDraft.colors[i];if(c>=Color::Count)c=Color::Default;
  std::string line=std::string(L(spellDisplayEN[i],spellDisplayKR[i]))+" - "+L(colorKeys[std::size_t(c)],colorKR[std::size_t(c)]);
  if(c!=active.colors[i])line+=L(" (unsaved)"," (저장 전)");
  text(line.c_str(),nullptr);ChooseCircleColor(c,"Spell"+std::to_string(i)+"_");PresSeparator();
 }
}
void __stdcall RenderColors(){
 using namespace TOAM::Presentation;Language();PresentationDraft();text(L("CIRCLE COLORS","마법진 색상"),nullptr);
 text(L("Default keeps the original v1.0.0 artwork. Swatches show preset tints.","Default는 v1.0.0 원래 외형입니다. 색상 견본은 프리셋 색조를 표시합니다."),nullptr);
 if(beginTabBar&&endTabBar&&beginTabItem&&endTabItem){
  if(beginTabBar("TOAMCircleColors",0)){
   if(beginTabItem(L("All spells","전체 색상 변경"),nullptr,0)){RenderAllCircleColors();endTabItem();}
   if(beginTabItem(L("Individual spells","마법별 개별 변경"),nullptr,0)){RenderIndividualCircleColors();endTabItem();}
   endTabBar();
  }
 }else{
  static bool individual=true;
  if(button(L("All spells###colorAllTab","전체 색상 변경###colorAllTab")))individual=false;sameLine(0,-1);
  if(button(L("Individual spells###colorIndividualTab","마법별 개별 변경###colorIndividualTab")))individual=true;
  if(individual)RenderIndividualCircleColors();else RenderAllCircleColors();
 }
 text(L("Apply and save below. Existing circles retain their cast snapshot; new casts use saved colors.","아래 적용 및 저장을 눌러 확정합니다. 기존 마법진은 유지하고 다음 시전부터 적용합니다."),nullptr);
 text(L("Glow and final brightness depend on each spell and game lighting.","실제 발광·밝기는 마법과 게임 조명에 따라 달라집니다."),nullptr);PresentationActions(2);
}
