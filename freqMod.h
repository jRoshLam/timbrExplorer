/***** freqMod.h *****/
#ifndef FREQMOD_H
#define FREQMOD_H

#include <vector>
#include <array>
#include "operator.h"


#define NUM_OPERATORS 4

enum fmConfigs
{
	kFmConfigAM = 0,
	kFmConfigDoubleStack22,
	kFmConfigDoubleStack31,
	kFmConfigDoubleStack33,
	kFmConfigTripleStack1,
	kFmConfigTripleStack2,
	kFmConfigFourStack,
	kAmConfigOddEven
};

enum waveShapes
{
	kWaveSine = 0,
	kWaveTriangle,
	kWaveSquare,
	kWaveSaw
};

//
class FreqMod
{
public:
	// Constructor
	FreqMod();
	
	// Constructor with sampleRate and Frequency
	FreqMod(float sampleRate, float frequency);
	
	void setSampleRate(float f);
	
	void reset();
	
	void initWavetables();
	
	const Operator& getDebugOperator();
	int getWaveTableSize();
	float getDebugWaveValue();
	
	// Set base frequency of FM sound
	void setFrequency(float frequency);
	
	// Set frequency ratios of operators
	void setFrequencyRatios(float ratio0, float ratio1, float ratio2, float ratio3);
	void setFrequencyRatios(const std::vector<float>& ratios);
	
	// Set amplitudes of operators
	void setAmplitudes(float amp0, float amp1, float amp2, float amp3);
	void setAmplitudes(const std::vector<float>& amps);
	
	// Set wavetable of operators
	void setWaveshapes(const std::vector<int>& waves);
	
	// Set amplitudes, frequency, and wavetables of operators
	void setSpectrum(const std::vector<float>& amps, const std::vector<float>& ratios, const std::vector<int>& waves);
	
	// Set operator algorithm based on enumerator (how operators are strung together)
	void setAlgorithm(int algorithm);
	
	// calculate and return current sample of the FM waveform
	float process();
	
private:
	// Source Wavetable 
	std::vector<std::vector<float>> tableVector_;
	std::vector<float> sineWaveTable_;
	std::vector<float> triWaveTable_;
	std::vector<float> squareWaveTable_;
	std::vector<float> sawWaveTable_;
	// float sineWaveTable_[WAVETABLE_SIZE];
	// float squareWaveTable_[WAVETABLE_SIZE];
	// float sawWaveTable_[WAVETABLE_SIZE];
	
	// FM Operators
	std::vector<Operator> operators_;
	
	//ratios of FM operator frequencies to the base frequency
	float opFreqRatios_[NUM_OPERATORS] = {1, 1, 1, 1};
	float opFrequencies_[NUM_OPERATORS];
	
	float opAmplitudes_[NUM_OPERATORS] = {1, 1, 1, 1};
	
	// sample rate
	float sampleRate_;
	// Base Frequency
	float frequency_;
	
	int opAlgorithm_;
	
};


#endif