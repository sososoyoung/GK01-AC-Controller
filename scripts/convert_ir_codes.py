#!/usr/bin/env python3
"""Convert Flipper RAW/NEC or SmartIR Broadlink/Raw codes to GK01 JSON.

No network access or third-party Python packages are required.
"""
import argparse
import base64
import json
from pathlib import Path


def normalize(values):
    """Split long durations without changing mark/space parity."""
    result = []
    for value in values:
        if isinstance(value, bool) or not isinstance(value, int) or value <= 0:
            raise ValueError("原始时序必须为正整数（微秒）")
        while value > 65535:
            result.extend((65535, 0))
            value -= 65535
        result.append(value)
    if not result or len(result) > 512:
        raise ValueError("单条信号为空或超过固件 512 个时序的限制")
    return result


def broadlink(encoded):
    packet = base64.b64decode(encoded, validate=True)
    if len(packet) < 4 or packet[0] != 0x26:
        raise ValueError("仅支持 Broadlink 红外数据包（0x26）")
    end = 4 + int.from_bytes(packet[2:4], "little")
    if end > len(packet):
        raise ValueError("Broadlink 数据包长度不完整")
    values, i = [], 4
    while i < end:
        ticks = packet[i]
        i += 1
        if ticks == 0:
            if i + 2 > end:
                raise ValueError("Broadlink 扩展时序不完整")
            ticks = int.from_bytes(packet[i:i + 2], "big")
            i += 2
        values.append(int(ticks * 32.84))
    # Header repeat byte specifies additional transmissions.
    return values * (packet[1] + 1)


def nec(address, command):
    if not 0 <= address <= 255 or not 0 <= command <= 255:
        raise ValueError("仅支持 8 位 NEC 地址和命令；其他协议请导出 RAW")
    values = [9000, 4500]
    for byte in (address, address ^ 255, command, command ^ 255):
        for bit in range(8):
            values.extend((560, 1690 if byte & (1 << bit) else 560))
    values.extend((560, 40000))
    return values


def flipper(text):
    signals, current = [], {}

    def append():
        if not current:
            return
        if current.get("type") == "raw":
            values = [int(x) for x in current["data"].split()]
        elif current.get("type") == "parsed" and current.get("protocol") == "NEC":
            address = int.from_bytes(bytes.fromhex(current["address"]), "little")
            command = int.from_bytes(bytes.fromhex(current["command"]), "little")
            values = nec(address, command)
        else:
            raise ValueError(f"{current.get('name')}: 不支持的协议，请提供 RAW 格式")
        signals.append({"name": current["name"], "frequency": int(current.get("frequency", 38000)), "raw": normalize(values)})

    for line in text.splitlines():
        if not line.strip() or line.lstrip().startswith("#") or ":" not in line:
            continue
        key, value = (x.strip() for x in line.split(":", 1))
        if key == "name":
            append()
            current = {}
        if current or key == "name":
            current[key] = value
    append()
    return signals


def smartir(data, frequency):
    encoding = data.get("commandsEncoding", "").lower()
    controller = data.get("supportedController", "").lower()
    if encoding != "raw" and not (encoding == "base64" and controller == "broadlink"):
        raise ValueError("SmartIR 仅支持 Raw 或 Broadlink Base64；不支持 Xiaomi/LOOKin 专有编码")
    signals = []

    def visit(node, path):
        if isinstance(node, dict):
            for key, value in node.items():
                visit(value, path + [str(key)])
            return
        if encoding == "base64":
            values = broadlink(node)
        else:
            values = node if isinstance(node, list) else [int(x) for x in node.replace(',', ' ').split()]
            values = [abs(x) for x in values]
        signals.append({"name": " / ".join(path), "frequency": frequency, "raw": normalize(values)})

    visit(data["commands"], [])
    return signals


def convert(path, frequency=38000):
    text = Path(path).read_text(encoding="utf-8-sig")
    signals = flipper(text) if text.startswith("Filetype:") else smartir(json.loads(text), frequency)
    if not 1 <= len(signals) <= 500:
        raise ValueError("码集必须包含 1～500 个按钮；请按模式或型号拆分")
    for signal in signals:
        if not signal['name'] or len(signal['name']) > 120 or not 20000 <= signal['frequency'] <= 60000:
            raise ValueError("按键名称或载波频率超出限制")
    return {"version": 1, "source": Path(path).name, "signals": signals}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--frequency", type=int, default=38000, help="SmartIR 载波频率（Hz），默认 38000")
    args = parser.parse_args()
    try:
        result = convert(args.input, args.frequency)
    except (ValueError, KeyError, TypeError) as exc:
        parser.error(str(exc))
    args.output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n")
    print(f"已转换 {len(result['signals'])} 个按键 → {args.output}")


if __name__ == "__main__":
    main()
