#!/usr/bin/env python3
"""Behavior tests for the application's boot-release guard and countdown game.

The stock demo's deep-sleep callback wiring is replaced by a two-sample release
barrier in this application, which does not enter deep sleep. Exercise the actual
shared C code, including held-at-boot/noisy release and all wire outcomes.
"""
import os
import struct
import wave
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class BombApplicationTest(unittest.TestCase):
    def test_real_sound_worker(self):
        with tempfile.TemporaryDirectory(prefix='bomb-sound-') as directory:
            executable = str(Path(directory) / 'bomb-sound')
            subprocess.run([os.environ.get('CC', 'cc'), '-std=c11', '-Wall', '-Wextra', '-Werror',
                '-I' + str(ROOT / 'tests/bomb_app_stubs'), '-I' + str(ROOT / 'tests/bsp_stubs'),
                '-I' + str(ROOT / 'components/bsp/include'), '-I' + str(ROOT / 'main'),
                str(ROOT / 'tests/test_bomb_sound.c'), '-o', executable], check=True)
            subprocess.run([executable, str(ROOT / 'assets/music/bomb-explosion.pcm')], check=True)

    def test_explosion_asset_has_transient_decay_and_silent_drain(self):
        pcm = (ROOT / 'assets/music/bomb-explosion.pcm').read_bytes()
        with wave.open(str(ROOT / 'assets/music/bomb-explosion.wav'), 'rb') as audio:
            self.assertEqual((audio.getnchannels(), audio.getsampwidth(), audio.getframerate()), (1, 2, 16000))
            self.assertEqual(audio.readframes(audio.getnframes()), pcm)
        values = struct.unpack('<' + 'h' * (len(pcm) // 2), pcm)
        self.assertEqual(len(values), 30720)
        self.assertLessEqual(max(abs(x) for x in values), 24000)
        self.assertGreater(max(abs(x) for x in values), 20000)
        self.assertTrue(all(x == 0 for x in values[-2048:]))
        self.assertLess(sum(x*x for x in values[20000:22000]), sum(x*x for x in values[4000:6000]) / 10)

    def test_real_application_callback_and_queue_loop(self):
        with tempfile.TemporaryDirectory(prefix='bomb-app-') as directory:
            executable = str(Path(directory) / 'bomb-app')
            subprocess.run([os.environ.get('CC', 'cc'), '-std=c11', '-Wall', '-Wextra', '-Werror',
                '-I' + str(ROOT / 'tests/bomb_app_stubs'),
                '-I' + str(ROOT / 'tests/bsp_stubs'),
                '-I' + str(ROOT / 'components/bsp/include'), '-I' + str(ROOT / 'main'),
                str(ROOT / 'tests/test_bomb_app.c'), str(ROOT / 'main/bomb_game.c'),
                '-o', executable], check=True)
            subprocess.run([executable], check=True)

    def test_startup_guard_and_round_lifecycle(self):
        with tempfile.TemporaryDirectory(prefix='bomb-model-') as directory:
            executable = str(Path(directory) / 'bomb-model')
            subprocess.run([os.environ.get('CC', 'cc'), '-std=c11', '-Wall', '-Wextra', '-Werror',
                '-I' + str(ROOT / 'main'), str(ROOT / 'tests/test_bomb_game.c'),
                str(ROOT / 'main/bomb_game.c'), '-o', executable], check=True)
            subprocess.run([executable], check=True)


if __name__ == '__main__': unittest.main()
