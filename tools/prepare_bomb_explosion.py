#!/usr/bin/env python3
"""Convert the user-supplied explosion to streamed PCM; requires local FFmpeg."""
import hashlib
import json
import struct
import subprocess
import wave
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DIRECTORY = ROOT / 'assets/music'


def main():
    source = DIRECTORY / 'xm4167.mp3'
    args = ['ffmpeg', '-hide_banner', '-loglevel', 'error', '-i', str(source),
            '-map_metadata', '-1', '-ac', '1', '-ar', '16000', '-c:a', 'pcm_s16le', '-f', 's16le', '-']
    raw = subprocess.check_output(args)
    samples = list(struct.unpack('<' + 'h' * (len(raw) // 2), raw))
    source_samples = len(samples)
    peak = max(abs(x) for x in samples)
    gain = min(1.0, 24000 / max(1, peak))  # Attenuate peaks; never boost quiet material.
    samples = [round(x * gain) for x in samples]
    last_signal = max((i for i, x in enumerate(samples) if x), default=-1)
    samples += [0] * max(0, 2048 - (len(samples) - last_signal - 1))
    samples += [0] * (-len(samples) % 512)
    pcm = struct.pack('<' + 'h' * len(samples), *samples)
    (DIRECTORY / 'bomb-explosion.pcm').write_bytes(pcm)
    with wave.open(str(DIRECTORY / 'bomb-explosion.wav'), 'wb') as audio:
        audio.setparams((1, 2, 16000, 0, 'NONE', 'not compressed'))
        audio.writeframes(pcm)
    record = {'source': 'xm4167.mp3', 'source_sha256': hashlib.sha256(source.read_bytes()).hexdigest(),
              'source_decoded_samples_16khz': source_samples, 'gain': gain,
              'pcm_sha256': hashlib.sha256(pcm).hexdigest(), 'pcm_bytes': len(pcm),
              'rate': 16000, 'bits': 16, 'channels': 1, 'duration_ms': len(samples) / 16,
              'tail_silence_min_ms': 128, 'chunk_samples': 512,
              'ffmpeg': subprocess.check_output(['ffmpeg', '-version'], text=True).splitlines()[0],
              'license': 'User supplied; no separate license provided.'}
    (DIRECTORY / 'bomb-explosion-source.json').write_text(json.dumps(record, indent=2) + '\n')
    print(f'Explosion: {len(pcm)} PCM bytes, {len(samples)/16000:.3f}s; full source retained, peak {max(abs(x) for x in samples)}')


if __name__ == '__main__':
    main()
