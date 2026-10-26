# Software previews (additive, static pose) for the ChargeCannon r2 package. usage: python3 previews.py
import os, numpy as np
from PIL import Image, ImageDraw
HERE=os.path.dirname(os.path.abspath(__file__)); PNG=os.path.join(HERE,"..","png")+"/"; SPEC=os.path.join(HERE,"..","nifspec")+"/"; OUTD=(os.environ.get("CC_PREVIEW_DIR") or os.path.join(HERE,"..","..","Previews")).rstrip("/")+"/"
_tex={}
def tex(n):
    if n not in _tex: _tex[n]=np.array(Image.open(PNG+n).convert("RGBA")).astype(np.float32)/255
    return _tex[n]
def parse(path):
    L=open(path).read().split("\n"); i=0; meshes=[]; nodes={"-":(0,0,0)}
    while i<len(L):
        t=L[i].split()
        if not t: i+=1; continue
        if t[0]=="NODE": p=nodes[t[2]]; nodes[t[1]]=(p[0]+float(t[3]),p[1]+float(t[4]),p[2]+float(t[5])); i+=1
        elif t[0]=="MESH":
            nv,nt=int(t[-2]),int(t[-1]); V=np.array([[float(x) for x in L[i+1+k].split()] for k in range(nv)]); T=np.array([[int(x) for x in L[i+1+nv+k].split()] for k in range(nt)])
            meshes.append(dict(name=t[1],parent=t[2],tex=t[3].split("ChargeCannon\\")[-1].replace("\\","/").replace(".dds",".png"),col=np.array([float(t[4]),float(t[5]),float(t[6])]),al=float(t[7]),em=float(t[8]),V=V,T=T)); i+=1+nv+nt
        else: i+=1
    return meshes,nodes
def render(spec,view="front",scale=2.0,W=900,H=900,repeat=1,seg=512.0,f=1.0,filt=None,lat=None,ox=None,bg=(10,26,68),tintfn=None,expo=1.4):
    meshes,nodes=parse(SPEC+spec+".nifspec"); canvas=np.zeros((H,W,3),np.float32); lat=lat or {}
    for k in range(repeat):
      for m in meshes:
        if filt and not any(s in m["name"] for s in filt): continue
        t=tex(m["tex"]); th,tw=t.shape[:2]; off=nodes[m["parent"]]; V=m["V"].copy()
        lf=next((v for s,v in lat.items() if s in m["name"] or s in m["parent"]),1.0)
        if view=="front": px=V[:,0]*lf+off[0]; py=V[:,2]*lf+off[2]
        else: px=(V[:,1]*f+k*seg*f)+off[1]; py=V[:,2]*lf+off[2]
        sx=(W/2+px*scale) if view=="front" else ((ox if ox is not None else 30)+px*scale); sy=H/2-py*scale
        for a,b,c in m["T"]:
            xs=sx[[a,b,c]]; ys=sy[[a,b,c]]
            x0,x1=int(max(0,xs.min())),int(min(W-1,xs.max()+1)); y0,y1=int(max(0,ys.min())),int(min(H-1,ys.max()+1))
            if x1<=x0 or y1<=y0: continue
            gx,gy=np.meshgrid(np.arange(x0,x1+1)+0.5,np.arange(y0,y1+1)+0.5); d=(ys[1]-ys[2])*(xs[0]-xs[2])+(xs[2]-xs[1])*(ys[0]-ys[2])
            if abs(d)<1e-9: continue
            w0=((ys[1]-ys[2])*(gx-xs[2])+(xs[2]-xs[1])*(gy-ys[2]))/d; w1=((ys[2]-ys[0])*(gx-xs[2])+(xs[0]-xs[2])*(gy-ys[2]))/d; w2=1-w0-w1
            ins=(w0>=0)&(w1>=0)&(w2>=0)
            if not ins.any(): continue
            u=w0*V[a,3]+w1*V[b,3]+w2*V[c,3]; v=w0*V[a,4]+w1*V[b,4]+w2*V[c,4]
            tu=np.clip((u%1.0)*tw,0,tw-1).astype(int); tv=np.clip((v%1.0)*th,0,th-1).astype(int); s=t[tv,tu]
            col=s[...,:3]*(np.array(tintfn(m)) if tintfn else m["col"])*(s[...,3:4]*m["al"]*min(m["em"],1.6)/1.6*expo); canvas[y0:y1+1,x0:x1+1]+=np.where(ins[...,None],col,0)
    b=np.array(bg,np.float32)/255; r=np.clip(b+np.clip(canvas,0,1)*(1-b*0.3),0,1); return Image.fromarray((r*255).astype(np.uint8))
def label(img,text,xy=(10,8)): ImageDraw.Draw(img).text(xy,text,fill=(191,230,255)); return img
def stack(imgs,gap=6,bg=(10,26,68)):
    w=max(i.width for i in imgs); h=sum(i.height for i in imgs)+gap*(len(imgs)-1); o=Image.new("RGB",(w,h),bg); y=0
    for i in imgs: o.paste(i,(0,y)); y+=i.height+gap
    return o
def grid(imgs,cols,gap=6,bg=(10,26,68)):
    w,h=imgs[0].size; rows=(len(imgs)+cols-1)//cols; o=Image.new("RGB",(cols*w+gap*(cols-1),rows*h+gap*(rows-1)),bg)
    for k,i in enumerate(imgs): o.paste(i,((k%cols)*(w+gap),(k//cols)*(h+gap)))
    return o
if __name__=="__main__":
    # 1 Circle large
    label(render("Charge_Circle_Core_Static","front",2.1,900,900),"Charge_Circle_Core_Static  (large, front, static pose)").save(OUTD+"Circle_Large.png")
    # 2 Circle small: big view + 3x3 cluster at ~1/4 size + large circle at the same small size for comparison
    big=label(render("Charge_Circle_Small","front",1.5,600,600),"Charge_Circle_Small  (1 texture quad + dim halo)")
    sm=[render("Charge_Circle_Small","front",0.27,110,110) for _ in range(9)]; lg=[render("Charge_Circle_Static","front",0.27,110,110) for _ in range(9)]
    row=Image.new("RGB",(600+20+3*116+20+3*116,600),(10,26,68)); row.paste(big,(0,0))
    g1=grid(sm,3); g2=grid(lg,3); row.paste(g1,(620,10)); row.paste(g2,(620+g1.width+20,10))
    ImageDraw.Draw(row).text((620,380),"Small x9 at 110px (left)   vs   Large scaled down x9 (right)",fill=(191,230,255)); row.save(OUTD+"Circle_Small.png")
    # 3 Beam unit alone (1 segment) with unit-length ruler
    im=render("Beam_Body_Unit","side",1.6,880,260,1); d=ImageDraw.Draw(im); d.line((30,240,30+512*1.6,240),fill=(191,230,255)); d.text((30,244),"0",fill=(191,230,255)); d.text((30+512*1.6-20,244),"512 (unit length)",fill=(191,230,255)); label(im,"Beam_Body_Unit : 1 segment, origin y=0, forward +Y").save(OUTD+"Beam_Unit.png")
    # 4 Seam test: 1 / 3 / 8 segments + zoom on a seam
    r1=label(render("Beam_Body_Unit_Static","side",1.6,880,240,1),"1 segment"); r3=label(render("Beam_Body_Unit_Static","side",0.55,880,240,3),"3 segments (seams at y=512, 1024)"); r8=label(render("Beam_Body_Unit_Static","side",0.21,880,240,8),"8 segments (4096 units)")
    z=render("Beam_Body_Unit_Static","side",3.2,880,300,3,ox=-512*3.2+440); label(z,"zoom x3.2 around seam at y=512 (centre of image)")
    ImageDraw.Draw(z).line((440,0,440,12),fill=(255,255,255)); stack([r1,r3,r8,z]).save(OUTD+"Beam_Seam_Test.png")
    # 5 Core/Glow independence: same length (3 segs), different widths
    rows=[label(render("Beam_Body_Unit_Static","side",0.55,880,200,3),"default widths (core 1.0, glow 1.0)"),
          label(render("Beam_Body_Unit_Static","side",0.55,880,200,3,lat={"BEAM_Core":0.5}),"core x0.5, glow x1.0"),
          label(render("Beam_Body_Unit_Static","side",0.55,880,200,3,lat={"BEAM_Glow":1.5,"BEAM_Halo":1.5}),"core x1.0, glow+halo x1.5"),
          label(render("Beam_Body_Unit_Static","side",0.55,880,200,3,filt=["BEAM_Core"]),"BEAM_Core only"),
          label(render("Beam_Body_Unit_Static","side",0.55,880,200,3,filt=["BEAM_Glow"]),"BEAM_Glow only (GlowIn + GlowShell)")]
    stack(rows).save(OUTD+"Beam_CoreGlow_Split.png")
    # 5b same width, different length (matrix-scale stretch f on forward axis)
    rows=[label(render("Beam_Body_Unit_Static","side",0.40,880,160,2,f=1.0),"length 1024 : 2 segments, f=1.0"),
          label(render("Beam_Body_Unit_Static","side",0.40,880,160,5,f=1.0),"length 2560 : 5 segments, f=1.0"),
          label(render("Beam_Body_Unit_Static","side",0.40,880,160,5,f=0.8),"length 2048 : 5 segments, f=0.8 (L = n*512*f) , same width")]
    stack(rows).save(OUTD+"Beam_Length_Width_Independence.png")
    # 6 Impact comparison: High vs High_Compact, front + side
    hf=label(render("Impact_High","front",2.2,520,520),"Impact_High (front, peak pose: all rings overlap)"); cf=label(render("Impact_High_Compact","front",2.2,520,520),"Impact_High_Compact (front, peak pose)")
    hs=label(render("Impact_High","side",1.8,520,400,ox=60),"Impact_High (side, +Y normal)"); cs=label(render("Impact_High_Compact","side",1.8,520,400,ox=60),"Impact_High_Compact (side)")
    grid([hf,cf,hs,cs],2).save(OUTD+"Impact_Comparison.png")
    print("previews ok")
