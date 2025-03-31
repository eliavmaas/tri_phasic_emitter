#ifndef SOUNDPROCESSING_H
#define SOUNDPROCESSING_H

#include <arduinoFFT.h>

#define AUDIO_PIN A0
#define SAMPLES 32
#define SAMPLING_FREQUENCY 5000

extern ArduinoFFT<double> FFT;

void setupSoundProcessing();
void processSound();

#endif