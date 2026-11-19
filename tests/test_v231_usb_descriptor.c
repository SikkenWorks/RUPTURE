/* Host regression for the UAC1 continuous singleton-rate descriptor. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "rupture_usb_rate_descriptor.h"

static int check(uint8_t rate48)
{
    /* One ordinary interface followed by a UAC Type-I 14-byte FORMAT_TYPE.
     * This exact shape occurs in both the USB-only and CDC composite layouts. */
    uint8_t cfg[]={9,2,32,0,1,1,0,0x80,50,
                   9,4,1,0,0,1,2,0,0,
                   14,0x24,2,1,2,2,16,2,0x44,0xac,0,0x80,0xbb,0};
    uint8_t old[sizeof cfg];
    memcpy(old,cfg,sizeof cfg);
    if(!ru_uac_restrict_format(cfg,sizeof cfg,rate48))return 1;
    uint32_t hz=rate48?48000u:44100u;
    if(sizeof cfg!=32u||cfg[25]!=0u)return 2;
    for(unsigned j=0;j<2;j++)
        for(unsigned b=0;b<3;b++)
            if(cfg[26+j*3+b]!=(uint8_t)(hz>>(8*b)))return 3;
    for(unsigned i=0;i<sizeof cfg;i++)
        if(i!=25u&&(i<26u||i>=32u)&&cfg[i]!=old[i])return 4;
    if(!ru_uac_restrict_format(cfg,sizeof cfg,!rate48))return 5;
    hz=rate48?44100u:48000u;
    if(cfg[26]!=(uint8_t)hz||cfg[27]!=(uint8_t)(hz>>8)||cfg[28]!=(uint8_t)(hz>>16))return 6;
    return 0;
}

int main(void)
{
    const uint8_t non_audio[]={9,2,18,0,0,1,0,0x80,50,9,4,0,0,0,0,0,0,0};
    uint8_t untouched[sizeof non_audio];
    memcpy(untouched,non_audio,sizeof non_audio);
    if(ru_uac_restrict_format(untouched,sizeof untouched,1u))return 7;
    if(memcmp(untouched,non_audio,sizeof non_audio))return 8;
    int a=check(0u),b=check(1u);
    if(a||b){fprintf(stderr,"USB descriptor failed: %d %d\n",a,b);return 1;}
    puts("PASS V2.0.31 UAC1 Type-I 44.1/48 singleton range, descriptor length intact");
    return 0;
}
