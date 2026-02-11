#include "knobs.h"

using namespace synthux::simpletouch;
using namespace daisy;
using namespace seed;

void Knobs::Init(DaisySeed &hw) {
    AdcChannelConfig cfg[kKnobCount];
    cfg[0].InitSingle(A0);
    cfg[1].InitSingle(A1);
    cfg[2].InitSingle(A2);
    cfg[3].InitSingle(A3);
    cfg[4].InitSingle(A4);
    cfg[5].InitSingle(A5);
    cfg[6].InitSingle(A6);
    cfg[7].InitSingle(A7);
    hw.adc.Init(cfg, kKnobCount);

    for (auto i = 0; i < kKnobCount; i++) {
        knobs_[i].Init(hw.adc.GetPtr(i), hw.AudioCallbackRate());
    }
};
