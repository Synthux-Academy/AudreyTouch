#include <daisy_seed.h>

#include "Source/SimpleTouch.h"

using namespace daisy;
using namespace synthux::simpletouch;

static constexpr auto kSampleRate = SaiHandle::Config::SampleRate::SAI_48KHZ;
static constexpr size_t kBlockSize = 4;

/** Global Hardware access */
static DaisySeed hw;

static SimpleTouch controls;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {
    controls.Process();
}

int main() {
    hw.Init();
    hw.SetAudioSampleRate(kSampleRate);
    hw.SetAudioBlockSize(kBlockSize);

    controls.Init(hw);

    hw.StartLog(true);

    hw.StartAudio(AudioCallback);

    while (true) {
        for (size_t i = 0; i < SimpleTouch::Ctrl::kCtrlLast; i++) {
            hw.Print(FLT_FMT(5) ",", FLT_VAR(5, controls.controls[i].Value()));
        }
        hw.PrintLine("");
        System::Delay(10);
    }
}
