/***** spectralDensity.h *****/
#ifndef SPECTRUM_H
#define SPECTRUM_H

#include "freqMod.h"
#include <vector>

#define MAX_SPECTRUM 256

class Spectrum
{
public:
	Spectrum();
	Spectrum(float sampleRate, float frequency);
	
	void setSampleRate(float f);
	
	void reset();
	
	int getSpectrum();
	Operator getDebugOperator();
	int getWaveTableSize();
	float getDebugWaveValue();
	
	void updateSpectrum(int spectrum);
	
	void setFrequency(float frequency);
	
	float process();
	
private:
	FreqMod fmSynth_;
	
	int spectrum_;
	
	// int sineThreshold_;
	
	float sampleRate_;
	float frequency_;
	
	std::vector<float> amps_;
	std::vector<float> fRatios_;
	std::vector<int> opWaves_;
};

#endif