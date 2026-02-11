#include "FeedbackSynthControls.h"
#include "daisysp.h"

using namespace infrasonic;
using namespace infrasonic::FeedbackSynth;
using namespace synthux;
using namespace daisy;

void Controls::Init(DaisySeed &hw) {
    input_volume_.Init(hw, 0.5f, 0.02f);
    output_volume_.Init(hw, 0.5f, 0.02f);
    output_volume_.Detach();

    envelope_shape_.Init(hw, 0.f, 0.02f);
    envelope_shape_.Detach();
    feedback_body_knob_.Init(hw, 0.f, 1.f);

    lfo_.Init(48000.0f);
    lfo_.SetAmp(1.f);
    lfo_.SetWaveform(Oscillator::WAVE_RAMP);
    lfo_.SetFreq(1.0f);
}

void Controls::UpdateAudioRate(DaisySeed &hw) {
    // Feedback Gain in dbFS
    engine_.SetFeedbackGain(fmap(touch_.knobs().s30().Process(), -60.0f, 12.0f));

    // Reverb Mix
    engine_.SetReverbMix(fmap(touch_.knobs().s32().Process(), 0.0f, 1.0f));

    // Reverb Feedback
    engine_.SetReverbFeedback(fmap(ftension(touch_.knobs().s33().Process(), -3.0f), 0.2f, 1.0f));

    // Feedback filter cutoffs in hz
    engine_.SetFeedbackLPFCutoff(fmap(touch_.knobs().s34().Process(), 100.0f, 18000.0f, Mapping::LOG));
    engine_.SetFeedbackHPFCutoff(fmap(touch_.knobs().s35().Process(), 10.0f, 4000.0f, Mapping::LOG));

    float raw_volume_knob = touch_.knobs().s31().GetRawFloat();
    engine_.SetInputLevel(fmap(input_volume_.Process(raw_volume_knob), 0.0f, 1.0f, Mapping::EXP));
    engine_.SetOutputLevel(fmap(output_volume_.Process(raw_volume_knob), 0.0f, 1.0f, Mapping::EXP));

    float raw_body_envelope_knob = touch_.knobs().s37().GetRawFloat();
    engine_.SetShape(envelope_shape_.Process(raw_body_envelope_knob));
    float body_knob_val = 1 - feedback_body_knob_.Process(raw_body_envelope_knob);

    float body_val;
    if (touch_.switches().B() == Switch3::POS_LEFT) {
        body_val = body_knob_val;
    } else {
        static float prev_osc = 0.0f;
        static float held_val = 0.0f;
        static float smoothed_val = 0.0f;

        float slew_rate;
        if (touch_.switches().B() == Switch3::POS_CENTER) {
            lfo_.SetFreq(0.01f + ((1.0f - body_knob_val) * 0.5f));
            slew_rate = 0.0001f; // lower is slower
        } else {
            // POS_RIGHT
            lfo_.SetFreq(1.0f + ((1.0f - body_knob_val) * 7.0f));
            slew_rate = 0.08f; // lower is slower
        }

        float curr_osc = lfo_.Process();
        if ((prev_osc < 0.0f && curr_osc >= 0.0f) || (prev_osc > 0.0f && curr_osc <= 0.0f)) {
            held_val = Random::GetFloat(body_knob_val - (0.05f + (0.07f * (1.0f - body_knob_val))),
                                        body_knob_val + (0.05f + (0.07f * (1.0f - body_knob_val))));
        }

        smoothed_val += slew_rate * (held_val - smoothed_val);
        body_val = smoothed_val;
        prev_osc = curr_osc;
    }

    feedback_body_final_.Process(fclamp(body_val, 0.0f, 1.0f));
    engine_.SetFeedbackDelay(fmap(feedback_body_final_.Value(), 0.001f, 0.1f, Mapping::EXP));

    float freq_shift = touch_.knobs().s36().Process() * 24.0f;
    engine_.SetStringPitch(fclamp(current_note_base_ + freq_shift + octave_shift_, 16.0f, 88.0f));
}

void Controls::UpdateSlowRate(DaisySeed &hw) {
    if (touch_.switches().A() == Switch3::POS_LEFT) {
        range_ = 0;
    } else if (touch_.switches().A() == Switch3::POS_CENTER) {
        range_ = 1;
    } else {
        // POS_RIGHT
        range_ = 2;
    }

    touch_.pads().Process();

    if (touch_.pads().IsRisingEdge(11)) {
        feedback_body_knob_.Detach();
        envelope_shape_.Attach();
    }

    if (touch_.pads().IsFallingEdge(11)) {
        envelope_shape_.Detach();
        feedback_body_knob_.Attach();
    }

    if (touch_.pads().IsRisingEdge(10)) {
        input_volume_.Detach();
        output_volume_.Attach();
    }

    if (touch_.pads().IsFallingEdge(10)) {
        output_volume_.Detach();
        input_volume_.Attach();
    }

    if (touch_.pads().IsTouched(11)) {
        if (touch_.pads().IsRisingEdge(0)) {
            scale_ = (scale_ + 1) % 3;
        }

        if (touch_.pads().IsRisingEdge(2)) {
            drone_mode_ = !drone_mode_;
            engine_.DroneMode(drone_mode_);
        }
    } else {
        if (touch_.pads().IsRisingEdge(0)) {
            octave_shift_ -= 12.0f;
            if (octave_shift_ < -12.0f) octave_shift_ = -12.0f;
        }

        if (touch_.pads().IsRisingEdge(2)) {
            octave_shift_ += 12.0f;
            if (octave_shift_ > 48.0f) octave_shift_ = 48.0f;
        }
    }

    static const int scales[][8] = {
        {0, 2, 4, 5, 9, 12, 14},
        {0, 5, 6, 9, 10, 12, 13},
        {0, 2, 3, 7, 9, 12, 14}
    };

    bool note_touched = false;
    for (int pad = 3; pad <= 9; ++pad) {
        if (touch_.pads().IsRisingEdge(pad)) {
            int note_index = pad - 3;
            float base_note = 16.0f;

            current_note_base_ = base_note + scales[scale_][note_index];
            note_touched = true;
            break;
        }
    }

    if (note_touched && !prev_note_touched_) {
        engine_.NoteOn();
    } else if (!note_touched && prev_note_touched_) {
        engine_.NoteOff();
    }

    prev_note_touched_ = note_touched;

    hw.SetLed(drone_mode_ || note_touched);
}
