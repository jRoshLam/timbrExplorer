/*-------------------------------------------------------------------
Joshua Ryan Lam
June 2020
Code taken from assignment 2, which was based on in-class MAP example code
filter.h: header file for defining a second order resonant kow pass filter
-------------------------------------------------------------------*/
// Copied from MAP code, modified by Joshua Ryan Lam, further streamlined by Joshua Ryan Lam

#ifndef FILTER_H
#define FILTER_H

//enumerator for filterType
enum {
	kLowPass = 0,
	kHighPass,
	kBandPass
};

//2nd order IIR filter
class Filter {
public:
	// Constructor
	Filter();
	
	// Constructor specifying a sample rate
	Filter(float sampleRate);
	
	// Set the sample rate, used for all calculations
	void setSampleRate(float rate);
	
	// Set the frequency and recalculate coefficients
	void setFrequency(float frequency);
	
	// Set the Q and recalculate the coefficients
	void setQ(float q);
	
	// Set the type of the filter
	void setFilterType(int filterType);
	void makeLowPass();
	void makeHighPass();
	void makeBandPass();
	
	// set multiple parameters at once (Fc, Q, type), -1 to skip setting
	void setFilterParams(float frequency, float q, int filterType);
	
	// Reset previous history of filter
	void reset();
	
	// Calculate the next sample of output, changing the envelope
	// state as needed
	float process(float input); 
	
	// Destructor
	~Filter();

private:
	// Calculate coefficients
	void calculateCoefficients(float frequency, float q);

	// State variables, not accessible to the outside world
	bool ready_;	// Have the coefficients been calculated?
	int filterType_;
	float sampleRate_, T_;
	float frequency_;
	float q_;
	float coeffA0_, coeffA1_, coeffA2_, invCoeffB0_, coeffB1_, coeffB2_;
	float lastX_[2];
	float lastY_[2];
};

#endif