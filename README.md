# 🪴 Audrey II Touch
Audrey II is a horrorscape synthesizer used by film composers, sound designers and composers. 

## 🪴 Audrey II Touch Faceplate
**Order the faceplate only:**  
[➡️ Audrey II Touch Faceplate](https://synthux.myshopify.com/products/audrey-ii-touch-faceplate-only?utm_source=copyToPasteBoard&utm_medium=product-links&utm_content=web)

**Or get the full Simple Touch package**, including all faceplates — *Audrey, Bass, String, FX, and Blank*:  
[🎛️ Order Simple Touch](https://www.synthux.academy/simple-synth/touch2)

---

📘 **Manual coming soon!**

![Audrey-Touch](https://github.com/user-attachments/assets/ead3868f-cd63-43c0-a0a0-303965cc6bd3)


# Installing Audrey II Firmware on Simple Touch
- Download the .bin file from this repository. 
- Hold down BOOT and then press RESET, then release both buttons. This will put the Daisy into BOOT MODE (you can tell you did it right if the top LED stops flashing).
- Upload the firmware via the [web flash tool](https://flash.daisy.audio/).

# Building the Audrey Touch firmware

## 1. Setup
- Follow the [Daisy Developer Setup Guide]((https://daisy.audio/tutorials/cpp-dev-env/#follow-along-with-the-video-guide)) to install the required toolchain (ARM GCC, Make, etc.).
- Clone this repository
- Install the submodules via

```bash
git submodule update --init --recursive
cd lib/DaisySP/
make
cd ../libDaisy/
make
```

## 2. Build

```bash
make clean ; make;
```
The resulting .bin file will appear in the build/ directory.
## 3. Flash
- To flash directly from your computer (via USB DFU mode):
```bash
make program-dfu
```
- OR use the [web flash tool](https://flash.daisy.audio/)