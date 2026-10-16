#include "core/rboot_cache.h"
#include "rboot.h"

#if IRAC_RBOOT_OTA
namespace {
// Initialized data is loaded by rBoot before the SDK starts. Do not put this
// in flash: the wrapper runs even before flash code can be accessed.
uint8_t mappedBank = 0xff;
}

extern "C" int SPIRead(uint32_t address, void* data, size_t size);
extern "C" void __real_Cache_Read_Enable(uint8_t map, uint8_t page, uint8_t size);

// Wrap the ROM API, rather than only Cache_Read_Enable_New: Arduino also
// enables the cache before SDK initialization, to initialize its heap.
// Every instruction and literal on this path must reside in IRAM/DRAM.
extern "C" void IRAM_ATTR __wrap_Cache_Read_Enable(uint8_t, uint8_t, uint8_t size) {
    if (mappedBank == 0xff) {
        rboot_config conf;
        uint8_t bank = 0;
        if (SPIRead(RBOOT_CONFIG_ADDRESS, &conf, sizeof(conf)) == 0 &&
            conf.magic == RBOOT_CONFIG_MAGIC &&
            conf.version == RBOOT_CONFIG_VERSION &&
            conf.count > 0 && conf.count <= RBOOT_MAX_ROMS &&
            conf.current_rom < conf.count) {
            const uint32_t address = conf.roms[conf.current_rom];
            if (address == ROM0_ADDRESS || address == ROM1_ADDRESS) {
                bank = static_cast<uint8_t>(address >> 20);
            }
        }
        // Latch once. OTA changes the boot config while this ROM is still
        // running; switching its mapping at that point would crash it.
        mappedBank = bank;
    }
    // Preserve Arduino's early 16KB-cache selection and the SDK's later size.
    __real_Cache_Read_Enable(mappedBank & 1, mappedBank >> 1, size);
}

uint8_t rbootMappedBank() { return mappedBank; }
#else
uint8_t rbootMappedBank() { return 0; }
#endif
