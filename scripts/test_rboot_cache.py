#!/usr/bin/env python3
"""Exercise the real early-cache wrapper without an ESP8266."""
import os
import argparse
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def verify_linked(elf):
    suffix = ".exe" if os.name == "nt" else ""
    toolchain = Path(os.environ.get(
        "XTENSA_BINDIR", str(Path.home() / ".platformio/packages/toolchain-xtensa/bin")
    ))
    def output(tool, *args):
        return subprocess.check_output([
            str(toolchain / ("xtensa-lx106-elf-" + tool + suffix)), *args, str(elf)
        ], text=True)
    symbols = output("nm")
    wrapper = next(line.split()[0] for line in symbols.splitlines()
                   if line.endswith(" T __wrap_Cache_Read_Enable"))
    assert 0x40100000 <= int(wrapper, 16) < 0x40108000, "cache wrapper must reside in IRAM"
    bank = next(line.split()[0] for line in symbols.splitlines()
                if line.endswith(" d _ZN12_GLOBAL__N_110mappedBankE"))
    assert 0x3FFE8000 <= int(bank, 16) < 0x40000000, "bank latch must reside in initialized DRAM"
    sdk = output("objdump", "-d", "--disassemble=Cache_Read_Enable_New")
    assert sdk.count("<__wrap_Cache_Read_Enable>") == 2, "both SDK branches must call the mapping wrapper"
    print(f"PASS linked mapping: {elf} (IRAM wrapper, DRAM latch, both SDK calls redirected)")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--elf", type=Path, action="append", default=[])
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="gk01-cache-") as folder:
        folder = Path(folder)
        (folder / "Arduino.h").write_text(
            "#pragma once\n#include <stdint.h>\n#include <stddef.h>\n#define IRAM_ATTR\n"
        )
        (folder / "spi_flash.h").write_text("#pragma once\n")
        source = '#include "' + str(ROOT / "firmware/src/rboot-cache.cpp") + '"\n'
        source += r'''
#include <cassert>
#include <cstring>
#include <cstdio>
static rboot_config stored;
static unsigned reads, actualMap, actualPage, actualSize;
static bool failRead;
extern "C" int SPIRead(uint32_t address, void* data, size_t size) {
    assert(address == RBOOT_CONFIG_ADDRESS && size == sizeof(stored));
    ++reads;
    if (failRead) return 1;
    memcpy(data, &stored, size);
    return 0;
}
extern "C" void __real_Cache_Read_Enable(uint8_t map, uint8_t page, uint8_t size) {
    actualMap = map; actualPage = page; actualSize = size;
}
static void reset(unsigned rom) {
    stored = {RBOOT_CONFIG_MAGIC, RBOOT_CONFIG_VERSION, 0,
              uint8_t(rom), 0, 2, {0, 0}, {ROM0_ADDRESS, ROM1_ADDRESS}};
    mappedBank = 0xff; reads = 0; failRead = false;
}
int main() {
    for (unsigned rom = 0; rom < 2; ++rom) {
        reset(rom);
        // Arduino's early heap path asks for bank 0 and 16KB cache.
        __wrap_Cache_Read_Enable(0, 0, 0);
        assert(actualMap == rom && actualPage == 0 && actualSize == 0);
        assert(rbootMappedBank() == rom && reads == 1);
        // SDK path asks for bank 0 and 32KB cache, even when booting ROM1.
        __wrap_Cache_Read_Enable(0, 0, 1);
        assert(actualMap == rom && actualSize == 1 && reads == 1);
        // OTA switches the next boot selection while this ROM is still running.
        stored.current_rom = 1 - rom;
        __wrap_Cache_Read_Enable(0, 0, 1);
        assert(actualMap == rom && reads == 1);
        // A reboot loads initialized data afresh and then selects the new bank.
        mappedBank = 0xff;
        __wrap_Cache_Read_Enable(0, 0, 0);
        assert(actualMap == 1 - rom && reads == 2);
    }
    for (unsigned bad = 0; bad < 7; ++bad) {
        reset(1);
        switch (bad) {
            case 0: failRead = true; break;
            case 1: stored.magic = 0; break;
            case 2: stored.version = 0; break;
            case 3: stored.count = 0; break;
            case 4: stored.count = 3; break;
            case 5: stored.current_rom = 255; break;
            case 6: stored.roms[1] = 0xffffffff; break;
        }
        __wrap_Cache_Read_Enable(1, 1, 0);
        assert(actualMap == 0 && actualPage == 0 && actualSize == 0);
    }
    puts("PASS rboot cache: ROM0/ROM1 early+SDK mapping, cache sizes, latch across OTA, next boot, invalid config/read failure");
}
'''
        (folder / "test.cpp").write_text(source)
        binary = folder / "cache_test"
        subprocess.run([
            os.environ.get("CXX", "c++"), "-std=c++17", "-Wall", "-Wextra", "-Werror",
            "-DIRAC_RBOOT_OTA=1", f"-I{folder}", f"-I{ROOT / 'firmware/rboot'}",
            f"-I{ROOT / 'firmware/src'}", str(folder / "test.cpp"), "-o", str(binary),
        ], check=True)
        subprocess.run([str(binary)], check=True)
    for elf in args.elf:
        verify_linked(elf)


if __name__ == "__main__":
    main()
