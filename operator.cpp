// ECS7012P Music and Audio Programming
// School of Electronic Engineering and Computer Science
// Queen Mary University of London
// Spring 2020

// perator.cpp: file for implementing the Operator oscillator class

#include <cmath>
#include "operator.h"

// Default constructor: set default values
Operator::Operator() : table_(dummyTable)
{
	sampleRate_ = 44100.0;
	tableLength_ = 0;
	currentTable_ = 0;
	// table_ = 0;
	lengthXinvSampleRate_ = 0;
	phaseIncr_ = 0;
	invTwoPi_ = 1.0 / 2.0 / M_PI;
}

// Constructor taking arguments
// Can also use initialisation lists instead of setting 
// variables inside the function
Operator::Operator(float sampleRate, const std::vector<std::vector<float>>& tableData) : table_(tableData)
{
	sampleRate_ = sampleRate;
	currentTable_ = 0;
	phaseIncr_ = 0;
	phase_ = 0;
	lastOutput_ = 0;
	amplitude_ = 1;
	invTwoPi_ = 1.0 / 2.0 / M_PI;
}


// Further setup to initialize waveshape and frequency
void Operator::setup(int tableIndex, float frequency)
{
	currentTable_ = tableIndex;
	frequency_ = frequency;
	tableLength_ = float(table_[currentTable_].size());
	lengthXinvSampleRate_ = tableLength_ / sampleRate_;
}

// Set the samplerate of operator
void Operator::setSampleRate(float f)
{
	sampleRate_ = f;
	lengthXinvSampleRate_ = tableLength_ / sampleRate_;
}

// reset state of operator
void Operator::reset()
{
	phase_ = 0;
	lastOutput_ = 0;
}

// Setters
void Operator::setAmplitude(float amplitude) {
	amplitude_ = amplitude;
	// Amplitude to use when operator is a modulator
	// amplitude * frequency * tableLength  / sampleRate
	modAmplitude_ = amplitude_ * tableLength_ * invTwoPi_;
}

// Set the oscillator frequency
void Operator::setFrequency(float frequency) {
	frequency_ = frequency;
	// amount to increment phase each frame
	// phaseIncr = tableLength * frequency / sampleRate
	phaseIncr_ = frequency_ * lengthXinvSampleRate_;
	modAmplitude_ = amplitude_ * tableLength_ * invTwoPi_;
}

// Set waveshape using enumerator
void Operator::setTable(int waveShapeEnum)
{
	currentTable_ = waveShapeEnum;
	//retrieve new wavetable size and re-calculate relevant quantities
	tableLength_ = float(table_[currentTable_].size());
	lengthXinvSampleRate_ = tableLength_ / sampleRate_;
	phaseIncr_ = frequency_ * lengthXinvSampleRate_;
	modAmplitude_ = amplitude_ * tableLength_ * invTwoPi_;
}

// Set amplitude, frequency and waveshape all at once
// Since there are so many quantities that are interdependent, having a single function
// will save time.
void Operator::setParameters(float amplitude, float frequency, int table)
{
	currentTable_ = table;
	amplitude_ = amplitude;
	frequency_ = frequency;
	
	tableLength_ = float(table_[currentTable_].size());
	lengthXinvSampleRate_ = tableLength_ / sampleRate_;
	phaseIncr_ = frequency_ * lengthXinvSampleRate_;
	// modAmplitude_ = amplitude_ * lengthXinvSampleRate_ * invTwoPi_;
	modAmplitude_ = amplitude_ * tableLength_ * invTwoPi_;
}

// Getters
float Operator::amplitude() {
	return amplitude_;
}
float Operator::modAmplitude() {
	return modAmplitude_;
}
float Operator::frequency() {
	return frequency_;
}
float Operator::tableLength()
{
	return tableLength_;
}
float Operator::phaseIncr()
{
	return phaseIncr_;
}
float Operator::debugValue()
{
	// return currentTable_;
	return table_[1][256];
}

// calculate modulation phase (amount to add to modulatee's phase) 
float Operator::modulationPhase(float modulation)
{
	if(tableLength_ == 0)
		return 0;

	float waveValue = process(modulation);
	
	// (sin_{n} - sin_{n-1})
	float phase = waveValue - lastOutput_;
	
	//set values for next iteration
	lastOutput_ = waveValue;
	
	//A*L/2pi(sin_n - sin_n-1)
	return modAmplitude_ * phase;
}
	
// Get the next sample and update the phase
float Operator::process(float modulation) {
	if(tableLength_ == 0)
		return 0;
		
	if (amplitude_ == 0)
		return 0;
	
	// Increment and wrap the phase
	phase_ += phaseIncr_ + modulation;
	while(phase_ >= tableLength_)
		phase_ -= tableLength_;
	while(phase_ < 0)
		phase_ += tableLength_;
	
	// interpolation
	int indexBelow = floorf(phase_);
	int indexAbove = indexBelow + 1;
	if (indexAbove >= tableLength_) indexAbove = 0;
	float fractionAbove = phase_ - indexBelow;
	float fractionBelow = 1.0 - fractionAbove;
	return  (fractionBelow * table_[currentTable_][indexBelow] +
				fractionAbove * table_[currentTable_][indexAbove]);
}	
	
// Destructor
Operator::~Operator() {
	// Nothing to do
}			
