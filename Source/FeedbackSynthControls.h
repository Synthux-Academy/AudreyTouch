#pragma once
#ifndef INFS_FEEDBACKSYNTHCONTROLS_H
#define INFS_FEEDBACKSYNTHCONTROLS_H

#include <daisy_seed.h>
#include "ControlValue.h"
#include "FeedbackSynthEngine.h"
#include "touch/touch.h"

using namespace synthux;

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

    float
            current_note_base_,
            octave_shift_;

    ControlValue
            input_volume_,
            output_volume_,
            envelope_shape_,
            feedback_body_knob_,
            feedback_body_final_;

    bool drone_mode_,
            prev_note_touched_;

    int scale_,
            range_;

    Oscillator lfo_;
};
}
}

#endif
