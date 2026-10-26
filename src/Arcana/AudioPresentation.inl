std::array<RE::BSSoundHandle,4> ultimateVoices{};unsigned ultimateVoiceCursor=0;
void ClearUltimateAudio(){for(auto& voice:ultimateVoices){if(voice.IsValid())static_cast<void>(voice.Stop());voice=RE::BSSoundHandle{};}}
// Engine fades the existing loop. A copy retains the original ID for bounded cleanup.
struct FadingSound { RE::BSSoundHandle handle{};std::chrono::steady_clock::time_point expires{};bool owned=false; };
std::array<FadingSound,4> fadingSounds{};unsigned fadingCursor=0;
void ReleaseFading(FadingSound& voice){
 if(voice.owned&&voice.handle.IsValid())static_cast<void>(voice.handle.Stop());
 voice=FadingSound{};
}
void ClearFadingAudio(){for(auto& voice:fadingSounds)ReleaseFading(voice);}
void FadeSustain(RE::BSSoundHandle& loop){
 if(!loop.IsValid())return;
 const auto now=std::chrono::steady_clock::now();
 // Prune on actual audio events, with no new polling hook or runtime collector.
 for(auto& voice:fadingSounds)if(voice.owned&&voice.expires<=now)ReleaseFading(voice);
 auto& voice=fadingSounds[fadingCursor++%fadingSounds.size()];ReleaseFading(voice);
 voice.handle=loop;voice.owned=true;voice.expires=now+std::chrono::milliseconds(sustainFadeMS);
 auto releaseRequest=loop;
 const bool accepted=releaseRequest.FadeOutAndRelease(sustainFadeMS);
 if(!accepted)ReleaseFading(voice);
 loop=RE::BSSoundHandle{};
 logger::info("SUSTAIN_FADE_REQUEST accepted={} ms={}",accepted,sustainFadeMS);
}
