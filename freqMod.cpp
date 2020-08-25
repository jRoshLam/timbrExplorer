/***** freqMod.cpp *****/
#include <Bela.h>
#include <math.h>
#include "freqMod.h"

// Constructor
FreqMod::FreqMod() : FreqMod(44100.0, 440.0) {}

// Constructor with sampleRate and Frequency
FreqMod::FreqMod(float sampleRate, float frequency)
{
	sampleRate_ = sampleRate;
	frequency_ = frequency;
	
	// default config is additive synthesis
	opAlgorithm_ = kFmConfigAdd;
	
	// initialize wavetables
	initWavetables();
	
	// initialize operator defaults 
	// operators are non-copyable (have const references) so they must be
	// constructed here, which is why we are using a vector of Operators
	for (unsigned int i = 0; i < NUM_OPERATORS; i++)
	{
		opFreqRatios_[i] = 1;
		opAmplitudes_[i] = 1;
		opFrequencies_[i] = opFreqRatios_[i]*frequency_;
		operators_.push_back(Operator(sampleRate_, tableVector_));
		operators_[i].setup(kWaveSine, frequency_);
		operators_[i].setFrequency(opFrequencies_[i]);
	}
}


// Calculate wavetables
void FreqMod::initWavetables()
{
	//inverse of wavetable size
	float invWavetableSize = 1.0 / float(WAVETABLE_SIZE+1);
	for (int i = 0; i < WAVETABLE_SIZE; i++)
	{
		sineWaveTable_.push_back(sinf(2*M_PI*i*invWavetableSize));
		// all wavetables calculated to be between 1 and -1
		sawWaveTable_.push_back(2*float(WAVETABLE_SIZE - i)/WAVETABLE_SIZE - 1);
		if (i < WAVETABLE_SIZE / 2)
		{
			triWaveTable_.push_back(4*float(i)/WAVETABLE_SIZE - 1);
			squareWaveTable_.push_back(1);
		}
		else
		{
			triWaveTable_.push_back(3.0 - 4* float(i)/WAVETABLE_SIZE);
			squareWaveTable_.push_back(-1);
		}
		
	}
	// add wavetables to vector of vectors
	tableVector_.push_back(sineWaveTable_);
	tableVector_.push_back(triWaveTable_);
	tableVector_.push_back(squareWaveTable_);
	tableVector_.push_back(sawWaveTable_);
}

// Set sample rate of class and its operator objects
void FreqMod::setSampleRate(float frequency)
{
	sampleRate_ = frequency;
	for (unsigned int i = 0; i < NUM_OPERATORS; i++)
		operators_[i].setSampleRate(sampleRate_);
}

// Reset operator frequencies
void FreqMod::reset()
{
	for (unsigned int i = 0; i < NUM_OPERATORS; i++)
		operators_[i].reset();
}

// return a reference to the 0th operator object
const Operator& FreqMod::getDebugOperator()
{
	return operators_[0];
}

// debug Getters
int FreqMod::getWaveTableSize()
{
	// return WAVETABLE_SIZE;
	// return static_cast<int>(operators_.size());
	return static_cast<int>(tableVector_[2].size());
}
float FreqMod::getDebugWaveValue()
{
	// return 0;
	return tableVector_[2][256];
	// return sineWaveTable_.at(256);
}

// Set base frequency of FM sound
void FreqMod::setFrequency(float frequency)
{
	frequency_ = frequency;
	for (unsigned int i = 0; i < NUM_OPERATORS; i++)
	{
		opFrequencies_[i] = opFreqRatios_[i]*frequency_;
		operators_[i].setFrequency(opFrequencies_[i]);
	}
}

// Set frequency ratios of operators
void FreqMod::setFrequencyRatios(const std::vector<float>& ratios)
{
	for (unsigned int i = 0; i < NUM_OPERATORS; i++)
	{
		opFreqRatios_[i] = ratios[i];
		opFrequencies_[i] = opFreqRatios_[i]*frequency_;
		operators_[i].setFrequency(opFrequencies_[i]);
	}
}

// Set amplitudes of operators
void FreqMod::setAmplitudes(const std::vector<float>& amps)
{
	for (unsigned int i = 0; i < NUM_OPERATORS; i++)
	{
		opAmplitudes_[i] = amps[i];
		operators_[i].setAmplitude(opAmplitudes_[i]);
	}
}

// Set waveshapes of operators
void FreqMod::setWaveshapes(const std::vector<int>& waves)
{
	for (unsigned int i = 0; i < NUM_OPERATORS; i++)
	{
		operators_[i].setTable(waves[i]);
	}
}

// Set amplitudes, ratios and waveshapes all at once.
void FreqMod::setSpectrum(const std::vector<float>& amps, const std::vector<float>& ratios, const std::vector<int>& waves)
{
	for (unsigned int i = 0; i < NUM_OPERATORS; i++)
	{
		opFreqRatios_[i] = ratios[i];
		opFrequencies_[i] = opFreqRatios_[i]*frequency_;
		operators_[i].setParameters(amps[i], opFrequencies_[i], waves[i]);
	}
}

// Set operator algorithm based on enumerator (how operators are strung together)
void FreqMod::setAlgorithm(int algorithm)
{
	opAlgorithm_ = algorithm;
}

// calculate and return current sample of the FM waveform
float FreqMod::process()
{
	float out = 0;
	switch(opAlgorithm_)
	{
		// no Modulation, just additive synthesis
		case kFmConfigAdd: {
			for (unsigned int i = 0; i < NUM_OPERATORS; i++)
				out += operators_[i].amplitude()*operators_[i].process(0);
			out *= 0.25;
			break;
		}
		// Operator 0 modulated by operator 1, operator 2 modulated by operator 3
		case kFmConfigDoubleStack22: {
			float mod1 = operators_[1].modulationPhase(0);
			out += operators_[0].amplitude()*operators_[0].process(mod1);
			float mod3 = operators_[3].modulationPhase(0);
			out += operators_[2].amplitude()*operators_[2].process(mod3);
			out *= 0.5;
			break;
		}
		// Operator 2 modulated by operator 3, added to operators 0 and 1
		case kFmConfigDoubleStack31: {
			out += operators_[0].amplitude()*operators_[0].process(0);
			out += operators_[1].amplitude()*operators_[1].process(0);
			float mod3 = operators_[3].modulationPhase(0);
			out += operators_[2].amplitude()*operators_[2].process(mod3);
			out *= 0.333;
			break;
		}
		// Operator 3 modulates operators 0, 1, and 2
		case kFmConfigDoubleStack33: {
			float modulation = operators_[3].modulationPhase(0);
			for (unsigned int i = 0; i < NUM_OPERATORS-1; i++)
				out += operators_[i].amplitude()*operators_[i].process(modulation);
			out *= 0.333;
			break;
		}
		
		// op0 modulated by op3 and op1 modulatd by op2, 
		case kFmConfigTripleStack1: {
			float mod3 = operators_[3].modulationPhase(0);
			float mod2 = operators_[2].modulationPhase(0);
			float mod1 = operators_[1].modulationPhase(mod2);
			out += operators_[0].process(mod3 + mod1);
			break;
		}
		// op0 modulated by op1, which is modulated by op2 and op3
		case kFmConfigTripleStack2: {
			float mod2 = operators_[2].modulationPhase(0);
			float mod3 = operators_[3].modulationPhase(0);
			float mod1 = operators_[1].modulationPhase(mod2 + mod3);
			out += operators_[0].process(mod1);
			break;
		}
		// Operator 3 modulates operator 2 modulates operator 1 modulates operator 0
		case kFmConfigFourStack: {
			float mod3 = operators_[3].modulationPhase(0);
			float mod2 = operators_[2].modulationPhase(mod3);
			float mod1 = operators_[1].modulationPhase(mod2);
			out += operators_[0].amplitude()*operators_[0].process(mod1);
			break;
		}
			
	}
	return out;
}