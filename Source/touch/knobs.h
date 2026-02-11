#pragma once

#include "daisy_seed.h"
#include <array>

#ifdef __cplusplus

using namespace daisy;

namespace synthux {
class Knobs {
public:
    static constexpr int kKnobCount = 8;

    Knobs() = default;

    ~Knobs() = default;

    void Init(DaisySeed &hw);

    AnalogControl &s30() { return knobs_[0]; };
    AnalogControl &s31() { return knobs_[1]; };
    AnalogControl &s32() { return knobs_[2]; };
    AnalogControl &s33() { return knobs_[3]; };
    AnalogControl &s34() { return knobs_[4]; };
    AnalogControl &s35() { return knobs_[5]; };
    AnalogControl &s36() { return knobs_[6]; };
    AnalogControl &s37() { return knobs_[7]; };

private:
    std::array<AnalogControl, kKnobCount> knobs_;
};
};

#endif
