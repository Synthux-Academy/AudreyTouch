#include <daisy_seed.h>

#include "touch.h"

using namespace daisy;
using namespace synthux::simpletouch;

static constexpr auto kSampleRate = SaiHandle::Config::SampleRate::SAI_48KHZ;
static constexpr size_t kBlockSize = 4;

/** Global Hardware access */
static DaisySeed hw;

static Touch touch;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {
    for (auto &knob : touch.knobs().knobs()) {
        knob.Process();
    }
}

int main() {
    hw.Init();
    hw.SetAudioSampleRate(kSampleRate);
    hw.SetAudioBlockSize(kBlockSize);

    touch.Init(hw);

    hw.StartLog();

    hw.StartAudio(AudioCallback);

    while (true) {
        for (auto &knob : touch.knobs().knobs()) {
            hw.Print(FLT_FMT(5) " ", FLT_VAR(5, knob.Value()));
        }

        touch.pads().Process();
        hw.Print("%d ", touch.pads().HasTouch());

        hw.PrintLine("%d %d", touch.switches().s7s8(), touch.switches().s9s10());
        System::Delay(10);
    }
}
