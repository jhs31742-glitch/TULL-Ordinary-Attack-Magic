# QA previews for the Neutral/Tintable set: software "runtime tint" = per-mesh RGB multiplier on the neutral texture. Writes Previews/ + Docs/TINTABLE_QA.md
import os, sys, numpy as np
from PIL import Image, ImageDraw
HERE=os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0,HERE)
import previews as P
ROOT=os.path.normpath(os.path.join(HERE,"..",".."))
R2={"core":(1,1,1),"main":(.749,.902,1.0),"out":(.31,.659,1.0),"aft":(.561,.89,1.0)}
PALETTES={"Blue (r2 palette)":R2,
 "Red":{"core":(1,1,1),"main":(1.0,.78,.74),"out":(1.0,.23,.18),"aft":(1.0,.55,.50)},
 "Purple":{"core":(1,1,1),"main":(.89,.79,1.0),"out":(.61,.30,1.0),"aft":(.78,.61,1.0)},
 "Gold":{"core":(1,1,1),"main":(1.0,.91,.69),"out":(1.0,.72,.11),"aft":(1.0,.84,.42)},
 "Green":{"core":(1,1,1),"main":(.82,1.0,.80),"out":(.36,.86,.30),"aft":(.60,.95,.55)}}
for _p in PALETTES.values(): _p["mid"]=tuple((a+b)/2 for a,b in zip(_p["main"],_p["out"]))   # saturated tint for single-tint baked-layout meshes
BAKED={"Circle_Rings","Circle_Face","Hebrew_Band","Circle_Small","Impact_Rings"}
def roles(r2name):
    meshes,_=P.parse(P.SPEC+r2name+".nifspec"); out={}
    for m in meshes:
        c=tuple(round(float(x),3) for x in m["col"]); base=os.path.basename(m["tex"])[:-4]
        out[m["name"]]="baked" if (c==(1,1,1) and base in BAKED) else "core" if c==(1,1,1) else next(k for k,v in R2.items() if tuple(round(x,3) for x in v)==c)
    return out
_roles={}
def tintfn(r2name,pal,baked="mid"):
    if r2name not in _roles: _roles[r2name]=roles(r2name)
    return lambda m: pal[baked if _roles[r2name][m["name"]]=="baked" else _roles[r2name][m["name"]]]
EXPO=0.75   # preview exposure (renderer default 1.4 clips the pale palettes to white)
def shots(pal,bg=(10,26,68),neutral=True,cell=320,baked="mid"):
    sfx="_Tintable" if neutral else ""; pre="Tintable/" if neutral else ""
    def spec(n): return pre+n+sfx
    r=lambda n,*a,**k: P.render(spec(n),*a,tintfn=tintfn(n,pal,baked),bg=bg,expo=EXPO,**k)
    return [r("Charge_Circle_Core_Static","front",0.76,cell,cell), r("Charge_Circle_Small","front",0.76,cell,cell),
            r("Beam_Body_Unit_Static","side",0.2,cell,cell,3,lat={"BEAM_":2.2},ox=6), r("Impact_High_Compact","front",1.35,cell,cell)]
def sat(img):   # brightness-weighted mean saturation of lit pixels
    a=np.array(img).astype(float)/255; mx=a.max(2); mn=a.min(2); lit=mx>0.15
    return float(((mx-mn)/np.maximum(mx,1e-6))[lit].mean()), float((mx[lit]).mean())
def decl(name): return f"{name}"
cell=320; names=["Charge_Circle_Core_Static","Charge_Circle_Small","Beam_Body_Unit_Static (3 seg, width x2.2 for visibility)","Impact_High_Compact"]
rows=list(PALETTES.items()); H=34+len(rows)*(cell+4)+ (cell+4+30); W=4*cell+3*4+0
sheet=Image.new("RGB",(W,H),(10,26,68)); d=ImageDraw.Draw(sheet)
for i,n in enumerate(names): d.text((8+i*(cell+4),10),n,fill=(191,230,255))
y=34; qa=["# TINTABLE_QA (자동 생성: Source/scripts/previews_tintable.py)\n","소프트웨어 렌더(가산, 정지 포즈) 기준의 정성/정량 검증입니다. 인게임 결과가 아닙니다.\n"]
black=(0,0,0); sat_rows=[]
for label,pal in rows:
    for i,im in enumerate(shots(pal)): sheet.paste(im,(i*(cell+4),y))
    d.text((8,y+cell-16),label+"  [baked-layout meshes tinted with mid=(main+out)/2]  main/out/aft="+"/".join("#%02X%02X%02X"%tuple(int(round(c*255)) for c in pal[k]) for k in ("main","out","aft")),fill=(255,255,255)); y+=cell+4
    # metrics on black background
    ims=shots(pal,bg=black); circ=ims[0]; s,b=sat(circ); beam=np.array(ims[2]); cy=beam.shape[0]//2; core=beam[cy-1:cy+2,60:260].reshape(-1,3).mean(0)
    sat_rows.append((label,s,b,core))
# reference row: r2 baked blue-white textures multiplied by the Red tint (what a naive runtime multiply would give)
red=PALETTES["Red"]; naive=shots(red,neutral=False); d.text((8,y+4),"REFERENCE: r2 baked blue-white textures x Red tint (naive multiply): blue lines remain",fill=(255,200,190)); 
for i,im in enumerate(naive): sheet.paste(im,(i*(cell+4),y+24))
sheet.save(P.OUTD+"Tintable_Palette_Test.png")
sn=sat(shots(red,bg=black)[0]); sb=sat(shots(red,neutral=False,bg=black)[0])
qa.append("## 1. 팔레트별 (Charge_Circle_Core_Static_Tintable, 검정 배경)\n\n| 팔레트 | 평균 채도(밝은 픽셀) | 평균 밝기(0~1) | 빔 중심선 평균 RGB (백색 코어 유지 확인) |\n|---|---|---|---|")
for label,s,b,core in sat_rows: qa.append(f"| {label} | {s:.3f} | {b:.3f} | ({core[0]:.0f}, {core[1]:.0f}, {core[2]:.0f}) |")
qa.append(f"\n## 2. 평균 채도/밝기 (Red 틴트, Circle Core Static, 검정 배경)\n\n| 입력 | 평균 채도 | 평균 밝기 |\n|---|---|---|\n| Neutral 텍스처 x Red 틴트 | {sn[0]:.3f} | {sn[1]:.3f} |\n| r2 청백 텍스처 x Red 틴트 (단순 곱) | {sb[0]:.3f} | {sb[1]:.3f} |\n\n전체 평균으로는 두 값이 비슷합니다. 차이는 평균 채도가 아니라 표 3의 청색 잔색에서 드러납니다.\n")
def bluedom(img):   # share of lit pixels whose blue channel clearly exceeds red (blue residue)
    a=np.array(img).astype(int); lit=a.max(2)>38; return float(((a[...,2]>a[...,0]+12)&lit).sum()/max(lit.sum(),1))
nm=["Circle Core Static","Circle Small","Beam Static","Impact High Compact"]; resid={}
for pn in ("Red","Gold","Green"):
    resid[pn]=([bluedom(i) for i in shots(PALETTES[pn],bg=black)],[bluedom(i) for i in shots(PALETTES[pn],bg=black,neutral=False)])
qa.append("## 3. 청색 잔색 테스트 (밝은 픽셀 중 B 채널이 R보다 12 이상 큰 픽셀의 비율. 0에 가까울수록 청색이 안 남음)\n\n| 팔레트 | 에셋 | Neutral x 틴트 | r2 청백 x 틴트 (단순 곱) |\n|---|---|---|---|")
for pn,(a,b) in resid.items():
    for n,x,y2 in zip(nm,a,b): qa.append(f"| {pn} | {n} | {x*100:.1f}% | {y2*100:.1f}% |")
qa.append("\n빔은 두 경우 모두 텍스처가 순백(Beam_Streak)이라 차이가 없습니다. 마법진과 폭발 링은 r2 텍스처에 청색이 구워져 있어 적색/금색/녹색 틴트에도 청색 선이 남고, Neutral은 이 잔색이 0%입니다.\n")
# Neutral + r2 colours vs r2 original
mine=shots(R2,bg=black,baked="main"); orig=[P.render(n,v,sc,cell,cell,*([3] if "Beam" in n else []),**kw,bg=black,tintfn=None,expo=EXPO) for n,v,sc,kw in [("Charge_Circle_Core_Static","front",0.76,{}),("Charge_Circle_Small","front",0.76,{}),("Beam_Body_Unit_Static","side",0.2,{"lat":{"BEAM_":2.2},"ox":6}),("Impact_High_Compact","front",1.35,{})]]
cmp=Image.new("RGB",(4*cell+12,2*cell+4+24),(10,26,68)); dd=ImageDraw.Draw(cmp); dd.text((8,6),"top: r2 original (baked blue-white)   bottom: Neutral + r2-equivalent tints (Blue)",fill=(191,230,255))
qa.append("## 4. Neutral + r2 색상 틴트 vs r2 원본 (검정 배경, 같은 렌더러)\n\n| 에셋 | 평균 절대 차이 (0~255) | 밝기 비 (Neutral/원본) |\n|---|---|---|")
for i,(a,b,n) in enumerate(zip(orig,mine,["Circle Core Static","Circle Small","Beam Static","Impact High Compact"])):
    cmp.paste(a,(i*(cell+4),24)); cmp.paste(P.render if False else b,(i*(cell+4),24+cell+4))
    A=np.array(a).astype(float); B=np.array(b).astype(float); qa.append(f"| {n} | {np.abs(A-B).mean():.2f} | {B.mean()/max(A.mean(),1e-6):.3f} |")
cmp.save(P.OUTD+"Tintable_vs_r2_Blue.png")
qa.append("\n밝기 비가 1보다 작으면 Neutral이 r2 원본보다 어둡게 보인다는 뜻입니다. 주된 원인(추정, 별도 실험으로 분리하지 않음): (1) 청백 텍스처에 구워진 색이 한 메시 안에서 서로 다른 청색 단계(#BFE6FF, #4FA8FF)를 가졌지만 Neutral은 밝기 단계만 유지하고 색조는 메시당 하나의 틴트로 통일되기 때문, (2) 백색 디스크/룬이 메시 틴트(main)로 물들기 때문입니다. 필요하면 DLL이 `Emissive Multiple`을 올려 보정하세요.\n")
open(os.path.join(ROOT,"Docs","TINTABLE_QA.md"),"w").write("\n".join(qa)+"\n"); print("\n".join(qa))
