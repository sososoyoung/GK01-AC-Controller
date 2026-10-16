#include "core/live_log.h"
#include "hw/hvac/hvac_registry.h"
#include "hw/hvac/wahin_encoder.h"
#include "hw/hvac/gree_encoder.h"
#include "hw/hvac/irac_encoder.h"
#include "core/constants.h"
#include <IRutils.h>

HvacRegistry hvacRegistry;

namespace {
WahinEncoder wahinEnc;
GreeEncoder greeEnc;
IracEncoder  iracEnc;
}

void HvacRegistry::begin(IrService& irRef) {
    ir_ = &irRef;
    count_ = 0;
    registerEncoder(&wahinEnc);
    registerEncoder(&greeEnc);
    appLog.println(F("[MODULE] hvac_registry real impl loaded"));
}

void HvacRegistry::registerEncoder(hvac::IEncoder* enc) {
    if (enc && count_ < MAX_ENCODERS) {
        encoders_[count_++] = enc;
    }
}

hvac::IEncoder* HvacRegistry::find(const String& vendor) const {
    for (size_t i = 0; i < count_; i++) {
        if (encoders_[i] && vendor == encoders_[i]->vendor()) {
            return encoders_[i];
        }
    }
    return nullptr;
}

bool HvacRegistry::isKnownVendor(const String& vendor) const {
    if (find(vendor)) return true;
    decode_type_t proto = strToDecodeType(vendor.c_str());
    return IRac::isProtocolSupported(proto);
}

bool HvacRegistry::isKnownModel(const String& vendor, int16_t model) const {
    if (!isKnownVendor(vendor)) return false;
    if (model == 0) return true;
    if (model < 1 || model > 32 || vendor == "WAHIN") return false;
    return irutils::modelToStr(strToDecodeType(vendor.c_str()), model) != "Unknown";
}

int HvacRegistry::minTempFor(const String& vendor) const {
    hvac::IEncoder* enc = find(vendor);
    if (enc) return enc->minTemp();
    return HVAC_MIN_TEMP_DEFAULT;
}

int HvacRegistry::maxTempFor(const String& vendor) const {
    hvac::IEncoder* enc = find(vendor);
    if (enc) return enc->maxTemp();
    return HVAC_MAX_TEMP_DEFAULT;
}

bool HvacRegistry::send(hvac::Command& cmd) {
    if (!isKnownModel(cmd.vendor, cmd.model)) return false;
    hvac::IEncoder* enc = cmd.model == 0 ? find(cmd.vendor) : nullptr;
    if (enc) return enc->send(cmd);

    appLog.printf("[HVAC] no dedicated encoder for %s, fallback IRac\n", cmd.vendor.c_str());
    return iracEnc.send(cmd);
}
