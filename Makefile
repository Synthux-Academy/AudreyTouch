# Project Name
TARGET = AudreyTouch
USE_DAISYSP_LGPL = 1

# Enabling DEBUG disables USB MIDI
# DEBUG = 1

LIBDAISY_DIR = lib/libDaisy
DAISYSP_DIR = lib/DaisySP
CMSIS_DSP_SRC_DIR = ${LIBDAISY_DIR}/Drivers/CMSIS-DSP/Source


C_INCLUDES = -ISource/
C_DEFS = -DTARGET_DAISY


CPP_SOURCES = \
    FeedbackSynth_main.cpp \
    $(wildcard Source/*.cpp) \
    $(wildcard Source/memory/*.cpp) \

CPP_STANDARD = -std=gnu++17

# Core location
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

libs:
	cd $(LIBDAISY_DIR) && $(MAKE)
	cd $(DAISYSP_DIR) && $(MAKE)

clean-libs:
	cd $(LIBDAISY_DIR) && $(MAKE) clean
	cd $(DAISYSP_DIR) && $(MAKE) clean