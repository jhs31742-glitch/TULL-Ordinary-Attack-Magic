# TULL Formation Editor 1.0

Open **index.html** in a modern desktop browser. It works offline; keep engine.js, presets.js and app.js beside it. No installation, Skyrim, plugin, NIF reader, external service or internet connection is needed. Choose English or 한국어 at the top. The program and these manuals support both languages.

## Named sets → spells → casting variants → assets
A named set contains multiple spells. Each spell owns its actor/target/world anchors and one or more casting variants. Each variant has an independent asset list. Add, rename, duplicate or delete spells. Duplicate/rename variants to represent single-left, single-right, both hands, airborne or other contexts. Add/remove/duplicate circles, illustrative blades and markers freely. Counts in this authoring file do not change an installed mod's hardcoded gameplay counts.

Use **Save named set** to store several named libraries in this browser. The same name replaces that local entry. Export set JSON is the portable backup and contains ALL spells and variants, including hand ownership, names, groups, positions, scale and orientation. Local storage is browser/device/path dependent and may be cleared; keep exported files. Import replaces the current workspace and can be undone. New set starts a separate library. JSON is unminified, versioned and intended for other AIs and mod tools.

## Selection and transformations
Click centers or use the multiple-selection list. Ctrl-click toggles individual selection. Drag empty space to make a selection rectangle; Shift adds to the selection. Drag a selected center to move all selected assets together. Front moves X/Z, side Y/Z, top/oblique X/Y. Optional snapping uses5 game units. Overlapping Sun layers are easier to select through the list or Select group. Group names are stored per asset.

Inspector XYZ position and rotation edits apply the same delta to all selected assets. Rotation is local to each asset's center, not an orbital rotation of the whole group's positions. Source normal sets the plane basis; XYZ degrees then rotate that basis. Positive axes/right-handed rotations and the exact order are documented below. Scale sets the same absolute scale on selected objects; **Selection scale ×** multiplies each existing scale, preserving the Sun's relative layer sizes. Circle radius and blade/box XYZ dimensions describe the SOURCE asset before scale. Model path is an authoring label, not an imported NIF. Current geometry previews are schematic outlines, not final art.

Undo/redo cover set/spell/variant creation, counts and transformations. Ctrl-Z/Ctrl-Shift-Z also work outside text fields. Delete removes selected assets. Fit views changes only the editor camera. It does not change any formation position.

## Coordinate and runtime contract
- X=actor-right, Y=actor-forward, Z=up. Positions use Skyrim game units; rotation fields use degrees. Do not silently treat them as metres or radians.
- A frame has world position, yaw, reference height, pivot(feet or bounds-center) and an optional offset. Center pivot adds height/2 only as an editor reference. Runtime target bounds.center should replace that approximation.
- Local → world: origin + yawBasis × local. Skyrim yaw uses worldX=cos(yaw)*x+sin(yaw)*y and worldY=-sin(yaw)*x+cos(yaw)*y. Yaw fields are degrees; convert once at the boundary. See engine.js for tested inverse transforms.
- Circle normal axis is local+Y. BaseNormalFrame is right-handed; orientation=BaseNormalFrame * Rz * Ry * Rx. Editor XYZ rotations apply Rx first. Preserve all three axes/roll when changing anchors, not only the normal.
- Changing actor/target yaw moves attached local layouts. Freeze to world bakes the complete position and orientation so later actor/target motion does not drag released effects. The world anchor should stay at its default zero origin/yaw for actual global coordinates.
- Rendering-parent conversion must use the real parent world inverse: local translation=parent.rotate.transpose*(desiredWorld-parent.translate)/parent.scale; local rotation=parent.rotate.transpose*desiredRotation; local scale=desiredScale/parent.scale. Reject zero/invalid parent scale. Apply after engine3D and pose-node readiness, retaining per-instance ownership and cleanup.
- Geometry/aim and collision endpoints must share an explicitly chosen authority. Never let an aim target replace a player-centered formation origin. Editor exports do not call ApplyArtObject, alter ESPs or implement game hooks.

## Reviewed TOAM presets and source history
presets/toam-reviewed.json contains Sun full charge, Magician VIII, ground World/Hermit, airborne World full sphere, target-centered Judgement/Temperance reference, Emperor/Chariot/Strength hand variants, and an illustrative back-blade example. Actual manually edited secondary coordinates differ from a simple mirrored copy. AirWorldNormal has lower-hemisphere directions and some circle centers below the airborne actor's feet.

reference/current-runtime-layout.json and the exact manual-layout-20261023-0145.json preserve earlier source/export evidence. reference/source-parameters.json records current local scales, radius formulas and source hashes. The historical original editor remains in the complete mod source at tools/layout_editor; this independent package contains the improved editor and numeric references only. It does not bundle Skyrim meshes, textures, animations or third-party reference pictures.

The target preview uses a128u sample shell, appropriate to Temperance. Judgement's real radius depends on initial target bounds.radius*profile radiusScale and its display scale adapts. The source-backed normalized9 points are exact, but this sample radius is not a universal NPC size. World preserves4/8/12 stage policy in the game; this general tool permits other counts for other mods. Full-charge Sun gap7 differs from the animated22→7 gap; JSON captures an authoring snapshot, not the whole Native timeline.

## Other-mod integration / AI handoff
Read SCHEMA.md and the tests before extending the tool. Select a spell and variant from actual cast context; do not concatenate both single/dual lists or charge resources from asset count by accident. hand=left/right/shared is explicit metadata; asset IDs are unique within a variant. Group names are authoring selections and do not imply runtime ownership sharing.

For the example “light attacks create summoned swords behind the actor, heavy attacks launch toward an enemy”: author actor-local follow positions in one variant; on the light-hit event create bounded owned visuals; on heavy attack snapshot their WORLD poses and a valid live target position, launch using the appropriate game system, then retire visuals. Native hit filtering, damage/resource policy, timing, save/load/cell/death cleanup and a launch owner must be implemented separately. This example is not a shipped TOAM feature or combat-event hook.

Do not claim runtime PASS from this editor. Check per-asset engine3D readiness, world/local conversion, actual NIF size/normal axis, visibility and cleanup in the target runtime. Validate JSON and keep original source hashes and authoring assumptions when changing a preset.

Run tests: node tests/engine.test.cjs. The unminified code and numeric examples are included so other developers and AIs can inspect and adapt them. Public redistribution licensing remains a publisher decision; do not infer rights over the game/mod's unrelated art from this editor package. TULL supplied design/manual coordinates; OpenAI Codex assisted with implementation, tests and documentation.

Default controls use mouse dragging and position/rotation/scale SLIDERS. Exact coordinates,normals,radius and dimensions are collapsed under Detailed numeric inputs. Open it only for precision work. Slider drags are grouped into one undo action;the same underlying JSON values are updated.

## AI coordinate reference JSON
Import `examples/ai-coordinate-examples.json` for compact actor-front, 90-degree yaw/world check, target snapshot and back-sword/hand examples. `examples/ai-reference-guide.json` is machine-readable documentation, not an editor project. For actual TOAM values use `presets/toam-reviewed.json`; source hashes and preview assumptions are explicit.
