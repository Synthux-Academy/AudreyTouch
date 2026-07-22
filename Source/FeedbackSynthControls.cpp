#include "FeedbackSynthControls.h"

#include <daisysp.h>
#include <functional>
#include <bitset>
#include <algorithm>

using namespace infrasonic::FeedbackSynth;
using namespace daisy;

static constexpr daisy::Pin kScaleASwitchPin            = daisy::seed::D8; // Simple bottom pin 9
static constexpr daisy::Pin kScaleBSwitchPin            = daisy::seed::D9; // Simple bottom pin 10
static constexpr daisy::Pin kLfoSwitchAPin              = daisy::seed::D6; // Simple bottom pin 7
static constexpr daisy::Pin kLfoSwitchBPin              = daisy::seed::D7; // Simple bottom pin 8

void Controls::Init(DaisySeed &hw, Engine &engine) {

    engine_ = &engine;

    params_.Init(hw.AudioSampleRate() / hw.AudioBlockSize());

    scale_switch_a_.Init(kScaleASwitchPin, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
    scale_switch_b_.Init(kScaleBSwitchPin, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);

    lfo_switch_a_.Init(kLfoSwitchAPin, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
    lfo_switch_b_.Init(kLfoSwitchBPin, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
    
    #if DEBUG
    hw.StartLog();
    #endif

    touch_.Init(hw);
    registerParams(engine);

    osc_.Init(48000.0f);
    osc_.SetAmp(1.f);
    osc_.SetWaveform(daisysp::Oscillator::WAVE_RAMP);
    osc_.SetFreq(1.0f);

    #ifdef USB_MIDI
    daisy::MidiUsbHandler::Config midi_cfg;
    midi_.Init(midi_cfg);
    #endif

    _pot_monitor.Init(_ui_queue, touch_, 500, 0.005f, 0.002f);
}

void Controls::Process(DaisySeed &hw) { //pads are updated at a slower rate
    #ifdef USB_MIDI
    processMIDI();
    #endif
    processTouch(hw);
    processUIQueue();
    params_.Process();
}

enum class BodyValueMode: uint8_t {
    None        = 0,
    FastLFO,
    Direct,
    SlowLFO
};
float Controls::bodyValue(const float param)
{
    std::bitset<2> lfo_sw;
    lfo_sw.set(0, lfo_switch_a_.Read());
    lfo_sw.set(1, lfo_switch_b_.Read());
    auto mode = static_cast<BodyValueMode>(lfo_sw.to_ulong());

    
    auto lfo_slew_rate = .08f;
    switch (mode) {
        case BodyValueMode::FastLFO: {
            osc_.SetFreq(1.f + ((1.f - param) * 7.f));
            break;
        }
        case BodyValueMode::SlowLFO: {
            osc_.SetFreq(.01f + ((1.0f - param) * .5f));
            lfo_slew_rate = .0001f; //lower is slower
            break;
        }
        default: return param;
    }

    auto curr_osc = osc_.Process();
    static auto prev_osc = 0.f, held_val = 0.f, smoothed_val = 0.f;
    if ((prev_osc < 0.f && curr_osc >= 0.f) || (prev_osc > 0.f && curr_osc <= 0.f)) {
        held_val = daisy::Random::GetFloat(param - (.05f + (.07f * (1.f - param))), param + (.05f + (.07f * (1.0f - param))));
    }
    smoothed_val += lfo_slew_rate * (held_val - smoothed_val);
    prev_osc = curr_osc;

    return std::clamp(smoothed_val, 0.f, 1.f);
}

static constexpr uint8_t kFirstNotePad  = 3;
static constexpr uint8_t kLastNotePad   = 9;
static constexpr uint8_t kScalesCount   = 3;
static constexpr uint8_t kScaleSize     = 7;
static constexpr uint8_t scales[kScalesCount][kScaleSize] = {
    { 0, 2, 4, 5, 9,  12, 14 },
    { 0, 5, 6, 9, 10, 12, 13 },
    { 0, 2, 3, 7, 9,  12, 14 }
};
void Controls::processTouch(DaisySeed& hw)
{
    touch_.Process();

    static auto zero_was_touched = false;
    static auto two_was_touched = false;
    auto zero_touched   = touch_.IsTouched(0);
    auto two_touched    = touch_.IsTouched(2);
    auto ch_touched     = touch_.IsTouched(11); 

    controlling_env_ = ch_touched && !drone_mode_;
    controlling_output_vol_ = touch_.IsTouched(10);

    if (ch_touched) {
        if (zero_touched && !zero_was_touched) {
            scale_idx_ = (scale_idx_ + 1) % kScalesCount;
        }
        if (two_touched && !two_was_touched) {
            drone_mode_ = !drone_mode_;
            engine_->DroneMode(drone_mode_);
        }
    }
    else {
        auto mult = 0;
        if (zero_touched && !zero_was_touched) mult = -1;
        else if (two_touched && !two_was_touched) mult = 1;
        octave_shift_ = std::clamp(octave_shift_ + 12 * mult, -12, 48);    
    }

    static auto was_note_touched = false;
    auto is_note_touched = false;
    for (auto pad = kFirstNotePad; pad <= kLastNotePad; ++pad) {
        is_note_touched = touch_.IsTouched(pad);
        if (is_note_touched) {
            note_base_ = 16 + scales[scale_idx_][pad - kFirstNotePad];
            applyFrequency();
            break;
        }
    }

    if (is_note_touched && !was_note_touched) engine_->NoteOn();
    else if (!is_note_touched && was_note_touched) engine_->NoteOff();
    
    hw.SetLed(is_note_touched || drone_mode_);

    was_note_touched = is_note_touched;
    zero_was_touched = zero_touched;
    two_was_touched = two_touched;
}

template <typename T>
inline T map(const T& x,
             const T& in_min,
             const T& in_max,
             const T& out_min,
             const T& out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
void Controls::processUIQueue()
{
    using P = synthux::simpletouch::Touch::Pot;
    _pot_monitor.Process();
    while(!_ui_queue.IsQueueEmpty()) {
        auto event = _ui_queue.GetAndRemoveNextEvent();
        if (event.type == UiEventQueue::Event::EventType::potMoved) {
            auto pot = static_cast<P>(event.asPotMoved.id);
            auto val = event.asPotMoved.newPosition;
            val = map(val, .02f, .95f, 0.f, 1.f);
            switch (pot) {
                case P::s30: {
                    params_.UpdateNormalized(Parameter::FeedbackGain, val);
                    break;
                }
                case P::s31: {
                    out_vol_.process(val, controlling_output_vol_);
                    if (out_vol_.apply()) params_.UpdateNormalized(Parameter::OutputVolume, out_vol_.value());

                    in_vol_.process(val, !controlling_output_vol_);
                    if (in_vol_.apply()) params_.UpdateNormalized(Parameter::InputVolume, in_vol_.value());
                    break;
                }
                case P::s32: {
                    params_.UpdateNormalized(Parameter::ReverbMix, val);
                    break;
                }
                case P::s33: {
                    params_.UpdateNormalized(Parameter::ReverbDecay, ftension(val, -3.0f));
                    
                    break;
                }
                case P::s34: {
                    params_.UpdateNormalized(Parameter::FeedbackLPFCutoff,  val);
                    break;
                }
                case P::s35: {
                    params_.UpdateNormalized(Parameter::FeedbackHPFCutoff,  val);
                    break;
                }
                case P::s36: {
                    freq_offset_norm_ = 24.f * val;
                    applyFrequency();
                    break;
                }
                case P::s37: {
                    env_.process(val, controlling_env_);
                    if (env_.apply()) params_.UpdateNormalized(Parameter::EnvelopeShape, env_.value());
                    body_.process(val, !controlling_env_);
                    if (body_.apply()) val = 1.0f - body_.value();
                    params_.UpdateNormalized(Parameter::FeedbackBody, bodyValue(val));
                    break;
                }
            }
        }
    }
}

void Controls::applyFrequency()
{
    auto note = std::clamp(
        note_base_ + freq_offset_norm_ + octave_shift_, 
        static_cast<float>(kMinNote), 
        static_cast<float>(kMaxNote));
    auto norm = (note - kMinNote) / (kMaxNote - kMinNote);
    params_.UpdateNormalized(Parameter::Frequency, norm);
}

void Controls::registerParams(Engine &engine) {
    using namespace std::placeholders;

    // String freq/pitch as note number
    params_.Register(Parameter::Frequency, 40.0f, 16.0f, 88.0f,
        std::bind(&Engine::SetStringPitch, &engine, _1), 0.2f);

    // Feedback Gain in dbFS
    params_.Register(Parameter::FeedbackGain, -60.0f, -60.0f, 12.0f,
        std::bind(&Engine::SetFeedbackGain, &engine, _1));

    // Feedback body/delay in seconds
    params_.Register(Parameter::FeedbackBody, 0.001f, 0.001f, 0.1f,
        std::bind(&Engine::SetFeedbackDelay, &engine, _1), 1.0f, daisysp::Mapping::EXP);

    // Feedback filter cutoffs in hz
    params_.Register(Parameter::FeedbackLPFCutoff, 18000.0f, 100.0f, 18000.0f,
        std::bind(&Engine::SetFeedbackLPFCutoff, &engine, _1), 0.05f, daisysp::Mapping::LOG);
    params_.Register(Parameter::FeedbackHPFCutoff, 250.0f, 10.0f, 4000.0f,
        std::bind(&Engine::SetFeedbackHPFCutoff, &engine, _1), 0.05f, daisysp::Mapping::LOG);

    // Reverb Mix
    params_.Register(Parameter::ReverbMix, 0.0f, 0.0f, 1.0f,
        std::bind(&Engine::SetReverbMix, &engine, _1));

    // Reverb Feedback (input is mapped to anti-exponential on ADC read)
    params_.Register(Parameter::ReverbDecay, 0.2f, 0.2f, 1.0f,
        std::bind(&Engine::SetReverbFeedback, &engine, _1));

    // Output level
    params_.Register(Parameter::OutputVolume, 0.5f, 0.0f, 1.0f,
        std::bind(&Engine::SetOutputLevel, &engine, _1), 0.05f, daisysp::Mapping::EXP);

    // Input level
    params_.Register(Parameter::InputVolume, 0.5f, 0.0f, 5.0f,
        std::bind(&Engine::SetInputLevel, &engine, _1), 0.05f, daisysp::Mapping::EXP);

    // Envelope shape
    params_.Register(Parameter::EnvelopeShape, 0.0f, 0.0f, 1.0f,
        std::bind(&Engine::SetShape, &engine, _1)); 
}

#ifdef USB_MIDI
float norm(const uint8_t value) {
    return std::clamp(static_cast<float>(value) / 127.f, 0.f, 1.f);
};
void Controls::processMIDI() {
    midi_.Listen();
    while(midi_.HasEvents()) {
        auto msg = midi_.PopEvent();
        switch(msg.type) {
            case NoteOn: {
                auto note_msg = msg.AsNoteOn();
                note_base_ = note_msg.note;
                engine_->NoteOn();
                applyFrequency();
                break;
            }
            case NoteOff: {
                engine_->NoteOff();
                break;
            }
            case ControlChange: {
                auto ctrl_msg = msg.AsControlChange();
                auto num = ctrl_msg.control_number;
                auto val = norm(ctrl_msg.value);
                using P = Parameter;
                auto p = P::None;
                     if (num == 7)  { p = P::OutputVolume; }
                else if (num == 12) { p = P::ReverbDecay; val = ftension(val, -3.0f); }
                else if (num == 70) { p = P::Frequency; }
                else if (num == 72) { p = P::EnvelopeShape; }
                else if (num == 74) { p = P::FeedbackLPFCutoff; }
                else if (num == 75) { p = P::FeedbackGain; }
                else if (num == 76) { p = P::FeedbackBody; }
                else if (num == 77) { p = P::InputVolume; }
                else if (num == 81) { p = P::FeedbackHPFCutoff; }
                else if (num == 91) { p = P::ReverbMix; }
                else if (num == 123) { engine_->NoteOff(); }

                if (p != Parameter::None) params_.UpdateNormalized(p, val);

                break;
            }
            default: break;
        }
    }
};
#endif
