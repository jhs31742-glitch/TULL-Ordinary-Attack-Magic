# TULL Ordinary Attack Magic v0.8.11

Install the Main archive as the MO2 mod TULL Ordinary Attack Magic.
Required runtime: Skyrim SE1.5.97, matching SKSE, Address Library and Open Animation Replacer. AE/VR runtime support is not enabled.
Enable TULL Ordinary Attack Magic.esp. The only native host is SKSE/Plugins/TOAM.dll; settings TOAM.ini; log OrdinaryAttackMagic.log.
Disable old TOOM/independent Arcana native installations to avoid duplicate hosts.

Stances is an optional separate download. Install its archive only with the main mod and Stances - Dynamic Weapon Movesets SE.esp. The TOAM addon has exactly these hard masters: Skyrim.esm; TULL Ordinary Attack Magic.esp; Stances - Dynamic Weapon Movesets SE.esp. The installed Stances plugin also requires Update.esm, Dawnguard.esm, Dragonborn.esm and Keytrace.esp. Follow that mod's dependency instructions and sort all masters before their dependents.
The addon provides High Priestess/Moon spells and40000-charge Unbound staffs. Cast mode is fixed when a cast starts; the next cast uses the next stance.

English ESPs are installed at the mod root and also retained in Language/en. Korean versions are in Language/kr. Copy the chosen language's corresponding ESPs to the root. Keep the language folders for switching back; install matching-language main and addon files. Language copies change display text only.
Attack books: Ordinary Attack Magic: Arcana - Card / 일반공격마법: 아르카나 - 카드명.
Levitation book: Ordinary General Magic: Arcana - Fool (Levitation) / 일반교양마법: 아르카나 - 광대 (공중부양).
Ring: Arcana Limiter / 술식 출력 제한 반지.

Staffs consume their own charge, never player Magicka. Ordinary capacity20000; Unbound capacity40000. Recharge using soul gems. Original unique enchantments remain protected.
All eight Arcana base costs were tripled in0.8.11; vanilla Destruction cost reductions still apply, including zero final cost. TOAM Magician/Judgement/World/Temperance numbers remain unchanged. See BALANCE.md.

Staff SMP is withdrawn and excluded. Disable earlier Staff SMP options and their ESP/patch before testing this static-cloth release. Experimental authoring remains in the private development history.
CP028 core gameplay was reported successful by the user. CP029 resource/damage changes and restored addon master have not received game testing. Existing158 QA questions are preserved; see RELEASE_NOTES.md for the focused remaining checks.
The GitHub repository stays private until the Nexus release. Do not publish the binary package until imported-asset permissions are resolved. Credits and provenance are in this folder.
