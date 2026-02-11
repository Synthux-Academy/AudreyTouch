#ifndef SYNTHUX_CONTROLVALUE_H
#define SYNTHUX_CONTROLVALUE_H

#include <daisy_seed.h>
#include <daisysp.h>

#include "DSPUtils.h"

using namespace daisysp;
using namespace daisy;

namespace synthux {
class ControlValue {
public:
    enum State {
        kStateAttached,
        kStateDetached,
        kStateTryToAttach,
    };

    ControlValue() = default;

    ~ControlValue() = default;

    void Init(const DaisySeed &hw, float initial, float slew_time, float threshold = 0.02f) {
        value_ = initial;
        state_ = kStateAttached;
        coef_ = infrasonic::onepole_coef_t60(slew_time, hw.AudioCallbackRate());
        threshold_ = threshold;
    }

    float Process(float input) {
        if (state_ == kStateTryToAttach) {
            if (fabs(input - value_) < threshold_) {
                state_ = kStateAttached;
            }
        }

        if (state_ == kStateAttached) {
            fonepole(value_, input, coef_);
        }

        return value_;
    }

    void Attach() {
        state_ = kStateTryToAttach;
    }

    void Detach() {
        state_ = kStateDetached;
    }

    float Value() const { return value_; }

private:
    State state_;
    float value_;
    float coef_;
    float threshold_;
};
}

#endif
