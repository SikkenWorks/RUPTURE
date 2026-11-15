/* RUPTURE V1.91: 257-point Q15 rational transfer, every 512 PCM units.
 * f(x) = 32767*x/(32767+x), x=0..131072. Linear interpolation guarantees
 * a monotone bounded approximation within 2 Q15 counts of the original
 * variable-division shaper; no runtime divider and no additional RAM.
 * Stored in ROM/flash as uint16_t and shared by both channels. */
#ifndef RUPTURE_MOSFET_LUT_H
#define RUPTURE_MOSFET_LUT_H
#include <stdint.h>
static const uint16_t ru_mosfet_transfer[257]={
 0u, 504u, 992u, 1467u, 1927u, 2374u, 2808u, 3230u, 3640u, 4039u, 4428u, 4805u,
 5173u, 5532u, 5881u, 6221u, 6553u, 6877u, 7192u, 7501u, 7801u, 8095u, 8382u, 8662u,
 8936u, 9204u, 9466u, 9722u, 9972u, 10217u, 10457u, 10692u, 10922u, 11147u, 11368u, 11584u,
 11796u, 12003u, 12207u, 12407u, 12602u, 12794u, 12983u, 13168u, 13349u, 13527u, 13702u, 13874u,
 14043u, 14208u, 14371u, 14531u, 14688u, 14843u, 14995u, 15144u, 15291u, 15435u, 15578u, 15717u,
 15855u, 15990u, 16123u, 16254u, 16383u, 16510u, 16635u, 16758u, 16880u, 16999u, 17117u, 17233u,
 17347u, 17460u, 17570u, 17680u, 17788u, 17894u, 17999u, 18102u, 18204u, 18304u, 18403u, 18501u,
 18597u, 18692u, 18786u, 18879u, 18970u, 19060u, 19149u, 19237u, 19324u, 19409u, 19494u, 19578u,
 19660u, 19741u, 19822u, 19901u, 19980u, 20057u, 20134u, 20209u, 20284u, 20358u, 20431u, 20503u,
 20574u, 20645u, 20715u, 20783u, 20851u, 20919u, 20985u, 21051u, 21116u, 21181u, 21244u, 21307u,
 21370u, 21431u, 21492u, 21552u, 21612u, 21671u, 21729u, 21787u, 21844u, 21901u, 21957u, 22012u,
 22067u, 22122u, 22175u, 22229u, 22281u, 22333u, 22385u, 22436u, 22487u, 22537u, 22587u, 22636u,
 22685u, 22733u, 22781u, 22828u, 22875u, 22921u, 22967u, 23013u, 23058u, 23103u, 23147u, 23191u,
 23234u, 23278u, 23320u, 23363u, 23405u, 23446u, 23488u, 23528u, 23569u, 23609u, 23649u, 23688u,
 23728u, 23766u, 23805u, 23843u, 23881u, 23918u, 23955u, 23992u, 24029u, 24065u, 24101u, 24137u,
 24172u, 24207u, 24242u, 24276u, 24311u, 24345u, 24378u, 24412u, 24445u, 24478u, 24510u, 24543u,
 24575u, 24607u, 24638u, 24670u, 24701u, 24732u, 24763u, 24793u, 24823u, 24853u, 24883u, 24912u,
 24942u, 24971u, 25000u, 25028u, 25057u, 25085u, 25113u, 25141u, 25169u, 25196u, 25223u, 25250u,
 25277u, 25304u, 25330u, 25356u, 25383u, 25408u, 25434u, 25460u, 25485u, 25510u, 25535u, 25560u,
 25585u, 25609u, 25634u, 25658u, 25682u, 25706u, 25729u, 25753u, 25776u, 25800u, 25823u, 25846u,
 25868u, 25891u, 25913u, 25936u, 25958u, 25980u, 26002u, 26024u, 26045u, 26067u, 26088u, 26109u,
 26130u, 26151u, 26172u, 26193u, 26213u
};
static inline int16_t ru_mosfet_shaper(int32_t sample,uint16_t gain_q8){
 int32_t signed_mag=sample<0?-sample:sample;
 uint32_t magnitude=(uint32_t)signed_mag;
 if(magnitude>32768u)magnitude=32768u;
 magnitude=(magnitude*gain_q8+128u)>>8;
 if(magnitude>131072u)magnitude=131072u;
 uint32_t idx=magnitude>>9;
 uint32_t value=ru_mosfet_transfer[idx];
 if(idx<256u){
  uint32_t next=ru_mosfet_transfer[idx+1u];
  value+=((next-value)*(magnitude&511u)+256u)>>9;
 }
 return sample<0?-(int16_t)value:(int16_t)value;
}
#endif
