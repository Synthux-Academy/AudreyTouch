#pragma once

#include "daisy.h"
#include "daisy_seed.h"
#include <array>

using namespace daisy;
using namespace seed;

using namespace daisy;

namespace synthux {
namespace simpletouch {

///////////////////////////////////////////////////////////////
//////////////////////// TOUCH SENSOR /////////////////////////
class Touch {

public:
  enum class Pot: uint8_t {
    s30 = 0,
    s31,
    s32,
    s33,
    s34,
    s35,
    s36,
    s37
  };
  // Implements PotMonitor Backend ////////////////
  /////////////////////////////////////////////////
  inline float GetPotValue(uint16_t pot_id)
  {
      return _hw.adc.GetFloat(pot_id);
  }

  Touch(): 
  _state      { 0 }, 
  _on_touch   { nullptr }, 
  _on_release { nullptr } 
  {}

  void Init(DaisySeed hw) {
    // Uncomment if you want to use i2C4
    // Wire.setSCL(D13);
    // Wire.setSDA(D14);
    _hw = hw;
    Mpr121I2C::Config config;

    int result = _cap.Init(config);

    if (result != 0) {
      _hw.PrintLine("MPR121 config failed");
    }

    initADCs(hw);
  }

#define P(a) std::underlying_type_t<Pot>(Pot::a)
void initADCs(DaisySeed &hw) {
    static const size_t kNumAdcChannels = 8;
    using namespace daisy::seed;
    
    Pin pins[kNumAdcChannels]; 
    pins[P(s30)] = A0;
    pins[P(s31)] = A1;
    pins[P(s32)] = A2;
    pins[P(s33)] = A3;
    pins[P(s34)] = A4;
    pins[P(s35)] = A5;
    pins[P(s36)] = A6;
    pins[P(s37)] = A7;

    AdcChannelConfig config[kNumAdcChannels];
    for (size_t i = 0; i < kNumAdcChannels; i++) {
      config[i].InitSingle(pins[i]);  
    }

    hw.adc.Init(config, kNumAdcChannels);
    hw.adc.Start();
}

  // Register note on callback
  void SetOnTouch(void (*on_touch)(uint16_t pad)) { _on_touch = on_touch; }

  void SetOnRelease(void (*on_release)(uint16_t pad)) {
    _on_release = on_release;
  }

  bool IsTouched(uint16_t pad) { return _state & (1 << pad); }

  bool HasTouch() { return _state > 0; }

  void Process() {
    uint16_t pad;
    bool is_touched;
    bool was_touched;
    auto state = _cap.Touched();
    for (uint16_t i = 0; i < 12; i++) {
      pad = 1 << i;
      is_touched = state & pad;
      was_touched = _state & pad;
      if (_on_touch != nullptr && is_touched && !was_touched) {
        _on_touch(i);
      } else if (_on_release != nullptr && was_touched && !is_touched) {
        _on_release(i);
      }
    }
    _state = state;
  }

  float GetAdcValue(int idx) { return _hw.adc.GetFloat(idx); }

  DaisySeed _hw;

private:
  uint16_t _state;
  void (*_on_touch)(uint16_t pad);
  void (*_on_release)(uint16_t pad);
  Mpr121I2C _cap;
  
};


}; // namespace simpletouch
}; // namespace synthux