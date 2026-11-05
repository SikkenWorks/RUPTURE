#!/usr/bin/env python3
"""Convert MIDI-only FM1_BENCH_KEYS actions to standard MIDI Type-0 (120 BPM).

A single source script can then drive the emulator and a DAW's physical MIDI output.
Only standard 2-/3-byte channel voice events are supported; no SysEx or arbitrary
hardware actions (keys, encoders, pots) are silently approximated.
"""
import argparse
import re
from pathlib import Path

EVENT = re.compile(r'(\d+)\s*:\s*m([0-9a-fA-F]+)\Z')

def parse_script(text):
    events=[]
    for item in text.replace('\n','').split(','):
        item = item.strip()
        if not item: continue
        match=EVENT.fullmatch(item)
        if not match:
            raise ValueError(f'unsupported action {item!r}; reference MIDI uses ms:mHEX only')
        ms=int(match.group(1)); data=bytes.fromhex(match.group(2))
        if ms>60000 or not data:
            raise ValueError('events must be within 60 seconds, with a nonempty payload')
        at=0
        while at < len(data):
            status=data[at]
            if status < 0x80 or status >= 0xF0:
                raise ValueError('only complete MIDI channel-voice messages with status bytes are supported')
            length=2 if status & 0xF0 in (0xC0,0xD0) else 3
            msg=data[at:at+length]
            if len(msg)!=length or any(b>=0x80 for b in msg[1:]):
                raise ValueError('invalid MIDI message length or data byte')
            events.append((ms,msg))
            at+=length
        if len(events)>10000:
            raise ValueError('too many MIDI events')
    if not events:
        raise ValueError('no MIDI events found')
    return sorted(events,key=lambda e:e[0])

def variable_length(value):
    out=[value&0x7f]; value >>= 7
    while value:
        out.append((value&0x7f)|0x80)
        value >>= 7
    return bytes(reversed(out))

def encode_midi(text):
    events=parse_script(text)
    track=bytearray(b'\x00\xff\x51\x03\x07\xa1\x20') # 120 BPM
    last=0
    for ms, msg in events:
        tick=round(ms*0.96)  # 480 PPQ * (120/60) = 960 ticks/s
        track.extend(variable_length(tick-last)); track.extend(msg)
        last=tick
    track.extend(b'\x00\xff\x2f\x00')
    header=b'MThd'+(6).to_bytes(4,'big')+b'\x00\x00\x00\x01\x01\xe0'
    return header+b'MTrk'+len(track).to_bytes(4,'big')+track

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('input',type=Path,help='timestamped FM1_BENCH_KEYS text file')
    parser.add_argument('output',type=Path,help='result .mid file')
    args=parser.parse_args()
    try: data=encode_midi(args.input.read_text(encoding='utf-8'))
    except ValueError as err: parser.error(str(err))
    args.output.write_bytes(data)
    print(f'Wrote {args.output}: {len(data)} bytes, 120 BPM, MIDI Type 0')

if __name__=='__main__':main()
