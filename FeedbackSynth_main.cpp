#include <daisy_seed.h>
#include "FeedbackSynthEngine.h"
#include "FeedbackSynthControls.h"
#include "Source/FeedbackSynthControls.h"
#include "Source/simpletouch/touch.h"

using namespace infrasonic;
using namespace daisy;
using namespace daisysp;
using namespace synthux;
using namespace simpletouch;

static const auto kSampleRate = SaiHandle::Config::SampleRate::SAI_48KHZ;
static const size_t kBlockSize = 4;

static DaisySeed hw;
static Touch touch;
static FeedbackSynth::Engine engine;
static FeedbackSynth::Controls controls(engine, touch);
static Limiter limiter[2];

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {
    controls.UpdateAudioRate(hw);
    for (size_t i = 0; i < size; i++) {
        engine.Process(IN_L[i], OUT_L[i], OUT_R[i]);
    }
    limiter[0].ProcessBlock(OUT_L, size, 0.7f);
    limiter[1].ProcessBlock(OUT_R, size, 0.7f);
}

int main() {
    hw.Init();
    hw.SetAudioSampleRate(kSampleRate);
    hw.SetAudioBlockSize(kBlockSize);

    touch.Init(hw);
    engine.Init(hw.AudioSampleRate());
    controls.Init(hw);

    for (auto &lim: limiter) {
        lim.Init();
    }

    hw.StartAudio(AudioCallback);

    while (true) {
        controls.UpdateSlowRate(hw);
        hw.DelayMs(4);
    }
}
