#!/usr/bin/env python3
"""Run firmware save/upload methods on a host with an in-memory FS and flash.

No ESP8266 is needed. The harness compiles the actual method bodies and headers,
including the HTTP upload callback; it does not emulate WiFi or flash hardware.
"""
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "firmware/src"


def function(source, signature):
    start = source.index(signature)
    return source[start:source.index("\n}", start) + 2]


def main():
    config = (SRC / "config/config_store.cpp").read_text()
    ota = (SRC / "ota/ota_manager.cpp").read_text()
    web = (SRC / "web/web_server.cpp").read_text()
    bodies = [function(config, f"bool ConfigStore::{name}(")
              for name in ("writeAtomic", "save")]
    bodies.append(ota[ota.index("inline constexpr uint32_t OTA_STATE_MAGIC"):
                      ota.index("uint32_t readLe32")])
    for signature in (
        "uint32_t readLe32(", "void verifyReason(", "bool imageHeaderValid(",
        "void OtaManager::reject(", "bool OtaManager::isUploading(",
        "void OtaManager::onUploadStart(", "bool OtaManager::onUploadChunk(",
        "void OtaManager::onUploadEnd(", "void OtaManager::onUploadAbort(",
    ):
        bodies.append(function(ota, signature))
    marker = "        []() {\n            HTTPUpload& upload"
    start = web.index(marker) + len("        []() {")
    end = web.index("\n        });", start)
    bodies.append("void handleUpload() {" + web[start:end] + "\n}")
    driver = (SRC / "rboot-api.cpp").read_text()
    bodies.append(function(driver, "bool rboot_write_flash(").replace(
        "bool rboot_write_flash(", "bool driver_write_flash(", 1))

    with tempfile.TemporaryDirectory(prefix="gk01-regression-") as tmp:
        tmp = Path(tmp)
        # Production headers depend on Arduino and LittleFS. Test doubles are
        # defined before including them in the harness.
        (tmp / "Arduino.h").write_text("#pragma once\n")
        (tmp / "LittleFS.h").write_text("#pragma once\n")
        (tmp / "methods.inc").write_text("\n\n".join(bodies))
        binary = tmp / "storage_ota_test"
        subprocess.run([
            os.environ.get("CXX", "c++"), "-std=c++17", "-Wall", "-Wextra",
            "-DIRAC_RBOOT_OTA=1", f"-I{tmp}", f"-I{SRC}",
            str(ROOT / "tests/host/storage_ota.cpp"), "-o", str(binary),
        ], check=True)
        subprocess.run([str(binary)], check=True)


if __name__ == "__main__":
    main()
