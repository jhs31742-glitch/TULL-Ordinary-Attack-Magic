# TOAM 1.0.1 CIRCLE_MATERIAL_AUDIT
Actual CP032 Production audit:23 circle-family NIFs,116 BSEffectShaderProperty blocks,9 referenced DDS.16 current AS_CirclePose engine models contain87 properties;7 preserved legacy circle models contain29. Arcana_Burst has AS_CirclePose but is an explosion,excluded by actual runtime role. No model,texture,geometry,UV,collision,rotation,placement or ESP is rewritten/deleted. Full per-block shader/shape hashes,flags,RGBA,gain,NiAlphaProperty,controller and actual ESP references are in CIRCLE_MATERIAL_AUDIT.json.

|NIF|Shaders|Alpha properties|Controllers|AS_CirclePose|Actual ESP references|SHA256|
|---|---:|---:|---:|---|---:|---|
|meshes/TOAM/Arcana/Circles/Arcana_Chariot.nif|5|5|0|True|2|2C8D86128CB2C752D6918EB7128E92194C50922B97DFC017DD8607361299C0D8|
|meshes/TOAM/Arcana/Circles/Arcana_Emperor.nif|6|6|0|True|322|7CCB2542FF3C25C9776DA31DFC44FE444BBDE422CF621640FDB4AD8E032A5ACE|
|meshes/TOAM/Arcana/Circles/Arcana_Field_Band.nif|3|3|0|True|8|E265A294FCD8B891779D3A1E2FEA21D9EB8EEFC96A00A2E9D356C56DFFE9AD41|
|meshes/TOAM/Arcana/Circles/Arcana_Hermit.nif|6|6|0|True|2|C2E3A312E82B03CB9DB912042023C4FE6151B1F6761CC731FAA80171CB45ED6D|
|meshes/TOAM/Arcana/Circles/Arcana_Justice.nif|7|7|0|True|2|F0F575DCF82571F2662DAF62B8304B0C0F2D3ED659FC015F955BF2B8C6EDDB9D|
|meshes/TOAM/Arcana/Circles/Arcana_Star.nif|7|7|0|True|2|A3A126A954019B8D0AE07D0C32143564E0DC3068632B9370B67F667BCCF038D6|
|meshes/TOAM/Arcana/Circles/Arcana_Strength.nif|6|6|0|True|2|A466E7DEB6AE020C9AB71CEE7F971EBC229FBE23673144E0F17F76A774CBA5DE|
|meshes/TOAM/Arcana/Circles/Arcana_Sun.nif|7|7|0|True|2|E1D2FA1F2AF3FCC31E33B659C858F70D77F9247D264B8B4CA6469B828FA814FF|
|meshes/TOAM/Arcana/Circles/Arcana_Sun_Full.nif|6|6|0|True|48|B36FA1ED3094B50B0A777001ADFF544FDCA32D01300474B7BEA2AB76D3F343A5|
|meshes/TOAM/Arcana/Circles/Arcana_Sun_OuterBand.nif|3|3|0|True|48|E818F544600DAB68B0DB439C7467A3E3EC817633C13DBB8E64CCF9EE5C4F6225|
|meshes/TOAM/Arcana/Circles/Arcana_Tower.nif|7|7|0|True|2|B249A415B14C92ED3102FEBFBDC28105DCC0E7CFEA41CC284D71B423078B15E5|
|meshes/TOAM/Arcana/Circles/Arcana_Tower_Sky_Thin.nif|6|6|0|True|8|660F67D3FED343B0D98D897F6FEFD2810B627E088039BFE52BB8F0EA5DD0AA4D|
|meshes/TOAM/Arcana/Circles/Arcana_World_Harmonized.nif|6|6|0|True|32|A5C3625B0DDDB07F9D483C8BF81F627E213C6F4CA725CB34C1A097101214BB38|
|meshes/TOAM/Arcana/Flight/Arcana_Fool_AscentBand.nif|3|3|0|True|4|51704849ECEA56315BE3BEAD5B0B3210CADBA48772BCCE32421C7404E0982446|
|meshes/TOAM/VFX/Circles/Arcana_Temperance_Halo.nif|3|3|0|False|1|2AB9ECA432088ABDD70FBA4A1EC422C2987684E38E8A377EBDC9AB2079A38950|
|meshes/TOAM/VFX/Circles/Arcana_Temperance_Halo_Engine.nif|3|3|0|True|1|D673776063B70A3C7247A769B3F76D6D67409A7A0E7D6F04DBA6061DD4338D29|
|meshes/TOAM/VFX/Circles/TOAM_Circle_Animated.nif|5|5|5|False|1|5860CB9950C8D3F04CABDA442646B02A17C0545F1DC82B3E468A92C938AFED44|
|meshes/TOAM/VFX/Circles/TOAM_Circle_Core_Animated.nif|6|6|6|False|0|BC7C70DC1F07338FDC7EFCED8BFC606049AEF7B608D617C021360EF1493D314E|
|meshes/TOAM/VFX/Circles/TOAM_Circle_Core_Static.nif|6|6|4|False|0|9C1CEEE90C3438FB581670D5744A7E17D4EE0CEEFAD08C445C404BF5AA65C7C3|
|meshes/TOAM/VFX/Circles/TOAM_Circle_OuterBand_Static.nif|2|2|0|False|0|DCB600BF2C08DBCAC5B63D73DA6864D1DEFF08ABDC918CDD11717A4E9E36439C|
|meshes/TOAM/VFX/Circles/TOAM_Circle_Small.nif|2|2|0|False|0|ACB3E20A45482DAC43597B16B368B2D20549ABA939D9B07ACF6934AED9FE68C0|
|meshes/TOAM/VFX/Circles/TOAM_Circle_Static.nif|5|5|3|False|1|915EEB7C94EA790FA871F0808F316FFEBB6D56FFB4978542C974D8521957D598|
|meshes/TOAM/VFX/Circles/TOAM_Engine_Circle.nif|6|6|3|True|128|F11F1D92405685DBA6FAB5C28C7AA3405CE2C6E69A3DE55765D3FE9E2196A2CE|

## Arcana Neutral mapping
Nondefault runtime instances reuse existing Arcana_Circle_Face/Rings/Glow_Soft/Hebrew_Band DDS. TOAM corresponding four blue DDS have pixel-identical alpha at the audited full-resolution decoded plane;their RGB differs. Full original DDS bytes and mipmaps remain unchanged;no all-mip alpha equality is inferred from the full-resolution test. Existing neutral smoke sidewalls remain on the same neutral smoke texture. No new color NIF/DDS families.
Default returns before traversal,cloning,texture replacement or RGB mutation. Thus original shared texture paths,RGB,alpha,gain,controller chain and model bytes remain unchanged exactly,including each spell's different original appearance. Legacy models are preserved as original exceptions;no active new owner is added for them. Shape differences are not normalized.

|Original blue DDS|Existing neutral DDS|Full-resolution alpha|RGB identical|
|---|---|---|---|
|textures\TOAM\VFX\TOAM_Circle_Face.dds|textures\TOAM\VFX\Arcana_Circle_Face.dds|PIXEL_IDENTICAL|False|
|textures\TOAM\VFX\TOAM_Circle_Rings.dds|textures\TOAM\VFX\Arcana_Circle_Rings.dds|PIXEL_IDENTICAL|False|
|textures\TOAM\VFX\TOAM_Glow_Soft.dds|textures\TOAM\VFX\Arcana_Glow_Soft.dds|PIXEL_IDENTICAL|False|
|textures\TOAM\VFX\TOAM_Hebrew_Band.dds|textures\TOAM\VFX\Arcana_Hebrew_Band.dds|PIXEL_IDENTICAL|False|

## Native ownership and lifetime evidence
Pinned CommonLib declares NiObject::Clone,BSEffectShaderProperty::SetMaterial,BSEffectShaderMaterial fields,NiPointer and BSShaderManager::GetTexture. On the actual SE1.5.97 unpacked executable matched to Address Library:property CreateClone(vtable304580 slot0x17,RVA12D4950) allocates a fresh0xA0 property despite the SDK's null-implementation comment. SetMaterial(SE98897,RVA1291D40) reaches the material manager with a unique seed,then actual Create/CopyMembers. Effect material Create/CopyMembers at12FE670/12FE6E0 retains immutable texture references and copies RGBA/gain/falloff fields;destructor owns the0x88 allocation. GetTexture(SE98986) reuses cached immutable resources. Raw binary/function evidence remains private in CP033 stage002;no game executable/address bin is redistributed.
Nondefault Apply runs at the existing first-ready pose,once per new root. Bounded traversal:max32 geometry,256 nodes,depth24. Each geometry must have one owner reference. Property RTTI/type and source texture are checked;Clone pointer and SetMaterial(true) material must differ,DoIsCopy/flags/alpha/gain must match. Animated alpha controllers must be distinct clones targeting the private property with equal timing/flags. Unexpected sharing/unknown texture/controller rejects the entire plan and leaves original appearance. No global material or cached prototype mutation.
All clones are prepared before assignment. RAII restores assigned properties on partial setup failure;SetupGeometry must keep the private material identity. Per-owner state freezes the preset,tracks attempted root,and resets with slot creation/finish. Existing retired/queued owners carry state;new generations receive new state. Existing CircleAlpha continues its original per-instance node fade updates after tint;no alpha-property edits are introduced. No per-frame material clone or repeated RGB write. Immutable textures may remain shared safely.
Native implementation/static guards are verified. Simultaneous real engine instance independence,cache reuse after destruction,live controllers and visible color are USER_RUNTIME NOT_RUN. A runtime guard rejection deliberately preserves original appearance and logs its reason;it must not be reported as successful color rendering. No preset asset fallback is silently generated.

## Resource budget and measurements
Plan uses32 bounded entries on stack and at most one new property/material per existing geometry for a nondefault owner;Default creates none. Actual C++ sizeof measurements and simultaneous slot budget are recorded separately in build evidence. Texture handles add four persistent references to pre-existing resources;no duplicate DDS data is authored. Runtime per-apply cpu_us instrumentation uses steady_clock in existing application path. No game CPU/GPU/FPS or live heap measurement has been made;do not infer them from these bounds.

Host MSVCx64 sizeof measurement:Values212 bytes,ColorState16 bytes,three-pointer plan entry24 bytes. Existing193 engine slots plus351 Arcana ArtSlot owners carry544 color states(8704 bytes of state fields,before surrounding structure padding). This is a static state-field count,not measured live heap usage. Native base property0xA0+effect material0x88=296 bytes per cloned geometry,before engine/controller/allocator overhead. Typical prepared geometry counts:Magician eight mixed full/outer circles36,World12 circles72,Judgement9artillery54 plus one6-property guide. These are resource bounds from actual assets,not measured simultaneous engine ownership. Plan refuses more than32 geometries in any root. Actual CPU/log measurements require user runtime;no FPS reported.

CP033 final material ownership refinement:actual pinned ModelReferenceEffect owns artObject3D through NiPointer at+C8. Root references must equal that effect owner plus exactly one verified parent child reference when attached,parent child scan is bounded256,and each intermediate NiNode must have one reference. Geometry alone cannot exclude a shared ancestor. These checks precede property/material clone/assignment;extra temporary/cache/active ownership rejects that instance and preserves original appearance. Default remains an immediate return. No model/texture/alpha/geometry/timing change. Live simultaneous engine behavior remains USER_RUNTIME NOT_RUN.
