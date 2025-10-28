#pragma once
#ifndef INFS_FEEDBACKSYNTHCONTROLS_H
#define INFS_FEEDBACKSYNTHCONTROLS_H

#include <daisy.h>
#include <daisy_seed.h>
#include "FeedbackSynthEngine.h"
#include "ParameterRegistry.h"
#include "simple-daisy-touch.h"

namespace infrasonic {
namespace FeedbackSynth {

class Controls {

public:

    Controls() = default;
    ~Controls() = default;

    void Init(daisy::DaisySeed &hw, Engine &engine);

    void UpdateAudioRate(daisy::DaisySeed &hw);
    void UpdateSlowRate(daisy::DaisySeed &hw);

    void UpdateFrequencyFromTouch();

    void Process() {
        params_.Process();
    }


private:

    Engine* engine_;

    static const size_t kNumAdcChannels = 11;

    // Touch sensor integration
    synthux::simpletouch::Touch touch_;


    /// Identifies a parameter of the synth engine
    /// The order here is the same order as the ADC pin configs in the cpp file
    enum class Parameter : uint8_t {
        Frequency           = 0,
        FeedbackGain,       // 1
        FeedbackBody,       // 2
        FeedbackLPFCutoff,  // 3
        FeedbackHPFCutoff,  // 4
        ReverbMix,          // 5
        ReverbDecay,        // 6
        EchoDelaySend,      // 7
        EchoDelayTime,      // 8
        EchoDelayFeedback,  // 9
        OutputVolume,       // 10
        InputVolume,        // 11
        EnvelopeShape,
        WetLevel
    };

    using Parameters = ParameterRegistry<Parameter>;

    Parameters params_;
    daisy::Switch del_sw_;
    GPIO scale_switch_a;
    GPIO scale_switch_b;
    GPIO lfo_switch_a;
    GPIO lfo_switch_b;
    daisysp::Oscillator _osc;
    

    void initADCs(daisy::DaisySeed &hw);
    void registerParams(Engine &engine);

    int scale = 0;
    bool drone_mode = false;
    float prev_val_env = 0.0f;
    float prev_val_body = 0.001f;
    bool controlling_env = false;
    float prev_val_input = 0.5f;
    float prev_val_output = 0.5f;
    int control_volmode = 0; // 0 = none, 1 = input, 2 = output
    float control_volumes[3] = {0.0f, 0.0f, 0.0f};
    float freq_shift = 0.0f;
    int range = 0;
    float octave_shift = 0.0f;
    bool octave_down_was = true;
    bool octave_up_was = true;
    bool octave_down_pad = false;
    bool octave_up_pad = false;
    float body_knob_val = 0.0f;
    float body_val = 0.0f;
    float lfo_depth = 0.0f;
    float note = 35.0f;
    float current_note_base = 40.0f;
    float min_note = 16.0f;
    float max_note = 88.0f;
    float body_knob = 0.0f;
    float volume_knob = 0.0f;

    bool env_knob_catched = false;
    bool vol_knob_catched = false;
    float env_target_val = 0.0f;
    float vol_target_val = 0.0f;

};

}
}

#endif
