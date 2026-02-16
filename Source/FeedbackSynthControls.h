#pragma once
#ifndef INFS_FEEDBACKSYNTHCONTROLS_H
#define INFS_FEEDBACKSYNTHCONTROLS_H

#include <daisy_seed.h>
#include "ControlValue.h"
#include "FeedbackSynthEngine.h"
#include "simpletouch/touch.h"

using namespace synthux;
using namespace simpletouch;

namespace infrasonic {
namespace FeedbackSynth {
class Controls {
public:
    explicit Controls(Engine &engine, Touch &touch) : engine_(engine),
                                                      touch_(touch) {
    }

    ~Controls() = default;

    void Init(DaisySeed &hw);

    void UpdateAudioRate(DaisySeed &hw);

    void UpdateSlowRate(DaisySeed &hw);

private:
    Engine &engine_;
    Touch &touch_;

    float current_note_base_;
    float octave_shift_;
    bool drone_mode_;

    ControlValue input_volume_cv_;
    ControlValue output_volume_cv_;
    ControlValue envelope_shape_cv_;
    ControlValue feedback_body_knob_cv_;
    ControlValue feedback_body_final_cv_;

    int scale_;
    int range_;

    Oscillator body_lfo_;

    AnalogControl &FrequencyFader() const { return touch_.knobs().s36(); }
    AnalogControl &FeedbackGainKnob() const { return touch_.knobs().s30(); }
    AnalogControl &VolumeKnob() const { return touch_.knobs().s31(); }
    AnalogControl &ReverbMixKnob() const { return touch_.knobs().s32(); }
    AnalogControl &ReverbSizeKnob() const { return touch_.knobs().s33(); }
    AnalogControl &LPFKnob() const { return touch_.knobs().s34(); }
    AnalogControl &HPFKnob() const { return touch_.knobs().s35(); }
    AnalogControl &EnvelopeBodyFader() const { return touch_.knobs().s37(); }

    int RangeSwitch() const { return touch_.switches().s7s8(); }
    int LfoSwitch() const { return touch_.switches().s9s10(); }
};
}
}

#endif
