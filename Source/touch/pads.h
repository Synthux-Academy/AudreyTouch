#pragma once

#include "daisy_seed.h"
#include "dev/mpr121.h"

#ifdef __cplusplus

using namespace daisy;

namespace synthux {
class Pads {
public:
    Pads() = default;

    ~Pads() = default;

    void Init() {
        mpr_.Init(Mpr121I2C::Config());
    }

    void Process() {
        prev_state_ = state_;
        state_ = mpr_.Touched();
    }

    bool IsTouched(uint16_t pad) const {
        return state_ & (1 << pad);
    };

    bool HasTouch() const {
        return state_ > 0;
    };

    bool IsRisingEdge(uint16_t pad) const {
        return state_ & ~prev_state_ & (1 << pad);
    }

    bool IsFallingEdge(uint16_t pad) const {
        return ~state_ & prev_state_ & (1 << pad);
    }

private:
    uint16_t state_, prev_state_;
    Mpr121I2C mpr_;
};
};

#endif
