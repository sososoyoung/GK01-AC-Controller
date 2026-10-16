#!/usr/bin/env python3
"""Regression checks for external code conversion and built-in TV waveforms."""
import base64
import json
from pathlib import Path
import re
import unittest
from convert_ir_codes import broadlink, convert, flipper, nec, normalize, smartir

ROOT = Path(__file__).resolve().parents[1]


class IrCodesTest(unittest.TestCase):
    def test_xiaomi_preserves_every_timing(self):
        result = convert(ROOT / 'data/ir/xiaomi_tv.ir')
        self.assertEqual(result, json.loads((ROOT / 'data/ir/xiaomi_tv.json').read_text()))
        self.assertEqual(len(result['signals']), 11)
        cpp = (ROOT / 'firmware/src/hw/tv_remote.cpp').read_text()
        arrays = re.findall(r'const uint16_t (\w+)\[\] PROGMEM = \{([^}]+)\}', cpp)
        for (_, body), signal in zip(arrays, result['signals'], strict=True):
            self.assertEqual([int(x) for x in re.findall(r'\d+', body)], signal['raw'])

    def test_long_duration_keeps_polarity(self):
        self.assertEqual(normalize([9000, 140000, 560]), [9000, 65535, 0, 65535, 0, 8930, 560])
        with self.assertRaises(ValueError):
            normalize([1] * 513)

    def test_broadlink_extended_and_repeat(self):
        packet = bytes([0x26, 1, 4, 0, 10, 0, 1, 0])
        self.assertEqual(broadlink(base64.b64encode(packet)), [328, 8407, 328, 8407])
        with self.assertRaises(ValueError):
            broadlink(base64.b64encode(bytes([0x26, 0, 3, 0, 0, 1])))
        with self.assertRaises(ValueError):
            broadlink(base64.b64encode(bytes([0xB2, 0, 0, 0])))

    def test_smartir_nested_commands_and_reject_unknown(self):
        result = smartir({'commandsEncoding': 'Raw', 'commands': {'cool': {'auto': {'26': [9000, -4500, 560]}}}}, 40000)
        self.assertEqual(result[0], {'name': 'cool / auto / 26', 'frequency': 40000, 'raw': [9000, 4500, 560]})
        with self.assertRaises(ValueError):
            smartir({'commandsEncoding': 'Base64', 'supportedController': 'Xiaomi'}, 38000)

    def test_nec_bit_order(self):
        values = nec(1, 0x15)
        self.assertEqual(values[:8], [9000, 4500, 560, 1690, 560, 560, 560, 560])
        self.assertEqual(len(values), 68)
        result = flipper('Filetype: IR signals file\nname: Power\ntype: parsed\nprotocol: NEC\naddress: 01 00 00 00\ncommand: 15 00 00 00\n')
        self.assertEqual(result[0]['raw'], values)
        with self.assertRaises(ValueError):
            nec(256, 1)


if __name__ == '__main__':
    unittest.main()
