from pathlib import Path
import numpy as np,struct,shutil
from PIL import Image,ImageFilter
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'source'/'textures';OUT.mkdir(parents=True,exist_ok=True)
rng=np.random.default_rng(29064);OLD=ROOT.parents[1]/'source'/'textures'
for mat in ['cloth','gem']:
    for suff in ['d','n']:shutil.copy2(OLD/f'{mat}_{suff}.png',OUT/f'{mat}_{suff}.png')
def smooth_noise(n,scale):
    a=(rng.random((scale,scale))*255).astype('uint8')
    return np.asarray(Image.fromarray(a).resize((n,n),Image.Resampling.BICUBIC),dtype=float)/255-.5
def normal_map(name,height,alpha):
    dx=(np.roll(height,-1,1)-np.roll(height,1,1))/2;dy=(np.roll(height,-1,0)-np.roll(height,1,0))/2
    norm=np.stack([-dx,dy,np.ones_like(dx)],-1);norm/=np.linalg.norm(norm,axis=-1,keepdims=True)
    out=np.empty((*height.shape,4),dtype='uint8');out[:,:,:3]=np.clip((norm+1)*127.5,0,255);out[:,:,3]=np.clip(alpha,0,255)
    Image.fromarray(out).save(OUT/f'{name}_n.png')
# Generated albedo is only resampled for the technical game texture size.
selected=ROOT/'source'/'imagegen'/'wood_albedo_selected.png'
im=Image.open(selected).convert('RGB').resize((2048,2048),Image.Resampling.LANCZOS);im.save(OUT/'wood_d.png')
lum=np.asarray(im.convert('L'),float);blur=np.asarray(im.convert('L').filter(ImageFilter.GaussianBlur(2.2)),float)
normal_map('wood',(lum-blur)*.035,12+lum*.10)
n=1024;x,y=np.meshgrid(np.linspace(-1,1,n),np.linspace(-1,1,n));angle=np.arctan2(y-.04,x+.10)
radius=np.sqrt((x+.10)**2+(y-.04)**2)
warp=radius*(1+.035*np.sin(angle*5)+.018*np.sin(angle*11))+smooth_noise(n,8)*.05
rings=np.sin(warp*2*np.pi*16+.4*np.sin(angle*3));fine=np.maximum(0,rings-.6)
tone=rings*10-fine*18+smooth_noise(n,64)*8;crack=np.zeros_like(radius)
for a in [-2.2,.65,2.7]:
    da=np.arctan2(np.sin(angle-a),np.cos(angle-a));crack+=np.exp(-(da/.012)**2)*np.clip((radius-.48)*2,0,1)
color=np.array([120,86,56])[None,None,:]+tone[:,:,None]-crack[:,:,None]*27
Image.fromarray(np.clip(color,0,255).astype('uint8')).save(OUT/'endgrain_d.png')
normal_map('endgrain',rings*.13-fine*.35-crack*.50,np.full((n,n),18))
# Muted aged steel: broad patina and minute irregular wear, without lathe stripes.
n=1024;coarse=smooth_noise(n,12);mid=smooth_noise(n,80);fine=rng.normal(0,.75,(n,n));tone=coarse*16+mid*5+fine
col=np.array([102,107,95])[None,None,:]+tone[:,:,None]
Image.fromarray(np.clip(col,0,255).astype('uint8')).save(OUT/'silver_d.png')
normal_map('silver',mid*.12+fine*.018,115+coarse*20+mid*8)
Image.fromarray(np.clip(100+coarse*18+mid*8,0,255).astype('uint8')).convert('RGB').save(OUT/'silver_m.png')
# A restrained reflection cubemap: B8G8R8A8, six faces, full mip chain.
n=128;mips=8;dds=ROOT/'package'/'textures'/'FernStaff'/'silver_e.dds';dds.parent.mkdir(parents=True,exist_ok=True)
header=struct.pack('<7I',124,0x2100F,n,n,n*4,0,mips)+b'\0'*44
header+=struct.pack('<8I',32,0x41,0,32,0x00ff0000,0x0000ff00,0x000000ff,0xff000000)
header+=struct.pack('<5I',0x401008,0xfe00,0,0,0);assert len(header)==124
with dds.open('wb') as f:
    f.write(b'DDS '+header)
    for face in range(6):
        for level in range(mips):
            size=max(1,n>>level);u,v=np.meshgrid(np.linspace(-1,1,size),np.linspace(-1,1,size))
            if face==0:d=np.stack([np.ones_like(u),-v,-u],-1)
            elif face==1:d=np.stack([-np.ones_like(u),-v,u],-1)
            elif face==2:d=np.stack([u,np.ones_like(u),v],-1)
            elif face==3:d=np.stack([u,-np.ones_like(u),-v],-1)
            elif face==4:d=np.stack([u,-v,np.ones_like(u)],-1)
            else:d=np.stack([-u,-v,-np.ones_like(u)],-1)
            d/=np.linalg.norm(d,axis=-1,keepdims=True);sky=.14+.28*(d[:,:,2]+1)/2
            l=np.array([.4,.5,.76]);l/=np.linalg.norm(l);soft=np.maximum(0,np.sum(d*l,axis=-1))**15*.45
            rgb=np.clip((sky+soft)[:,:,None]*np.array([.98,1,.97])*255,0,255).astype('uint8')
            rgba=np.empty((size,size,4),dtype='uint8');rgba[:,:,:3]=rgb[:,:,::-1];rgba[:,:,3]=255;f.write(rgba.tobytes())
print('TEXTURES_V020_READY')
