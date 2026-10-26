# Generates .nifspec files for all 9 ChargeCannon NIFs, then calls ./nifbuild for each.
import math, subprocess, os, sys
# usage: gen_nifs.py [bluewhite|neutral] [alt_output_root]
#   bluewhite : r2 production NIFs (default, unchanged)
#   neutral   : Tintable NIFs -> .../ChargeCannon/Tintable/<Name>_Tintable.nif (all shader base RGB = 1,1,1, textures from Tintable/)
MODE = sys.argv[1] if len(sys.argv)>1 else "bluewhite"
HERE = os.path.dirname(os.path.abspath(__file__))
SPEC = os.path.join(HERE, "..", "nifspec"); OUT = os.path.join(HERE, "..", "..", "Data", "meshes", "effects", "ChargeCannon"); LEG = os.path.join(OUT, "Legacy_r1")
if len(sys.argv)>2:   # QA: regenerate into another root without touching the package
    ALT=sys.argv[2]; SPEC=os.path.join(ALT,"nifspec"); OUT=os.path.join(ALT,"meshes"); LEG=os.path.join(OUT,"Legacy_r1")
if MODE=="neutral": SPEC=os.path.join(SPEC,"Tintable"); OUT=os.path.join(OUT,"Tintable")
os.makedirs(SPEC, exist_ok=True); os.makedirs(OUT, exist_ok=True); os.makedirs(LEG, exist_ok=True)
TX = lambda n: "textures\\effects\\ChargeCannon\\" + ("Tintable\\" if (MODE=="neutral" and n!="Beam_Streak") else "") + n + ".dds"   # Beam_Streak is already pure white: shared
WHITE=(1,1,1); MAIN=(.749,.902,1.0); OUTC=(.31,.659,1.0); AFT=(.561,.89,1.0)
if MODE=="neutral": WHITE=MAIN=OUTC=AFT=(1,1,1)   # runtime tint supplies the colour; r2-equivalent tints are listed in Docs/NODE_MAP.md

# ---------------- geometry helpers: return (verts[(x,y,z,u,v,nx,ny,nz)], tris) ----------------
def quad_xz(h, y=0.0, uv=(0,0,1,1), cx=0.0, cz=0.0, hz=None):
    hz = h if hz is None else hz; u0,v0,u1,v1 = uv
    V=[(cx-h,y,cz+hz,u0,v0,0,-1,0),(cx+h,y,cz+hz,u1,v0,0,-1,0),(cx+h,y,cz-hz,u1,v1,0,-1,0),(cx-h,y,cz-hz,u0,v1,0,-1,0)]
    return V,[(0,1,2),(0,2,3)]
def star_flash(h):
    V=[];T=[]
    for plane in range(3):
        b=len(V)
        for (a,c,u,v) in ((-h,h,0,0),(h,h,1,0),(h,-h,1,1),(-h,-h,0,1)):
            V.append(((a,0,c,u,v,0,-1,0),(a,c,0,u,v,0,0,-1),(0,a,c,u,v,-1,0,0))[plane])
        T+= [(b,b+1,b+2),(b,b+2,b+3)]
    return V,T
def annulus(rin, rout, seg, v_in=1.0, v_out=0.0, u_fixed=None):
    V=[];T=[]
    for i in range(seg+1):
        th=2*math.pi*i/seg; u=i/seg if u_fixed is None else u_fixed
        for r,v in ((rout,v_out),(rin,v_in)):
            V.append((-r*math.sin(th),0.0,r*math.cos(th),u,v,0,-1,0))
    for i in range(seg):
        a=2*i; T+=[(a,a+1,a+3),(a,a+3,a+2)]
    return V,T
def cylinder(profile, seg, tile_len):      # axis +Y, profile=[(y,r),...], u along length, v around
    V=[];T=[]
    for (y,r) in profile:
        for j in range(seg+1):
            a=2*math.pi*j/seg; V.append((r*math.cos(a),y,r*math.sin(a),y/tile_len,j/seg,math.cos(a),0,math.sin(a)))
    n=seg+1
    for k in range(len(profile)-1):
        for j in range(seg):
            a=k*n+j; b=a+n; T+=[(a,b,a+1),(a+1,b,b+1)]
    return V,T
def half_sphere(r, lat=6, lon=16):          # dome on +Y side, planar uv for Glow_Soft
    V=[];T=[]
    for i in range(lat+1):
        phi=(math.pi/2)*i/lat
        for j in range(lon+1):
            th=2*math.pi*j/lon; x=r*math.cos(phi)*math.cos(th); z=r*math.cos(phi)*math.sin(th); y=r*math.sin(phi)
            V.append((x,y,z,(x/r+1)/2,(z/r+1)/2,x/r,y/r,z/r))
    n=lon+1
    for i in range(lat):
        for j in range(lon):
            a=i*n+j; T+=[(a,a+n,a+1),(a+1,a+n,a+n+1)]
    return V,T
def scatter(nq, radius, size, y=0.0):
    V=[];T=[]
    for i in range(nq):
        a=2*math.pi*i/nq + (0.2 if i%2 else 0); rr=radius*(0.85 if i%2 else 1.0); cx=rr*math.cos(a); cz=rr*math.sin(a); b=len(V)
        q,_=quad_xz(size,y,(0,0,1,1),cx,cz); V+=q; T+=[(b,b+1,b+2),(b,b+2,b+3)]
    return V,T

# ---------------- spec writer ----------------
class Spec:
    def __init__(s): s.l=[]
    def root(s,name): s.l.append(f"ROOT {name}")
    def node(s,name,parent="-",x=0,y=0,z=0,sc=1): s.l.append(f"NODE {name} {parent} {x} {y} {z} {sc}")
    def mesh(s,name,parent,geo,tex,color=WHITE,alpha=1.0,emis=1.5,dbl=1,fall=None,clamp=0):
        V,T=geo; f=((1,)+tuple(fall)) if fall else (0,1,1,1,1)
        s.l.append(f"MESH {name} {parent} {TX(tex)} {color[0]} {color[1]} {color[2]} {alpha} {emis} {dbl} {f[0]} {f[1]} {f[2]} {f[3]} {f[4]} {clamp} {len(V)} {len(T)}")
        for v in V: s.l.append(" ".join(f"{c:.5f}" if isinstance(c,float) else str(c) for c in v))
        for t in T: s.l.append(f"{t[0]} {t[1]} {t[2]}")
    def spin(s,node,axis,period,d): s.l.append(f"SPIN {node} {axis} {period} {d}")
    def nscale(s,node,loop,keys): s.l.append(f"NODESCALE {node} {loop} {len(keys)} "+" ".join(f"{t} {v}" for t,v in keys))
    def shf(s,mesh,var,loop,keys): s.l.append(f"SHFLOAT {mesh} {var} {loop} {len(keys)} "+" ".join(f"{t} {v}" for t,v in keys))
    def pulse(s,mesh,lo,hi,period,n=8):
        ks=[(round(period*i/n,4), round(lo+(hi-lo)*(0.5-0.5*math.cos(2*math.pi*i/n)),4)) for i in range(n+1)]; s.shf(mesh,0,1,ks)
    def scroll(s,mesh,tiles_per_sec): s.shf(mesh,6,1,[(0,0),(round(1.0/tiles_per_sec,4),-1)])
    def fade(s,mesh,keys): s.shf(mesh,5,0,keys)
    def write(s,name,outdir=None,specname=None):
        if MODE=="neutral": name=name+"_Tintable"; specname=None
        p=os.path.join(SPEC,(specname or name)+".nifspec"); open(p,"w").write("\n".join(s.l)+"\n")
        out=os.path.join(outdir or OUT,name+".nif"); r=subprocess.run([os.path.join(HERE,"nifbuild"),p,out],capture_output=True,text=True)
        print(name, r.stdout.strip(), r.stderr.strip())


# ================= r2 assets =================
# Axis convention (Skyrim): +X right, +Y forward, +Z up. Circle plane = XZ (normal +Y). Beam axis = +Y. Impact: +Y = surface normal.
P=0.1
def circle(fname, core, animated):
    s=Spec(); s.root("CC_Root"); em=1.3 if core else 1.0
    s.node("CC_Glow",y=0*P); s.mesh("CC_Glow_Mesh","CC_Glow",quad_xz(200),"Glow_Soft",OUTC,0.35,1.2*em); s.pulse("CC_Glow_Mesh",0.9*em,1.5*em,1.6)
    s.node("CC_Outer",y=1*P); s.mesh("CC_Outer_Mesh","CC_Outer",quad_xz(200),"Circle_Rings",WHITE,1.0,1.6*em)
    s.node("CC_Band",y=2*P); s.mesh("CC_Band_Mesh","CC_Band",annulus(150,172,128),"Hebrew_Band",WHITE,1.0,1.8*em)
    s.node("CC_Face",y=3*P); s.mesh("CC_Face_Mesh","CC_Face",quad_xz(150),"Circle_Face",WHITE,1.0,1.8*em); s.pulse("CC_Face_Mesh",1.3*em,2.2*em,1.6)
    s.node("CC_NodeGlow","CC_Face",y=P)   # child of CC_Face: stays aligned with the satellite nodes when CC_Face is rotated
    pts=[(0,0)]+[(100*math.sin(math.radians(a)),100*math.cos(math.radians(a))) for a in (0,72,144,216,288)]
    V=[];T=[]
    for i,(cx,cz) in enumerate(pts):
        q,_=quad_xz(38 if i else 30,0,(0,0,1,1),cx,cz); b=len(V); V+=q; T+=[(b,b+1,b+2),(b,b+2,b+3)]
    s.mesh("CC_NodeGlow_Mesh","CC_NodeGlow",(V,T),"Glow_Soft",WHITE,0.55,1.6*em); s.pulse("CC_NodeGlow_Mesh",1.0*em,2.0*em,1.6)
    if core:
        s.node("CC_Lens",y=5*P); s.mesh("CC_Lens_Mesh","CC_Lens",annulus(150,170,128,0.85,0.5,0.5),"Glow_Soft",WHITE,0.9,2.2); s.pulse("CC_Lens_Mesh",1.6,2.6,1.6)
    if animated: s.spin("CC_Outer",1,60,1); s.spin("CC_Band",1,36,-1)
    s.write(fname)
circle("Charge_Circle_Animated",False,True); circle("Charge_Circle_Core_Animated",True,True)
circle("Charge_Circle_Static",False,False); circle("Charge_Circle_Core_Static",True,False)

# ---- Charge_Circle_Small: 1 texture quad (Circle_Small) + dim optional halo; no band/lens/node-glow layers ----
s=Spec(); s.root("CC_Root")
s.node("CC_Glow",y=0); s.mesh("CC_Glow_Mesh","CC_Glow",quad_xz(200),"Glow_Soft",OUTC,0.18,0.8)
s.node("CC_Face",y=P); s.mesh("CC_Face_Mesh","CC_Face",quad_xz(200),"Circle_Small",WHITE,1.0,1.4)
s.write("Charge_Circle_Small")

# ---- Beam_Body_Unit: 512-unit seamless cylinder segment, origin y=0, forward +Y, nominal glow radius 40 ----
U=512.0; RN=40.0
def beam_unit(fname, scroll):
    s=Spec(); s.root("BEAM_Root")
    def cyl(r): return cylinder([(0,r),(U,r)],16,U)       # two rings, constant radius, u: 0 -> 1 across the segment (tiles seamlessly)
    s.node("BEAM_Core","-"); s.mesh("BEAM_Core_Mesh","BEAM_Core",cyl(0.26*RN),"Beam_Streak",WHITE,1.0,4.0,clamp=3)
    s.node("BEAM_Glow","-")
    s.mesh("BEAM_GlowIn_Mesh","BEAM_Glow",cyl(0.65*RN),"Beam_Streak",MAIN,0.9,2.5,fall=(1,0.25,1,0.2),clamp=3)
    s.mesh("BEAM_GlowShell_Mesh","BEAM_Glow",cyl(1.0*RN),"Beam_Streak",OUTC,0.85,2.0,fall=(1,0.1,1,0.45),clamp=3)
    s.node("BEAM_Halo","-"); s.mesh("BEAM_Halo_Mesh","BEAM_Halo",cyl(1.2*RN),"Beam_Streak",AFT,0.25,1.5,fall=(1,0.1,1,0.3),clamp=3)
    if scroll:   # app-time loop controllers (global clock) so all segments stay phase-aligned
        for m,sp in (("BEAM_Core_Mesh",2.0),("BEAM_GlowIn_Mesh",1.2),("BEAM_GlowShell_Mesh",0.8),("BEAM_Halo_Mesh",0.6)): s.scroll(m,sp)
        s.pulse("BEAM_Core_Mesh",3.4,4.4,0.5); s.pulse("BEAM_GlowIn_Mesh",2.1,2.75,0.5)
    s.write(fname)
beam_unit("Beam_Body_Unit",True); beam_unit("Beam_Body_Unit_Static",False)

# ---- Beam_Muzzle ----
s=Spec(); s.root("MUZZLE_Root")
s.node("MUZZLE_Flash"); s.mesh("MUZZLE_Flash_Mesh","MUZZLE_Flash",star_flash(80),"Glow_Soft",WHITE,1.0,3.0); s.nscale("MUZZLE_Flash",0,[(0,0.2),(0.04,1.0),(0.12,0.0001)])
s.node("MUZZLE_Ring"); s.mesh("MUZZLE_Ring_Mesh","MUZZLE_Ring",quad_xz(100),"Circle_Rings",WHITE,1.0,2.2)
s.nscale("MUZZLE_Ring",0,[(0,0.5),(0.3,1.2)]); s.fade("MUZZLE_Ring_Mesh",[(0,1),(0.15,0.9),(0.3,0)])
s.write("Beam_Muzzle")

# ---- Impact ----
CELL={"low":((0,0,.5,.5),60),"mid":((.5,0,1,.5),86),"high":((0,.5,.5,1),102)}
IMP={
 "Impact_Low":    dict(cell="low", flash=28,fend=.13,rings=[0],life=.5, sph=None,pil=[],nsh=8, rad=50,ssz=5,after=None),
 "Impact_Mid":    dict(cell="mid", flash=40,fend=.2, rings=[0,.08],life=.9,sph=(48,None),pil=[(8,90)],nsh=16,rad=60,ssz=5,after=None),
 "Impact_High":   dict(cell="high",flash=48,fend=.2, rings=[0,.08,.16],life=1.5,sph=(56,24),pil=[(30,130),(19,130)],nsh=24,rad=78,ssz=6,after=(110,0.6,3.0)),
 "Impact_High_Compact": dict(cell="high",flash=34,fend=.15,rings=[0,.08],life=1.1,sph=(44,20),pil=[(22,70),(13,70)],nsh=12,rad=70,ssz=5,after=(70,0.35,1.2)),
}
def impact(name):
    c=IMP[name]; s=Spec(); s.root("IMPACT_Root"); life=c["life"]; uv,mx=CELL[c["cell"]]
    s.node("IMPACT_Flash"); s.mesh("IMPACT_Flash_Mesh","IMPACT_Flash",star_flash(c["flash"]*2),"Glow_Soft",WHITE,1.0,3.5)
    s.nscale("IMPACT_Flash",0,[(0,0.2),(0.05,1.0),(c["fend"],0.0001)])
    for i,d in enumerate(c["rings"]):
        n=f"IMPACT_Ring{i+1}"; s.node(n,"-",0,0.2*i,0); s.mesh(n+"_Mesh",n,quad_xz(mx*256/240,0.0,uv),"Impact_Rings",WHITE,1.0,2.2)
        if d==0: s.nscale(n,0,[(0,0.3),(life*0.7,1.0)]); s.fade(n+"_Mesh",[(0,1),(life*0.4,0.9),(life*0.7,0)])
        else:
            s.nscale(n,0,[(0,0.0001),(d,0.3),(d+life*0.7,1.0)]); s.fade(n+"_Mesh",[(0,0),(d,0),(d+0.02,1),(d+life*0.4,0.9),(d+life*0.7,0)])
    if c["sph"]:
        r1,r2=c["sph"]
        s.node("IMPACT_Sphere"); s.mesh("IMPACT_Sphere_Mesh","IMPACT_Sphere",half_sphere(r1),"Glow_Soft",MAIN,0.5,1.6,fall=(1,0.3,1,0.2))
        s.nscale("IMPACT_Sphere",0,[(0,0.1),(0.3,1.0),(life,1.1)]); s.fade("IMPACT_Sphere_Mesh",[(0,0.8),(0.3,0.5),(life,0)])
        if r2:
            s.node("IMPACT_SphereIn"); s.mesh("IMPACT_SphereIn_Mesh","IMPACT_SphereIn",half_sphere(r2),"Glow_Soft",WHITE,0.8,2.2); s.nscale("IMPACT_SphereIn",0,[(0,0.1),(0.25,1.0),(life*0.8,0.0001)])
    for i,(pr,plen) in enumerate(c["pil"]):
        pn=f"IMPACT_Pillar{i+1}"; prof=[(0,pr)]+[((plen-pr)+pr*math.sin(a*math.pi/8),pr*math.cos(a*math.pi/8)) for a in range(0,5)]
        s.node(pn); s.mesh(pn+"_Mesh",pn,cylinder(prof,16,8*2*math.pi*pr),"Beam_Streak",WHITE if i else MAIN,0.8 if i==0 else 0.9,2.5,clamp=3,fall=(1,0.1,1,0.4))
        s.nscale(pn,0,[(0,0.01),(0.1,1.0),(life*0.5,1.0),(life*0.9,0.0001)]); s.fade(pn+"_Mesh",[(0,1),(life*0.5,0.8),(life*0.9,0)])
    s.node("IMPACT_Shards"); s.mesh("IMPACT_Shards_Mesh","IMPACT_Shards",scatter(c["nsh"],c["rad"],c["ssz"]),"Glow_Soft",AFT,1.0,2.5)
    s.nscale("IMPACT_Shards",0,[(0,0.4),(life,1.8)]); s.fade("IMPACT_Shards_Mesh",[(0,1),(life*0.5,0.7),(life,0)])
    if c["after"]:
        h,a,t=c["after"]; s.node("IMPACT_After","-",0,0.5,0); s.mesh("IMPACT_After_Mesh","IMPACT_After",quad_xz(h),"Glow_Soft",MAIN,a,1.4)
        s.nscale("IMPACT_After",0,[(0,0.5),(min(0.5,t/3),1.0)]); s.fade("IMPACT_After_Mesh",[(0,0),(0.08,1.0),(t*0.3,0.9),(t,0)])
    s.write(name)
for n in IMP: impact(n)

# ---- Legacy r1 fixed-length beams (superseded by Beam_Body_Unit; kept for reference) ----
def beam_legacy(name,R,L,em,high):
    s=Spec(); seg=16; tl=lambda r:8*2*math.pi*r
    def prof(r): return [(0,0.0),(0.015*L,0.9*r),(0.04*L,r),(L,r)]
    layers=[("Core",0.26,WHITE,1.0,em[0],None,2.0),("Glow",0.65,MAIN,0.9,em[1],(1,0.25,1,0.2),1.2),("Shell",1.0,OUTC,0.85,em[2],(1,0.1,1,0.45),0.8)]
    if high: layers+= [("Shell2",1.2,AFT,0.25,em[2],(1,0.1,1,0.3),0.6),("Distort",1.34,MAIN,0.10,1.0,(1,0.1,1,0.1),0.3)]
    s.node("Beam")
    for (nm,k,col,al,e,fall,speed) in layers:
        r=R*k; s.mesh("Beam_"+nm,"Beam",cylinder(prof(r),seg,tl(r)),"Beam_Streak",col,al,e,dbl=1,fall=fall,clamp=3); s.scroll("Beam_"+nm,speed)
        if nm in ("Core","Glow"): s.pulse("Beam_"+nm,e*0.85,e*1.1,0.5)
    s.write(name,LEG,"legacy_"+name)
if MODE=="bluewhite": beam_legacy("Beam_Low",14,2000,(3,2,1.5),False)
if MODE=="bluewhite": beam_legacy("Beam_Mid",33.6,3200,(4,2.5,2),False)
if MODE=="bluewhite": beam_legacy("Beam_High",67.2,5000,(5,3,2.5),True)
