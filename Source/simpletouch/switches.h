#pragma once
#ifndef SYNTHUX_SIMPLETOUCH_SWITCHES_H
#define SYNTHUX_SIMPLETOUCH_SWITCHES_H

#include "daisy_seed.h"

#ifdef __cplusplus

using namespace daisy;

namespace synthux {
namespace simpletouch {
class Switches {
public:
    Switches() = default;

    ~Switches() = default;

    void Init();

    int s7s8();

    int s9s10();

private:
    Switch3 switch_7_8_;
    Switch3 switch_9_10_;
};
}
}

#endif
#endif
