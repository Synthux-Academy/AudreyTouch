# Project Name
TARGET = AudreyTouch
USE_DAISYSP_LGPL = 1


LIBDAISY_DIR = ../lib/libDaisy
DAISYSP_DIR = ../lib/DaisySP
CMSIS_DSP_SRC_DIR = ${LIBDAISY_DIR}/Drivers/CMSIS-DSP/Source


C_INCLUDES = -ISource/
C_DEFS = -DTARGET_DAISY


CPP_SOURCES = \
	FeedbackSynth_main.cpp \
	Source/BiquadFilters.cpp \
	Source/FeedbackSynthControls.cpp \
	Source/FeedbackSynthEngine.cpp \
	Source/KarplusString.cpp \
	Source/memory/sdram_alloc.cpp

# Core location
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

# Target per compilare le librerie se serve
libs:
	cd $(LIBDAISY_DIR) && $(MAKE)
	cd $(DAISYSP_DIR) && $(MAKE)

clean-libs:
	cd $(LIBDAISY_DIR) && $(MAKE) clean
	cd $(DAISYSP_DIR) && $(MAKE) clean

.PHONY: copy-bin
copy-bin: build/$(TARGET).elf
	cp build/$(TARGET).bin $(TARGET).bin