#include "switches.h"

using namespace synthux::simpletouch;
using namespace daisy;
using namespace seed;

void Switches::Init() {
    switch_7_8_.Init(D7, D6);
    switch_9_10_.Init(D9, D8);
};

int Switches::s7s8() {
    return switch_7_8_.Read();
};

int Switches::s9s10() {
    return switch_9_10_.Read();
};
