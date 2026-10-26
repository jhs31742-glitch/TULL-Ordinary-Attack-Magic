# Static SSE ribbon repair / 정적 SSE 리본 교정
Python standard-library tool: `python repair_sse_ribbon.py input.nif candidate.nif --shape 10_Hanging_Red_Ribbon`.
It refuses in-place writes. It supports only inspected SSE20.2.0.7/User12/Stream100 static BSTriShape data with the specific position/UV/normal/tangent layout and a closed two-manifold without split seam vertices. Unsupported/skinned/open/non-orientable geometry fails closed. Do not apply this to arbitrary cloth,actors or a previously skinned weapon. A no-conflict shape is returned unchanged.

The triangle graph constrains adjacent edge traversal directions. Propagate orientation parity per component,choose outward signed volume,recalculate area-weighted vertex normals and a UV-derived orthogonal tangent frame,then replace only the owned raw fields. Every other NIF byte remains exact. No double-sided shader workaround,texture rewrite,library Save/normalization,physics,helper or gameplay change.
`nifly_mesh_audit.cpp` is an independent read-only geometry export utility. It uses external nifly headers/library (upstream GPLv3;not bundled). Follow upstream license obligations if distributing a linked audit binary. This source package does not contain that binary or imported game assets.

정점 위치·UV·재질·장착 원점·BSX·충돌을 보존하고 삼각형 방향과 법선/접선 필드만 교정합니다. 출처/원본 해시와 현재 활성 설치본을 먼저 대조하고,후보를 독립 NIF 리더로 재읽은 후 백업·배포·인벤토리를 하나의 트랜잭션으로 갱신합니다. 본 도구는 배포 트랜잭션이나 게임 화면 검증을 대신하지 않습니다. 페른/Silver는 UV 이음새를 포함하며 별도 용접 위상 감사에서 동일 결함이 없음을 확인했으므로 원본을 유지했습니다.

CP032 evidence:Gold closed mesh300vertices/596triangles;152same-direction shared edges and0back projected area before;296triangles reversed,0conflicts and balanced26.0435/26.0435front/back projected area after.Exact changed bytes2582perNIF. See Docs/CP032_STAFF_CLOTH_REPORT.md for actual hashes,reference mappings and QA. Preserve CP031 gameplay23/23 USER_PASS separately from CP032 cloth NOT_RUN.
