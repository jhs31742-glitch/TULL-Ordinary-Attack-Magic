using System;
using System.IO;
using System.Text;
using System.Linq;
using System.Collections.Generic;
public static class CP019Esp {
 public static int NameChanges;
 static string Sig(byte[] b,int p){return Encoding.ASCII.GetString(b,p,4);}
 static byte[] Cut(byte[] b,int p,int n){var v=new byte[n];Buffer.BlockCopy(b,p,v,0,n);return v;}
 static byte[] Join(params byte[][] parts){using(var m=new MemoryStream()){foreach(var p in parts)m.Write(p,0,p.Length);return m.ToArray();}}
 static void U32(byte[] b,int p,uint n){Buffer.BlockCopy(BitConverter.GetBytes(n),0,b,p,4);}
 static byte[] Sub(string tag,byte[] payload){return Join(Encoding.ASCII.GetBytes(tag),BitConverter.GetBytes((ushort)payload.Length),payload);}
 static byte[] Str(string s){return Encoding.UTF8.GetBytes(s+"\0");}
 public static Dictionary<uint,byte[]> Records(byte[] b){var d=new Dictionary<uint,byte[]>();Walk(b,0,b.Length,d);return d;}
 static void Walk(byte[] b,int p,int end,Dictionary<uint,byte[]> d){
  while(p<end){if(end-p<24)throw new Exception("short record");var tag=Sig(b,p);int len=checked((int)BitConverter.ToUInt32(b,p+4));
   if(tag=="GRUP"){if(len<24||p+len>end)throw new Exception("group size");Walk(b,p+24,p+len,d);p+=len;}
   else{if(p+24+len>end)throw new Exception("record size");var id=BitConverter.ToUInt32(b,p+12);if(d.ContainsKey(id))throw new Exception("duplicate FormID");d[id]=Cut(b,p,24+len);p+=24+len;}
  }if(p!=end)throw new Exception("boundary");
 }
 static byte[] ReplaceSub(byte[] record,string tag,byte[] value){
  if((BitConverter.ToUInt32(record,8)&0x40000)!=0)throw new Exception("compressed edit rejected");
  using(var output=new MemoryStream()){output.Write(record,0,24);int p=24,n=0;
   while(p<record.Length){var s=Sig(record,p);int len=BitConverter.ToUInt16(record,p+4);if(s=="XXXX")throw new Exception("extended edit rejected");
    var bytes=s==tag?Sub(tag,value):Cut(record,p,6+len);if(s==tag)++n;output.Write(bytes,0,bytes.Length);p+=6+len;
   }if(n!=1)throw new Exception("subrecord count "+tag+"="+n);
   var result=output.ToArray();U32(result,4,(uint)(result.Length-24));return result;
  }
 }
 static byte[] Rewrite(byte[] b,int p,int end,Func<byte[],byte[]> edit,byte[] extra){
  using(var output=new MemoryStream()){
   while(p<end){var tag=Sig(b,p);int len=checked((int)BitConverter.ToUInt32(b,p+4));
    if(tag=="GRUP"){var header=Cut(b,p,24);var inside=Rewrite(b,p+24,p+len,edit,null);
     if(extra!=null&&Sig(b,p+8)=="ARTO"&&BitConverter.ToUInt32(b,p+12)==0)inside=Join(inside,extra);
     U32(header,4,(uint)(24+inside.Length));var g=Join(header,inside);output.Write(g,0,g.Length);p+=len;
    }else{var r=edit(Cut(b,p,24+len));output.Write(r,0,r.Length);p+=24+len;}
   }return output.ToArray();
  }
 }
 public static byte[] BuildTOOM(byte[] source){
  var old=Records(source);var template=old.Values.First(r=>Sig(r,0)=="ARTO");var owner=BitConverter.ToUInt32(template,12)&0xFF000000;
  var extra=new List<byte[]>();for(int i=0;i<192;++i){
   uint id=owner+(uint)(0xD00+i);if(old.ContainsKey(id))throw new Exception("new ID overlaps");
   string model=i<128?"effects\\ChargeCannon\\CP019\\Charge_Circle_Engine.nif":
    i<144?"effects\\TOOMShared\\Arcana\\v3\\Sun_Full.nif":
    i<160?"effects\\TOOMShared\\Arcana\\v3\\Sun_OuterBand.nif":
    "effects\\TOOMShared\\Arcana\\v3\\Circle_"+((i-160)%8)+".nif";
   var r=ReplaceSub(template,"EDID",Str("OOM_ART_Circle_"+i.ToString("D3")));
   r=ReplaceSub(r,"MODL",Str(model));U32(r,12,id);extra.Add(r);
  }
  var bytes=Rewrite(source,0,source.Length,r=>{
   if(Sig(r,0)!="TES4")return r;
   int q=24;while(q<r.Length){int len=BitConverter.ToUInt16(r,q+4);
    if(Sig(r,q)=="HEDR"){U32(r,q+10,BitConverter.ToUInt32(r,q+10)+192);U32(r,q+14,0xDC0);return r;}q+=6+len;
   }throw new Exception("HEDR missing");
  },Join(extra.ToArray()));
  var current=Records(bytes);if(current.Count!=old.Count+192)throw new Exception("new record count");
  foreach(var item in old){if(item.Key==0)continue;if(!current[item.Key].SequenceEqual(item.Value))throw new Exception("old record changed");}
  return bytes;
 }
 public static byte[] Rename(byte[] source,bool addon){
  NameChanges=0;var old=Records(source);var cards=new[]{"황제","은둔자","전차","태양","힘","정의","별","탑"};
  var bytes=Rewrite(source,0,source.Length,r=>{
   string type=Sig(r,0);uint id=BitConverter.ToUInt32(r,12)&0xFFFFFF;string name=null;
   if(!addon){
    if(type=="SPEL"&&id>=0x800&&id<=0x807)name=cards[id-0x800];
    if(type=="WEAP"&&id>=0x830&&id<=0x837)name=cards[id-0x830]+"의 지팡이";
    if(type=="BOOK"&&id>=0x840&&id<=0x847)name="마법서: "+cards[id-0x840];
   }else{
    var names=new[]{"여사제","달"};
    if(type=="SPEL"&&id>=0x800&&id<=0x801)name=names[id-0x800];
    if(type=="WEAP"&&id>=0x830&&id<=0x831)name=names[id-0x830]+"의 지팡이";
    if(type=="BOOK"&&id>=0x840&&id<=0x841)name="마법서: "+names[id-0x840];
    if(type=="MGEF"&&id>=0x870&&id<=0x871)name=names[id-0x870];
   }
   if(name==null)return r;++NameChanges;return ReplaceSub(r,"FULL",Str(name));
  },null);
  var current=Records(bytes);if(current.Count!=old.Count)throw new Exception("rename record count");
  foreach(var entry in old){
   var a=entry.Value;var b=current[entry.Key];if(a.SequenceEqual(b))continue;
   if(BitConverter.ToUInt32(a,12)!=BitConverter.ToUInt32(b,12))throw new Exception("FormID changed");
   // Every changed record differs only in FULL and derived payload length.
   var normalizedA=ReplaceSub(a,"FULL",new byte[]{0});var normalizedB=ReplaceSub(b,"FULL",new byte[]{0});
   if(!normalizedA.SequenceEqual(normalizedB))throw new Exception("non-FULL delta");
  }
  return bytes;
 }
}