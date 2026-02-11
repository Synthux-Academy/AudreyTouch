#pragma once

#include "daisy_seed.h"

#ifdef __cplusplus

using namespace daisy;

namespace synthux {
class Switches {
public:
    Switches() = default;

    ~Switches() = default;

    void Init();

    int A();

    int B();

private:
    Switch3 switch_7_8_;
    Switch3 switch_9_10_;
};
};

#endif
