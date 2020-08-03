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
	invTwoPi = 1.0 / 2.0 / M_PI;
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
	lastFrameCount_ = 0;
	lastOutput_ = 0;
	amplitude_ = 1;
	invTwoPi = 1.0 / 2.0 / M_PI;
}

void Operator::setup(int tableIndex, float frequency)
{
	currentTable_ = tableIndex;
	frequency_ = frequency;
	tableLength_ = float(table_[currentTable_].size());
	lengthXinvSampleRate_ = tableLength_ / sampleRate_;
	
	// tableLength_ = float(tableLength);
	// lengthXinvSampleRate_ = tableLength_ / sampleRate_;
	// table_ = &tableData;
	
	// table_ = tableData;
	// tableLength_ = tableLength;
	
	// // Free the old table, if it exists
	// if(table_ != 0)
	// 	delete[] table_;
	
	// // Allocate memory for the Operator
	// table_ = new float[tableLength];
	
	// // Copy the data to our table
	// if(table_ != 0) {
	// 	for(int i = 0; i < tableLength; i++) {
	// 		table_[i] = tableData[i];
	// 	}
	// 	tableLength_ = tableLength;
	// 	lengthXinvSampleRate_ = tableLength_ / sampleRate_;
	// }
}

void Operator::setSampleRate(float f)
{
	sampleRate_ = f;
	lengthXinvSampleRate_ = tableLength_ / sampleRate_;
}

void Operator::reset()
{
	lastFrameCount_ = 0;
	lastOutput_ = 0;
}

// Setters
void Operator::setAmplitude(float a) {
	amplitude_ = a;
	// Amplitude to use when operator is a modulator
	// amplitude_ * frequency_ * tableLength_  / sampleRate_
	modAmplitude_ = amplitude_ * tableLength_ * invTwoPi;
	// modAmplitude_ = amplitude_ * phaseIncr_;
}

// Set the oscillator frequency
void Operator::setFrequency(float f) {
	frequency_ = f;
	// tableLength * freq / sampleRate
	phaseIncr_ = frequency_ * lengthXinvSampleRate_;
	// modAmplitude_ = amplitude_ * lengthXinvSampleRate_ * invTwoPi;
	modAmplitude_ = amplitude_ * tableLength_ * invTwoPi;
}

void Operator::setTable(int i)
{
	currentTable_ = i;
	tableLength_ = float(table_[currentTable_].size());
	lengthXinvSampleRate_ = tableLength_ / sampleRate_;
	phaseIncr_ = frequency_ * lengthXinvSampleRate_;
	// modAmplitude_ = amplitude_ * lengthXinvSampleRate_ * invTwoPi;
	modAmplitude_ = amplitude_ * tableLength_ * invTwoPi;
}

void Operator::setParameters(float amplitude, float frequency, int table)
{
	currentTable_ = table;
	amplitude_ = amplitude;
	frequency_ = frequency;
	
	tableLength_ = float(table_[currentTable_].size());
	lengthXinvSampleRate_ = tableLength_ / sampleRate_;
	phaseIncr_ = frequency_ * lengthXinvSampleRate_;
	// modAmplitude_ = amplitude_ * lengthXinvSampleRate_ * invTwoPi;
	modAmplitude_ = amplitude_ * tableLength_ * invTwoPi;
}

// Getters
float Operator::amplitude() {
	return amplitude_;
}

float Operator::modAmplitude() {
	return modAmplitude_;
}

// Get the oscillator frequency
float Operator::frequency() {
	return frequency_;
}

float Operator::tableLength()
{
	// return float(table_.size());
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

float Operator::modulationPhase(float modulation)
{
	if(tableLength_ == 0)
		return 0;
		
	float waveValue = process(modulation);
	
	// sin_n + (sin_n - sin_n-1)*(n-1)
	// float phase = waveValue + (waveValue - lastOutput_)*float(lastFrameCount_);
	float phase = waveValue - lastOutput_;
	
	//set values for next iteration
	lastFrameCount_++;
	lastOutput_ = waveValue;
	
	//A*L/2pi/Fs*(sin_n + (sin_n - sin_n-1)*(n-1))
	//A*L/2pi(sin_n - sin_n-1)
	return modAmplitude_ * phase;
}

// float Operator::modulatedProcess(Operator modulator)
// {
// 	if(tableLength_ == 0)
// 		return 0;
		
// 	// Increment and wrap the phase
// 	phase_ += (phaseIncr_ + modulator.modulationPhase(0));
// 	while(phase_ >= tableLength_)
// 		phase_ -= tableLength_;
// 	while(phase_ < 0)
// 		phase_ += tableLength_;
	
// 	// interpolation
// 	int indexBelow = floorf(phase_);
// 	int indexAbove = indexBelow + 1;
// 	if (indexAbove >= tableLength_) indexAbove = 0;
// 	float fractionAbove = phase_ - indexBelow;
// 	float fractionBelow = 1.0 - fractionAbove;
// 	return  (fractionBelow * table_[currentTable_][indexBelow] +
//     				fractionAbove * table_[currentTable_][indexAbove]);
// 	// return  (fractionBelow * table_->at(indexBelow) +
//  //   				fractionAbove * table_->at(indexAbove));
// }
	
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
	// return  (fractionBelow * table_->at(indexBelow) +
 //   				fractionAbove * table_->at(indexAbove));
}	
	
// Destructor
Operator::~Operator() {
	// Free the memory we allocated (if any)
	// if(table_ != 0)
	// 	delete[] table_;
}			
