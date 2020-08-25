/***** freqMod.h *****/
#ifndef FREQMOD_H
#define FREQMOD_H

#include <vector>
#include <array>
#include "operator.h"


#define NUM_OPERATORS 4


// Enumeration for Operator configurations
enum fmConfigs
{
	kFmConfigAdd = 0,
	kFmConfigDoubleStack22,
	kFmConfigDoubleStack31,
	kFmConfigDoubleStack33,
	kFmConfigTripleStack1,
	kFmConfigTripleStack2,
	kFmConfigFourStack,
	kAmConfigOddEven
};

// Enumeration for waveshapes
enum waveShapes
{
	kWaveSine = 0,
	kWaveTriangle,
	kWaveSquare,
	kWaveSaw
};


class FreqMod
{
public:
	// Constructor
	FreqMod();
	
	// Constructor with sampleRate and Frequency
	FreqMod(float sampleRate, float frequency);
	
	// Set sample rate 
	void setSampleRate(float frequency);
	
	// reset operator phases
	void reset();
	
	// calculate wavetable values
	void initWavetables();
	
	// retrieve a reference to an operators for debug purposes
	const Operator& getDebugOperator();
	// other getters for debug purposes
	int getWaveTableSize();
	float getDebugWaveValue();
	
	// Set base frequency of FM sound
	void setFrequency(float frequency);
	
	// Set frequency ratios of operators
	void setFrequencyRatios(const std::vector<float>& ratios);
	
	// Set amplitudes of operators
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
	// wavetables as a vector of vectors.
	// First index selects the wavetable, second index indexes wavetable contents
	std::vector<std::vector<float>> tableVector_;
	//individual wavetables as vectors
	std::vector<float> sineWaveTable_; // sine
	std::vector<float> triWaveTable_; // triangle
	std::vector<float> squareWaveTable_; // square
	std::vector<float> sawWaveTable_; //sawtooth
	
	// FM Operators
	std::vector<Operator> operators_;
	
	// ratios of FM operator frequencies to the base frequency
	float opFreqRatios_[NUM_OPERATORS];
	// operator frequencies, product of base frequency and ratios
	float opFrequencies_[NUM_OPERATORS];
	// operator amplitudes
	float opAmplitudes_[NUM_OPERATORS];
	// current FM algorithm (how to arrange operators)
	int opAlgorithm_;
	
	// sample rate
	float sampleRate_;
	// Base Frequency
	float frequency_;
	
};


#endif