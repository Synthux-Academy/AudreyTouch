#include "FeedbackSynthControls.h"
#include <functional>
#include <algorithm>
#include "simple-daisy-touch.h"
#include "daisysp.h"



using namespace infrasonic;
using namespace infrasonic::FeedbackSynth;
using namespace daisy;

////////////// SIMPLE X DAISY PINOUT CHEATSHEET ///////////////

// 3v3           29  |       |   20    AGND
// D15 / A0      30  |       |   19    OUT 01
// D16 / A1      31  |       |   18    OUT 00
// D17 / A2      32  |       |   17    IN 01
// D18 / A3      33  |       |   16    IN 00
// D19 / A4      34  |       |   15    D14
// D20 / A5      35  |       |   14    D13
// D21 / A6      36  |       |   13    D12
// D22 / A7      37  |       |   12    D11
// D23 / A8      38  |       |   11    D10
// D24 / A9      39  |       |   10    D9
// D25 / A10     40  |       |   09    D8
// D26           41  |       |   08    D7
// D27           42  |       |   07    D6
// D28 / A11     43  |       |   06    D5
// D29           44  |       |   05    D4
// D30           45  |       |   04    D3
// 3v3 Digital   46  |       |   03    D2
// VIN           47  |       |   02    D1
// DGND          48  |       |   01    D0

// TODO: Add footprint numbers to these

static constexpr daisy::Pin kInputVolumeAdcPin          = daisy::seed::A1;  // Simple bottom pin 31
static constexpr daisy::Pin kFreqKnobAdcPin             = daisy::seed::A6; // Simple bottom pin 36
static constexpr daisy::Pin kFeedbackGainKnobPin        = daisy::seed::A0;  // Simple bottom pin 30
static constexpr daisy::Pin kFeedbackBodyKnobPin        = daisy::seed::A7;  // Simple bottom pin 37
static constexpr daisy::Pin kFeedbackLowpassKnobAdcPin  = daisy::seed::A4;  // Simple bottom pin 34
static constexpr daisy::Pin kFeedbackHighpassKnobAdcPin = daisy::seed::A5;  // Simple bottom pin 35
static constexpr daisy::Pin kRevMixKnobAdcPin           = daisy::seed::A2;  // Simple bottom pin 32
static constexpr daisy::Pin kRevDecayKnobAdcPin         = daisy::seed::A3;  // Simple bottom pin 33

//Delay controls are not implemented
static constexpr daisy::Pin kEchoSendKnobAdcPin         = daisy::seed::A8;  // Simple bottom pin 38
static constexpr daisy::Pin kEchoTimeKnobAdcPin         = daisy::seed::A9;  // Simple bottom pin 39
static constexpr daisy::Pin kEchoFeedbackKnobAdcPin     = daisy::seed::A10;  // Simple bottom pin 40
static constexpr daisy::Pin kDelaySwitchPin             = daisy::seed::D14; // Simple bottom pin 15

static constexpr daisy::Pin kScaleASwitchPin            = daisy::seed::D8; // Simple bottom pin 9
static constexpr daisy::Pin kScaleBSwitchPin            = daisy::seed::D9; // Simple bottom pin 10
static constexpr daisy::Pin kLfoSwitchAPin              = daisy::seed::D6; // Simple bottom pin 7
static constexpr daisy::Pin kLfoSwitchBPin              = daisy::seed::D7; // Simple bottom pin 8

void Controls::Init(DaisySeed &hw, Engine &engine) {

    engine_ = &engine;

    params_.Init(hw.AudioSampleRate() / hw.AudioBlockSize());

    //delay switch not implemented
    /*del_sw_.Init(
        static_cast<dsy_gpio_pin>(kDelaySwitchPin),
        1000.0f,
        Switch::TYPE_TOGGLE,
        Switch::POLARITY_INVERTED,
        Switch::PULL_UP
    );*/

    scale_switch_a.Init(kScaleASwitchPin, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
    scale_switch_b.Init(kScaleBSwitchPin, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);

    lfo_switch_a.Init(kLfoSwitchAPin, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
    lfo_switch_b.Init(kLfoSwitchBPin, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);


    initADCs(hw);
    registerParams(engine);

    touch_.Init(hw);

    _osc.Init(48000.0f);
    _osc.SetAmp(1.f);
    _osc.SetWaveform(daisysp::Oscillator::WAVE_RAMP);
    _osc.SetFreq(1.0f);

}

void Controls::UpdateAudioRate(DaisySeed &hw) { //pots are updated at audio rate

    float freq_knob = hw.adc.GetFloat(0);
    /*float FeedbackGain = hw.adc.GetFloat(1);
    if (range == 0) {FeedbackGain = FeedbackGain * 0.44f;}
    else if (range == 1) {FeedbackGain = (FeedbackGain * 0.50f)+0.16f;}
    else if( range == 2) {FeedbackGain = (FeedbackGain * 0.7f)+0.3f;}
    params_.UpdateNormalized(Parameter::FeedbackGain,       FeedbackGain);*/
    params_.UpdateNormalized(Parameter::FeedbackGain,       hw.adc.GetFloat(1));
    body_knob = hw.adc.GetFloat(2);
    params_.UpdateNormalized(Parameter::FeedbackLPFCutoff,  hw.adc.GetFloat(3));
    params_.UpdateNormalized(Parameter::FeedbackHPFCutoff,  hw.adc.GetFloat(4));
    params_.UpdateNormalized(Parameter::ReverbMix,          hw.adc.GetFloat(5));
    params_.UpdateNormalized(Parameter::ReverbDecay,        ftension(hw.adc.GetFloat(6), -3.0f));
    volume_knob = hw.adc.GetFloat(10);

    //delay is 
    //del_sw_.Debounce();
    //float delay_scale = del_sw_.Pressed() ? 0.5f : 1.0f;
    params_.UpdateNormalized(Parameter::EchoDelaySend, 0.0f);
    params_.UpdateNormalized(Parameter::EchoDelayTime, 0.0f);
    params_.UpdateNormalized(Parameter::EchoDelayFeedback, 0.0f);


    if (controlling_env) {
        if(!env_knob_catched) {
            if (fabsf(body_knob - env_target_val) < 0.02)
                env_knob_catched = true;
        }

        if (env_knob_catched && fabsf(body_knob - prev_val_env) > 0.01)
        {
            params_.UpdateNormalized(Parameter::EnvelopeShape, body_knob);
            prev_val_env = body_knob;
        }
    }
    else {
        if(!env_knob_catched) {
            if (fabsf(body_knob - env_target_val) < 0.02)
                env_knob_catched = true;
        }

        if (env_knob_catched && fabsf(body_knob - prev_val_body) > 0.01)
        {
            body_knob_val = 1.0f - body_knob;
            prev_val_body = body_knob;
        }
    }
        

    if (!lfo_switch_a.Read() && lfo_switch_b.Read()) {body_val = body_knob_val;}
    //else if (lfo_switch_a.Read() && lfo_switch_b.Read()) {body_val = body_knob_val + (_osc.Process() * (0.05 + (0.07f * (1.0f - body_knob_val))));}
    else if ((lfo_switch_a.Read() && !lfo_switch_b.Read()) || (lfo_switch_a.Read() && lfo_switch_b.Read())) {
        static float prev_osc = 0.0f;
        static float held_val = 0.0f;
        static float smoothed_val = 0.0f;

        if (lfo_switch_a.Read() && lfo_switch_b.Read()){
            _osc.SetFreq(0.01f + ((1.0f - body_knob_val)*0.5f));
            slewRate = 0.0001f; //lower is slower
        } else if (lfo_switch_a.Read() && !lfo_switch_b.Read()) {
            _osc.SetFreq(1.0f + ((1.0f - body_knob_val) * 7.0f));
            slewRate = 0.08f; //lower is slower
        }

        float curr_osc = _osc.Process();
        if ((prev_osc < 0.0f && curr_osc >= 0.0f) || (prev_osc > 0.0f && curr_osc <= 0.0f)) {

            held_val = daisy::Random::GetFloat(body_knob_val - (0.05f + (0.07f * (1.0f - body_knob_val))), body_knob_val + (0.05f + (0.07f * (1.0f - body_knob_val))));
        }

        smoothed_val += slewRate * (held_val - smoothed_val);

        body_val = smoothed_val;

        prev_osc = curr_osc;
    }

    if(body_val < 0.0f) body_val = 0.0f;
    if(body_val > 1.0f) body_val = 1.0f;

    params_.UpdateNormalized(Parameter::FeedbackBody, body_val);
    
    if (controlling_OutputVol) {
        if(!vol_knob_catched) {
            if (fabsf(volume_knob - vol_target_val) < 0.02)
                vol_knob_catched = true;
        }

        if (vol_knob_catched && fabsf(volume_knob - prev_val_output) > 0.01)
        {
            params_.UpdateNormalized(Parameter::OutputVolume, volume_knob);
            prev_val_output = volume_knob;
        }
        }
    else {
        if(!vol_knob_catched) {
            if (fabsf(volume_knob - vol_target_val) < 0.02)
                vol_knob_catched = true;
        }

        if (vol_knob_catched && fabsf(volume_knob - prev_val_input) > 0.01)
        {
            params_.UpdateNormalized(Parameter::InputVolume, volume_knob);
            prev_val_input = volume_knob;
        }
    }
            

    freq_shift = freq_knob * 24.0f;
    note = current_note_base + freq_shift + octave_shift;

    if (note < min_note) note = min_note;
    if (note > max_note) note = max_note;

    float norm = (note - min_note) / (max_note - min_note);
    params_.UpdateNormalized(Parameter::Frequency, norm);
}

void Controls::UpdateSlowRate(DaisySeed &hw) { //pads are updated at a slower rate
    static bool pad_was_touched = false;
    bool any_pad_touched = false;

    if (!scale_switch_a.Read() && scale_switch_b.Read()) {range = 0;}
    else if (scale_switch_a.Read() && scale_switch_b.Read()) {range = 1;}
    else if (scale_switch_a.Read() && !scale_switch_b.Read()) {range = 2;}


    touch_.Process();

    if (touch_.IsTouched(11) && !drone_mode) {
        if(!controlling_env){
            env_knob_catched = false;
            env_target_val = prev_val_env;
        }
        controlling_env = true;
    } else {
        if(controlling_env){
            env_knob_catched = false;
            env_target_val = prev_val_body;
        }
        controlling_env = false;
    }

    if (touch_.IsTouched(10)) {
        if(!controlling_OutputVol){
            vol_knob_catched = false;
            vol_target_val = prev_val_output;
        }
        controlling_OutputVol = true;
    } else {
        if(controlling_OutputVol){
            vol_knob_catched = false;
            vol_target_val = prev_val_input;
        }
        controlling_OutputVol = false;
    }

    static bool drone_toggle_pressed = false;

    bool drone_pad_combo = touch_.IsTouched(11) && touch_.IsTouched(2);
    if(drone_pad_combo && !drone_toggle_pressed) {
        drone_mode = !drone_mode;
        engine_->DroneMode(drone_mode);
        hw.SetLed(drone_mode);
    }
    drone_toggle_pressed = drone_pad_combo;

    bool octave_down_pad = touch_.IsTouched(0);
    bool octave_up_pad   = touch_.IsTouched(2);

    if (octave_down_pad && !octave_down_was && !touch_.IsTouched(11)) {
        octave_shift -= 12.0f;
        if (octave_shift < -12.0f) octave_shift = -12.0f;
    }

    if (octave_up_pad && !octave_up_was && !touch_.IsTouched(11)) {
        octave_shift += 12.0f;
        if (octave_shift > 48.0f) octave_shift = 48.0f;
    }

    octave_down_was = octave_down_pad;
    octave_up_was   = octave_up_pad;

    octave_down_was = octave_down_pad;
    octave_up_was = octave_up_pad;
    
    static bool scale_pad_pressed = false;

    bool scale_pad = touch_.IsTouched(0);
    if(scale_pad && !scale_pad_pressed && touch_.IsTouched(11)) {
        scale = (scale + 1) % 3;
    }
    scale_pad_pressed = scale_pad;

    static const int scaleA[] = {0, 2, 4, 5, 9, 12, 14};
    static const int scaleB[] = {0, 5, 6, 9, 10, 12, 13};
    static const int scaleC[] = {0, 2, 3, 7, 9, 12, 14};

    for (int pad = 3; pad <= 9; ++pad) {
        if (touch_.IsTouched(pad)) {
            int note_index = pad - 3;
            float base_note = 16.0f;

            if(scale == 0){current_note_base = base_note + scaleA[note_index];}
            else if(scale == 1){current_note_base = base_note + scaleB[note_index];}
            else if(scale == 2){current_note_base = base_note + scaleC[note_index];}
            any_pad_touched = true;
            break;
        }
    }

    if (any_pad_touched && !pad_was_touched) {
        engine_->NoteOn();
        if(!drone_mode){hw.SetLed(true);}
    } else if (!any_pad_touched && pad_was_touched) {
        engine_->NoteOff();
        if(!drone_mode){hw.SetLed(false);}
    }
    
    pad_was_touched = any_pad_touched;

}

void Controls::initADCs(DaisySeed &hw) {
    AdcChannelConfig config[kNumAdcChannels];

    config[0].InitSingle(kFreqKnobAdcPin);
    config[1].InitSingle(kFeedbackGainKnobPin);
    config[2].InitSingle(kFeedbackBodyKnobPin);
    config[3].InitSingle(kFeedbackLowpassKnobAdcPin);
    config[4].InitSingle(kFeedbackHighpassKnobAdcPin);
    config[5].InitSingle(kRevMixKnobAdcPin);
    config[6].InitSingle(kRevDecayKnobAdcPin);
    config[7].InitSingle(kEchoSendKnobAdcPin);
    config[8].InitSingle(kEchoTimeKnobAdcPin);
    config[9].InitSingle(kEchoFeedbackKnobAdcPin);
    config[10].InitSingle(kInputVolumeAdcPin);

    hw.adc.Init(config, kNumAdcChannels);
    hw.adc.Start();
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

    // Echo Delay send
    params_.Register(Parameter::EchoDelaySend, 0.0f, 0.0f, 1.0f,
        std::bind(&Engine::SetEchoDelaySendAmount, &engine, _1), 0.05f, daisysp::Mapping::EXP);

    // Echo Delay time in s
    params_.Register(Parameter::EchoDelayTime, 0.5f, 0.05f, 5.0f,
        std::bind(&Engine::SetEchoDelayTime, &engine, _1), 0.1f, daisysp::Mapping::EXP);

    // Echo Delay feedback
    params_.Register(Parameter::EchoDelayFeedback, 0.0f, 0.0f, 1.5f,
        std::bind(&Engine::SetEchoDelayFeedback, &engine, _1));

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