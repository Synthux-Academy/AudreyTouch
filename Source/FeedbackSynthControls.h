#pragma once

#if !DEBUG
#define USB_MIDI
#endif

#include <daisy_seed.h>

#include "FeedbackSynthEngine.h"
#include "ParameterRegistry.h"
#include "simple-daisy-touch.h"
#include "mvalue.h"

namespace infrasonic {
namespace FeedbackSynth {

class Controls {

public:

    Controls() = default;
    ~Controls() = default;

    void Init(daisy::DaisySeed &hw, Engine &engine);

    void Process(DaisySeed &hw);

private:

    Engine* engine_;

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
        count,
        None
    };

    using Parameters = ParameterRegistry<Parameter>;

    Parameters params_;
    GPIO scale_switch_a_;
    GPIO scale_switch_b_;
    GPIO lfo_switch_a_;
    GPIO lfo_switch_b_;
    daisysp::Oscillator osc_;

    void initADCs(daisy::DaisySeed &hw);
    void registerParams(Engine &engine);

    void processTouch(DaisySeed&);
    float bodyValue(const float param);

    daisy::UiEventQueue _ui_queue;
    daisy::PotMonitor<synthux::simpletouch::Touch, 8> _pot_monitor;
    void processUIQueue();

    #ifdef USB_MIDI
    daisy::MidiUsbHandler midi_;
    void processMIDI();
    #endif

    void applyFrequency();

    synthux::MValue env_;
    synthux::MValue body_;
    synthux::MValue in_vol_;
    synthux::MValue out_vol_;

    static constexpr uint8_t kMinNote = 16;
    static constexpr uint8_t kMaxNote = 88;

    float freq_offset_norm_;

    uint8_t scale_idx_      = 0;
    uint8_t note_base_      = 40;
    int8_t octave_shift_    = 0;

    bool octave_down_was_ = true;
    bool octave_up_was_ = true;
    bool drone_mode_ = false;
    bool controlling_env_ = false;
    bool controlling_output_vol_ = false;
};

}
}
