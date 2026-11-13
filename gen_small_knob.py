from pathlib import Path
import re
R=Path('/mnt/data/rupture-v1861/vendor/etch-v1.88-source/firmware/fm1-native')
s=(R/'rupture_knob_ring_pixels.h').read_text()
pts=[(int(a),int(b)) for a,b in re.findall(r'\{(-?\d+),(-?\d+)\}',s)]
# Offline nearest-neighbour ring bitmap reduction: original 45x45 -> 33x33.
full={(x,y) for x,y in pts}
small=set()
for y in range(-16,17):
 for x in range(-16,17):
  ix=round(x*22/16)
  iy=round(y*22/16)
  if (ix,iy) in full:
   small.add((x,y))
# Guarantee a contiguous edge at scale by raster-distance approximation.
for x,y in pts:
 small.add((round(x*16/22),round(y*16/22)))
# dedupe, sorted for reproducibility
small=sorted(small,key=lambda z:(z[1],z[0]))
h=R/'rupture_bend_knob_pixels.h'
h.write_text('''/* RUPTURE BEND ONLY: offline downsampled 33x33 ring geometry.
 * Other RUPTURE knobs remain their existing 45x45 original bitmap.
 * Not resized during rendering; all positions are precomputed integer pixels.
 * SPDX-License-Identifier: GPL-3.0-only */
#ifndef RUPTURE_BEND_KNOB_PIXELS_H
#define RUPTURE_BEND_KNOB_PIXELS_H
#include <stdint.h>
static const int8_t ru_bend_knob_ring[][2]={
'''+',\n'.join(' {'+str(x)+','+str(y)+'}' for x,y in small)+'''
};
#define RU_BEND_KNOB_PIXELS (sizeof(ru_bend_knob_ring)/sizeof(ru_bend_knob_ring[0]))
#endif
''')
print('Ring pixels',len(small),'bounds',min(x for x,y in small),max(x for x,y in small),min(y for x,y in small),max(y for x,y in small))
p=R/'rupture_ui.c'
s=p.read_text()
s=s.replace('#include "rupture_bend_curves.h"\n','#include "rupture_bend_curves.h"\n#include "rupture_bend_knob_pixels.h"\n',1)
needle='static void ru_knob_focus_marker(const RuState *v,int x,int y,uint8_t control){'
assert needle in s
s=s.replace(needle,'''static void ru_bend_knob(const RuState *v,int x,int y,uint8_t control){
 uint32_t value=v->animate?v->knob_visual[control]:ru_knob_normal(v,control);
 if(value>100u)value=100u;
 for(uint32_t j=0u;j<RU_BEND_KNOB_PIXELS;j++)
  ru_set(x+ru_bend_knob_ring[j][0],y+ru_bend_knob_ring[j][1]);
 const int nx=(int)ru_needle_xy[value][0]*16/22;
 const int ny=(int)ru_needle_xy[value][1]*16/22;
 ru_line(x,y,x+nx,y+ny);ru_rect(x-1,y-1,3,3);
 if(v->knob_focus_visible && v->knob_focus==control){
  ru_rect(x-21,y-5,2,10);ru_rect(x+19,y-5,2,10);
 }
}
'''+needle,1)
old='''  static const int bx[3]={39,120,201};
  for(int k=0;k<3;k++){
   ru_knob(bx[k],191,v->animate?v->knob_visual[k]:ru_knob_normal(v,k));
   ru_knob_focus_marker(v,bx[k],191,(uint8_t)k);
  }
'''
new='''  static const int bx[3]={39,120,201};
  for(int k=0;k<3;k++)ru_bend_knob(v,bx[k],188,(uint8_t)k);
'''
assert old in s
s=s.replace(old,new,1)
p.write_text(s)
