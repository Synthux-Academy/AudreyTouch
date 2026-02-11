#include "switches.h"

using namespace synthux;
using namespace daisy;
using namespace seed;

void Switches::Init() {
    switch_7_8_.Init(D7, D6);
    switch_9_10_.Init(D9, D8);
};

int Switches::A() {
    return switch_7_8_.Read();
};

int Switches::B() {
    return switch_9_10_.Read();
};
