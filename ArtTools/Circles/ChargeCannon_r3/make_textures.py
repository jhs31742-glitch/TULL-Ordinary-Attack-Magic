# Generates all ChargeCannon source PNGs. Colors are baked (white-ish RGB + alpha); shader base color stays white.
import math, io, cairosvg, numpy as np
from PIL import Image, ImageDraw, ImageFont
import os, sys
# ---- Palette table. "bluewhite" reproduces the r2 production textures exactly.
# "neutral" is derived from it by Rec.601 luminance (R=G=B), so brightness relationships are kept and hue/saturation are removed.
# Add a palette here (e.g. "red") to render a baked-colour variant from the same geometry.
PALETTES = {"bluewhite": dict(core="#FFFFFF", main="#BFE6FF", out="#4FA8FF", aft="#8FE3FF", glow=(232,246,255), hebrew=(191,230,255))}
def _hex2rgb(h): return tuple(int(h[i:i+2],16) for i in (1,3,5))
def _luma(rgb): return int(round(0.299*rgb[0]+0.587*rgb[1]+0.114*rgb[2]))
def _gray_palette(p):
    g={k:_luma(_hex2rgb(p[k])) for k in ("core","main","out","aft")}; q={k:"#%02X%02X%02X"%(v,v,v) for k,v in g.items()}
    q["glow"]=(_luma(p["glow"]),)*3; q["hebrew"]=(_luma(p["hebrew"]),)*3; return q
PALETTES["neutral"]=_gray_palette(PALETTES["bluewhite"])
PAL = sys.argv[1] if len(sys.argv)>1 else "bluewhite"
P = PALETTES[PAL]
OUT = (sys.argv[2] if len(sys.argv)>2 else os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "png")).rstrip("/")+"/"
os.makedirs(OUT, exist_ok=True)
C_MAIN, C_OUT, C_CORE, C_AFT = P["main"], P["out"], P["core"], P["aft"]


def svg2img(svg, px):
    png = cairosvg.svg2png(bytestring=svg.encode(), output_width=px, output_height=px)
    return Image.open(io.BytesIO(png)).convert("RGBA")

# ---------- Circle_Rings 1024 (viewBox +-200) ----------
rings = f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="-200 -200 400 400">
<circle r="190" fill="none" stroke="{C_MAIN}" stroke-width="3"/>
<circle r="180" fill="none" stroke="{C_OUT}" stroke-width="1.5"/>
<circle r="185" fill="none" stroke="{C_AFT}" stroke-width="6" stroke-dasharray="1.5 14.64" opacity="0.8"/>
<circle r="172" fill="none" stroke="{C_MAIN}" stroke-width="1.5"/>
<circle r="150" fill="none" stroke="{C_MAIN}" stroke-width="1.5"/></svg>'''
svg2img(rings, 1024).save(OUT+"Circle_Rings.png")

# ---------- Circle_Face 2048 (viewBox +-150) ----------
RUNES = [
 (47,-121,21.2,"M0 -7V7M0 -3L5 -7M0 2L5 -2"),(-47,-121,-21.2,"M-4 -7L4 -2L-4 3M0 -7V7"),
 (100,-83,50.3,"M-4 -7L4 7M4 -7L-4 7M-5 0H5"),(-100,-83,-50.3,"M3 -7V7M3 -3L-4 -7M3 2L-4 -1M-4 7H3"),
 (53,-75,35.2,"M0 -7L5 0L0 7L-5 0ZM0 -3V3"),(-53,-75,-35.2,"M-4 -3A4 4 0 1 1 4 -3A4 4 0 1 1 -4 -3M0 1V7M-3 4H3"),
 (129,6,92.7,"M-4 -7V7H4M-4 -1L4 -6"),(-129,6,-92.7,"M-5 -7H5M-5 -7V0H0V7M0 0H5"),
 (83,26,107.4,"M-4 7L0 -7L4 7M-3 2H3"),(-83,26,-107.4,"M-4 7V-7L0 -1L4 -7V7"),
 (111,66,120.7,"M-4 -7H4L-4 7H4"),(-111,66,-120.7,"M0 -7L4 -3V3L0 7L-4 3V-3ZM-4 -3L4 3"),
 (36,125,164,"M0 -7V7M-4 -3H4M-4 3H4"),(-36,125,-164,"M0 7V-1M-5 -7L0 -1L5 -7M-4 3H4"),
 (0,88,180,"M-4 -7H4M0 -7V7M-4 7H4")]
def head(vb): return f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="{vb}">'
sat = "".join(f'<g transform="rotate({a})"><circle cy="-100" r="30" fill="none" stroke="{C_MAIN}" stroke-width="3"/><circle cy="-100" r="24" fill="none" stroke="{C_OUT}" stroke-width="1.5"/><circle cy="-100" r="19" fill="none" stroke="{C_CORE}" stroke-width="1"/><circle cy="-100" r="15" fill="{C_CORE}"/></g>' for a in (0,72,144,216,288))
center = f'<circle r="32" fill="none" stroke="{C_CORE}" stroke-width="6" stroke-dasharray="2 23.1"/><circle r="26" fill="none" stroke="{C_MAIN}" stroke-width="3"/><circle r="20" fill="none" stroke="{C_OUT}" stroke-width="1.5"/><circle r="12" fill="{C_CORE}"/>'
lines = f'<polygon points="87,-119.8 0,148 -87,-119.8 140.8,45.7 -140.8,45.7" fill="none" stroke="{C_MAIN}" stroke-width="2" stroke-linejoin="round"/>' + "".join(f'<line transform="rotate({a})" x1="0" y1="-26" x2="0" y2="-70" stroke="{C_MAIN}" stroke-width="2"/>' for a in (0,72,144,216,288))
runes = "".join(f'<g transform="translate({x} {y}) rotate({r})"><path transform="scale(1.2)" d="{d}" fill="none" stroke="{C_MAIN}" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round"/></g>' for x,y,r,d in RUNES)
VB = "-150 -150 300 300"
occ = head(VB) + "".join(f'<g transform="rotate({a})"><circle cy="-100" r="30" fill="#000"/></g>' for a in (0,72,144,216,288)) + '<circle r="26" fill="#000"/></svg>'
lay_lines = svg2img(head(VB)+lines+"</svg>", 2048)
occ_img = svg2img(occ, 2048)
a = np.array(lay_lines).astype(np.float32); o = np.array(occ_img)[...,3].astype(np.float32)/255
a[...,3] *= (1-o)
base = Image.fromarray(a.astype(np.uint8), "RGBA")
for layer in (head(VB)+sat+center+"</svg>", head(VB)+runes+"</svg>"):
    base = Image.alpha_composite(base, svg2img(layer, 2048))
base.save(OUT+"Circle_Face.png")

# ---------- Hebrew_Band 4096x256 ----------
TEXT = "הקסם נאגר בטבעות · הטבעות מתיישרות לקנה אחד · הקרן נורית ועוברת בכל אויב · בפגיעה באדמה האור מתפוצץ · "
chars = list(TEXT); N = len(chars); assert N == 102, N
SQ = 4096/ (2*math.pi*161) / (256/22.0)          # horizontal squash so mapped letters keep aspect (~0.349)
WW = int(round(4096/SQ)); slot = WW/N
font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 165)
big = Image.new("RGBA", (WW, 256), (0,0,0,0)); d = ImageDraw.Draw(big)
for i,c in enumerate(chars):
    if c == " ": continue
    d.text(((i+0.5)*slot, 128), c, font=font, fill=tuple(P['hebrew'])+(255,), anchor="mm")
big.resize((4096,256), Image.LANCZOS).save(OUT+"Hebrew_Band.png")

# ---------- Glow_Soft 256 ----------
y,x = np.mgrid[0:256,0:256]; r = np.hypot(x-127.5,y-127.5)/128.0
al = np.clip(1-r,0,1)**2.2
g = np.zeros((256,256,4),np.uint8); g[...,:3]=P['glow']; g[...,3]=(al*255).astype(np.uint8)
Image.fromarray(g,"RGBA").save(OUT+"Glow_Soft.png")

# ---------- Beam_Streak 512x64 (tiles in U and V) ----------
bs = np.zeros((64,512,4),np.uint8); bs[...,:3]=255
al = np.full((64,512),0.45,np.float32)
xs = np.arange(512)
def band(v0,w,amp,per=None,duty=0.6,ph=0):
    for dy in range(-w,w+1):
        row=(v0+dy)%64; k=amp*(1-abs(dy)/(w+1))
        m = np.ones(512) if per is None else (((xs+ph)%per)<per*duty).astype(np.float32)
        al[row]=np.maximum(al[row], k*m)
band(6,1,0.95); band(22,2,1.0,128,0.7); band(38,1,0.85,64,0.55,10); band(52,2,0.9,256,0.8,40)
for px in range(0,512,32):          # rune-flow ticks
    for dy in range(-4,5): al[(46+dy)%64, (px+14):(px+17)] = np.maximum(al[(46+dy)%64,(px+14):(px+17)],0.8)
bs[...,3]=(np.clip(al,0,1)*255).astype(np.uint8); Image.fromarray(bs,"RGBA").save(OUT+"Beam_Streak.png")

# ---------- Impact_Rings 1024 (2x2 cells of 512) ----------
def cell(inner, maxr): s=240/maxr; return f'<g transform="scale({s})">{inner}</g>'
low = f'<circle r="44" fill="none" stroke="{C_MAIN}" stroke-width="2.5"/><circle r="50" fill="none" stroke="{C_OUT}" stroke-width="1"/><circle r="58" fill="none" stroke="{C_AFT}" stroke-width="10" stroke-dasharray="3 42.55"/>'
mid = f'<circle r="58" fill="none" stroke="{C_AFT}" stroke-width="1" stroke-dasharray="2 4"/><circle r="66" fill="none" stroke="{C_MAIN}" stroke-width="3"/><circle r="74" fill="none" stroke="{C_OUT}" stroke-width="1.5"/><circle r="80" fill="none" stroke="{C_AFT}" stroke-width="12" stroke-dasharray="3 28.4"/>'
high= f'<circle r="64" fill="none" stroke="{C_CORE}" stroke-width="2"/><circle r="36" fill="none" stroke="{C_CORE}" stroke-width="1" stroke-dasharray="2 4"/><circle r="72" fill="none" stroke="{C_MAIN}" stroke-width="3"/><circle r="84" fill="none" stroke="{C_OUT}" stroke-width="2"/><circle r="92" fill="none" stroke="{C_OUT}" stroke-width="1" stroke-dasharray="6 4"/><circle r="98" fill="none" stroke="{C_AFT}" stroke-width="8" stroke-dasharray="3 22.66"/>'
shards = f'<circle cx="-120" cy="-120" r="14" fill="{C_CORE}"/><line x1="40" y1="-120" x2="110" y2="-120" stroke="{C_CORE}" stroke-width="8" stroke-linecap="round"/><path d="M-120 40 L-100 80 L-120 120 L-140 80Z" fill="none" stroke="{C_MAIN}" stroke-width="5"/><circle cx="90" cy="90" r="26" fill="none" stroke="{C_AFT}" stroke-width="6"/>'
atlas = f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1024 1024"><g transform="translate(256 256)">{cell(low,60)}</g><g transform="translate(768 256)">{cell(mid,86)}</g><g transform="translate(256 768)">{cell(high,102)}</g><g transform="translate(768 768) scale(0.9)">{shards}</g></svg>'
svg2img(atlas,1024).save(OUT+"Impact_Rings.png")

# ---------- Circle_Small 1024 (viewBox +-200): single-layer lightweight circle, thicker strokes, 8 runes, no Hebrew band ----------
SV = "-200 -200 400 400"
S_SAT = "".join(f'<g transform="rotate({a})"><circle cy="-100" r="30" fill="none" stroke="{C_MAIN}" stroke-width="5"/><circle cy="-100" r="22" fill="none" stroke="{C_OUT}" stroke-width="2.5"/><circle cy="-100" r="14" fill="{C_CORE}"/></g>' for a in (0,72,144,216,288))
S_CEN = f'<circle r="33" fill="none" stroke="{C_CORE}" stroke-width="8" stroke-dasharray="3 22.1"/><circle r="26" fill="none" stroke="{C_MAIN}" stroke-width="5"/><circle r="19" fill="none" stroke="{C_OUT}" stroke-width="2.5"/><circle r="11" fill="{C_CORE}"/>'
S_RING = (f'<circle r="190" fill="none" stroke="{C_MAIN}" stroke-width="6"/><circle r="176" fill="none" stroke="{C_OUT}" stroke-width="3"/>'
          f'<circle r="183" fill="none" stroke="{C_AFT}" stroke-width="10" stroke-dasharray="4 27.94" opacity="0.85"/>'
          f'<circle r="172" fill="none" stroke="{C_MAIN}" stroke-width="3"/><circle r="150" fill="none" stroke="{C_MAIN}" stroke-width="3"/>'
          f'<circle r="161" fill="none" stroke="{C_MAIN}" stroke-width="8" stroke-dasharray="10 6 4 6 14 6 6 3.85" opacity="0.8"/>')
S_LINES = f'<polygon points="87,-119.8 0,148 -87,-119.8 140.8,45.7 -140.8,45.7" fill="none" stroke="{C_MAIN}" stroke-width="4" stroke-linejoin="round"/>' + "".join(f'<line transform="rotate({a})" x1="0" y1="-26" x2="0" y2="-70" stroke="{C_MAIN}" stroke-width="4"/>' for a in (0,72,144,216,288))
KEEP = (0,3,5,6,9,10,13,14)
S_RUNES = "".join(f'<g transform="translate({RUNES[k][0]} {RUNES[k][1]}) rotate({RUNES[k][2]})"><path transform="scale(1.7)" d="{RUNES[k][3]}" fill="none" stroke="{C_MAIN}" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round"/></g>' for k in KEEP)
sv_occ = head(SV) + "".join(f'<g transform="rotate({a})"><circle cy="-100" r="31" fill="#000"/></g>' for a in (0,72,144,216,288)) + '<circle r="27" fill="#000"/></svg>'
sl = np.array(svg2img(head(SV)+S_LINES+"</svg>", 1024)).astype(np.float32); so = np.array(svg2img(sv_occ,1024))[...,3].astype(np.float32)/255
sl[...,3] *= (1-so); small = Image.fromarray(sl.astype(np.uint8),"RGBA")
for layer in (head(SV)+S_RING+"</svg>", head(SV)+S_SAT+S_CEN+"</svg>", head(SV)+S_RUNES+"</svg>"):
    small = Image.alpha_composite(small, svg2img(layer,1024))
small.save(OUT+"Circle_Small.png")
print("png ok")
