#pragma once
#ifndef SYNTHUX_SIMPLETOUCH_TOUCH_H
#define SYNTHUX_SIMPLETOUCH_TOUCH_H

#include "daisy_seed.h"
#include "knobs.h"
#include "pads.h"
#include "switches.h"

#ifdef __cplusplus

using namespace daisy;

namespace synthux {
namespace simpletouch {
class Touch {
public:
    Touch() = default;

    ~Touch() = default;

    void Init(DaisySeed &hw) {
        pads_.Init();
        knobs_.Init(hw);
        switches_.Init();

        hw.adc.Start();
    }

    Pads &pads() { return pads_; }

    Knobs &knobs() { return knobs_; }

    Switches &switches() { return switches_; }

private:
    Knobs knobs_;
    Pads pads_;
    Switches switches_;
};
}
}

#endif
#endif
