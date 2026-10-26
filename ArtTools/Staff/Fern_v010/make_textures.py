from pathlib import Path
import numpy as np
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'source'/'textures';OUT.mkdir(parents=True,exist_ok=True)
rng=np.random.default_rng(73821)
def save(name,n,color,height,spec):
    color=np.broadcast_to(color,(n,n,3)); color=np.clip(color,0,255).astype('uint8')
    Image.fromarray(color,'RGB').save(OUT/f'{name}_d.png')
    # Tangent normal; alpha is the Skyrim specular mask.
    dx=(np.roll(height,-1,1)-np.roll(height,1,1))*0.5
    dy=(np.roll(height,-1,0)-np.roll(height,1,0))*0.5
    norm=np.stack([-dx,dy,np.ones_like(dx)],axis=-1);norm/=np.linalg.norm(norm,axis=-1,keepdims=True)
    rgba=np.empty((n,n,4),dtype='uint8');rgba[:,:,:3]=np.clip((norm+1)*127.5,0,255)
    rgba[:,:,3]=np.clip(spec,0,255)
    Image.fromarray(rgba,'RGBA').save(OUT/f'{name}_n.png')
def grid(n):
    return np.meshgrid(np.arange(n,dtype=float)/n,np.arange(n,dtype=float)/n)
n=2048;x,y=grid(n)
warp=x+0.007*np.sin(y*2*np.pi*3)+0.003*np.sin(y*2*np.pi*11)
grain=np.sin(warp*2*np.pi*66+0.8*np.sin(y*2*np.pi*2))
grain2=np.sin(warp*2*np.pi*211+np.sin(y*2*np.pi*5))
pores=np.maximum(0,np.sin(warp*2*np.pi*377)-0.84)**2
noise=rng.normal(0,0.8,(n,n))
tone=grain*7+grain2*3-pores*150+noise
base=np.array([105,78,49])[None,None,:]
color=base+tone[:,:,None]*np.array([1,.9,.7])+np.sin(x*2*np.pi*9)[:,:,None]*2.5
height=grain*.75+grain2*.24-pores*6
save('wood',n,color,height,58+grain*12)
n=2048;x,y=grid(n)
weave=np.sin(x*2*np.pi*512)*np.sin(y*2*np.pi*512)
fiber=.6*np.sin(x*2*np.pi*217)+.5*np.sin(y*2*np.pi*271)
tonal=weave*3+fiber*1.5+3*np.sin(x*2*np.pi*4)*np.sin(y*2*np.pi*3)+rng.normal(0,.5,(n,n))
save('cloth',n,np.array([61,27,107])[None,None,:]+tonal[:,:,None]*np.array([.9,.7,1.2]),weave*.26+fiber*.13,36+weave*5)
n=1024;x,y=grid(n)
brush=np.sin(y*2*np.pi*307)*1.7+np.sin(y*2*np.pi*71)*1.1
wear=rng.normal(0,1.0,(n,n));tone=brush+wear+np.sin(x*2*np.pi*5)*2
save('silver',n,np.array([164,174,173])[None,None,:]+tone[:,:,None],brush*.15+wear*.09,170+wear*4)
n=512;x,y=grid(n)
tone=8*np.sin(x*2*np.pi)*np.sin(y*2*np.pi)+rng.normal(0,.3,(n,n))
save('gem',n,np.array([148,12,25])[None,None,:]+tone[:,:,None]*np.array([1,.3,.3]),np.zeros((n,n)),np.full((n,n),235))
print('SOURCE_TEXTURES',len(list(OUT.glob('*.png'))))
