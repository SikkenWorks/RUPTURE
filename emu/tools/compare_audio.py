#!/usr/bin/env python3
"""Offline FM-1 audio A/B comparison (stdlib only).

Input: two *16-bit PCM* WAV recordings, from the same firmware, patch,
MIDI test and sample rate. This tool measures agreement; it does NOT
establish chip-level accuracy or claim identical output stages.
"""
import argparse
import array
import json
import math
import sys
import wave
from pathlib import Path


def read_mono(path):
    with wave.open(str(path), 'rb') as wav:
        channels = wav.getnchannels()
        sample_rate = wav.getframerate()
        if channels not in (1, 2) or wav.getsampwidth() != 2 or wav.getcomptype() != 'NONE':
            raise ValueError(f'{path}: only mono/stereo 16-bit PCM WAV is supported')
        # Cap input size: >60s stereo captures are unnecessary for timing calibration.
        if wav.getnframes() > sample_rate * 60:
            raise ValueError(f'{path}: record a clip shorter than 60 seconds')
        raw = wav.readframes(wav.getnframes())
    values = array.array('h')
    values.frombytes(raw)
    if sys.byteorder != 'little':
        values.byteswap()
    if channels == 2:
        mono = [(values[i] + values[i + 1]) / 65536.0 for i in range(0, len(values), 2)]
    else:
        mono = [v / 32768.0 for v in values]
    return sample_rate, mono


def correlation(a, b):
    if len(a) != len(b) or len(a) < 32:
        return 0.0
    n = len(a)
    sa = sum(a); sb = sum(b)
    ab = sum(x * y for x, y in zip(a, b)) - sa * sb / n
    aa = sum(x * x for x in a) - sa * sa / n
    bb = sum(y * y for y in b) - sb * sb / n
    return ab / math.sqrt(aa * bb) if aa > 1e-10 and bb > 1e-10 else 0.0


def aligned(a, b, lag):
    # Positive lag means the emulator file begins its waveform later than the FM-1 recording.
    if lag >= 0:
        length = min(len(a), len(b) - lag)
        return a[:max(0, length)], b[lag:lag + max(0, length)]
    lag = -lag
    length = min(len(a) - lag, len(b))
    return a[lag:lag + max(0, length)], b[:max(0, length)]


def analyze(reference, emulator, sample_rate, max_lag_ms=250, skip_seconds=0, duration=4.0):
    if not (10 <= max_lag_ms <= 1500):
        raise ValueError('max_lag_ms must be 10–1500')
    start = int(skip_seconds * sample_rate)
    count = int(duration * sample_rate)
    a = reference[start:start + count]
    b = emulator[start:start + count + int(max_lag_ms * sample_rate)]
    if len(a) < sample_rate // 4 or len(b) < sample_rate // 4:
        raise ValueError('recordings are too short for the requested window')
    # Coarse cross-correlation at ~2kHz. A single lag is then used to measure full-rate audio.
    stride = max(1, round(sample_rate / 2000))
    search_a = a[::stride]
    search_b = b[::stride]
    lag_steps = math.ceil(max_lag_ms * sample_rate / (1000 * stride))
    best = (-1.0, 0, 0.0)
    for lag in range(-lag_steps, lag_steps + 1):
        x, y = aligned(search_a, search_b, lag)
        if len(x) < len(search_a) // 2:
            continue
        r = correlation(x, y)
        if abs(r) > best[0]:
            best = (abs(r), lag * stride, r)
    if best[0] < 0.05:
        raise ValueError('no clear correlation: compare the same patch, notes and timing')
    # Refine by up to a decimation stride in original sample rate.
    best_full = (-1.0, best[1], 0.0)
    for lag in range(best[1] - stride, best[1] + stride + 1):
        x, y = aligned(a, b, lag)
        x = x[: min(len(x), sample_rate)]
        y = y[:len(x)]
        r = correlation(x[::4], y[::4])
        if abs(r) > best_full[0]:
            best_full = (abs(r), lag, r)
    _, lag, r = best_full
    x, y = aligned(a, b, lag)
    if len(x) < sample_rate // 4:
        raise ValueError('insufficient aligned audio')
    n = len(x)
    mean_x = sum(x) / n; mean_y = sum(y) / n
    xx = sum((v - mean_x)**2 for v in x)
    yy = sum((v - mean_y)**2 for v in y)
    xy = sum((vx - mean_x)*(vy - mean_y) for vx, vy in zip(x, y))
    if xx < 1e-8 or yy < 1e-8:
        raise ValueError('mostly silent capture; record a clearly audible note')
    gain = xy / yy
    residual = sum(((vx - mean_x) - gain*(vy - mean_y))**2 for vx,vy in zip(x,y))
    rms_ref = math.sqrt(xx/n)
    rms_emu = math.sqrt(yy/n)
    return {
        'sampleRate': sample_rate,
        'reference': 'physical FM-1 audio capture',
        'other': 'emulated firmware audio export',
        'analyzedSeconds': round(n/sample_rate,3),
        'alignmentLagMs': round(lag*1000/sample_rate,3),
        'correlation': round(xy/math.sqrt(xx*yy),6),
        'phaseInverted': gain < 0,
        'emulatorRmsVsPhysicalDb': round(20 * math.log10(rms_emu/rms_ref),3),
        'gainToMatchPhysical': round(gain,6),
        'normalizedResidualRms': round(math.sqrt(residual/xx),6),
        'physicalPeak': round(max(abs(v) for v in x),6),
        'emulatorPeak': round(max(abs(v) for v in y),6),
        'interpretation': 'Numerical audio similarity, not proof of hardware-cycle accuracy.',
    }


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('physical_wav', type=Path)
    ap.add_argument('emulator_wav', type=Path)
    ap.add_argument('--max-lag-ms', type=int, default=250)
    ap.add_argument('--skip-seconds', type=float, default=0)
    ap.add_argument('--duration', type=float, default=4)
    ap.add_argument('--json', type=Path, help='write machine-readable result here')
    args = ap.parse_args(argv)
    try:
        sr_a, a = read_mono(args.physical_wav)
        sr_b, b = read_mono(args.emulator_wav)
        if sr_a != sr_b:
            raise ValueError(f'sample rates differ: {sr_a} vs {sr_b}; use identical recording rates')
        result = analyze(a,b,sr_a,args.max_lag_ms,args.skip_seconds,args.duration)
    except (ValueError, OSError, wave.Error) as exc:
        ap.error(str(exc))
    print(json.dumps(result, indent=2))
    if args.json:
        args.json.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')

if __name__=='__main__':
    main()
