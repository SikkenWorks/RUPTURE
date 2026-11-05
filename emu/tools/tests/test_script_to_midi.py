import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from script_to_midi import encode_midi,parse_script,variable_length

class TestMidiScript(unittest.TestCase):
    def test_midi_file_and_tempo(self):
        mid=encode_midi('0:m903C64,500:m803C00')
        self.assertEqual(mid[:4],b'MThd')
        self.assertEqual(mid[8:14],b'\x00\x00\x00\x01\x01\xe0')
        self.assertEqual(mid[14:18],b'MTrk')
        self.assertIn(b'\x00\x90\x3c\x64',mid)
        self.assertIn(b'\x83\x60\x80\x3c\x00',mid)  # 480 ticks=500 ms at 120 BPM
        self.assertTrue(mid.endswith(b'\x00\xff\x2f\x00'))
    def test_midi_multi_message(self):
        self.assertEqual(len(parse_script('100:m903C64803C00')),2)
        self.assertEqual(variable_length(127),b'\x7f')
        self.assertEqual(variable_length(128),b'\x81\x00')
    def test_invalid_nonmidi_rejected(self):
        for text in ('100:21','10:pot:100','100:m90','100:mF0FF','100:m903C80'):
            with self.subTest(text=text), self.assertRaises(ValueError): encode_midi(text)

if __name__=='__main__':unittest.main()
