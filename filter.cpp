/*-------------------------------------------------------------------
Joshua Ryan Lam
June 2020
Code taken from assignment 2, which was based on in-class MAP example code
filter.cpp: implement a second-order lowpass filter of variable frequency and Q
-------------------------------------------------------------------*/
// Copied from MAP code, modified by Joshua Ryan Lam, further streamlined by Joshua Ryan Lam

#include <cmath>
#include <complex>
#include "filter.h"

// Constructor
Filter::Filter() : Filter(44100.0) {}

// Constructor specifying a sample rate
Filter::Filter(float sampleRate)
{
	setSampleRate(sampleRate);
	reset();

	// Set some defaults
	filterType_ = kLowPass;
	frequency_ = 1000.0;
	q_ = 0.707;
	ready_ = false;	// This flag will be set to true when the coefficients are calculated
}
	
// Set the sample rate, used for all calculations
void Filter::setSampleRate(float frequency)
{
	sampleRate_ = frequency;
	T_ = 1.0/sampleRate_;
	
	if(ready_)
		calculateCoefficients(frequency_, q_);
}

// Set the frequency and recalculate coefficients
void Filter::setFrequency(float frequency)
{
	frequency_ = frequency;
	calculateCoefficients(frequency_, q_);
}
	
// Set the Q and recalculate the coefficients
void Filter::setQ(float q)
{
	q_ = q;
	calculateCoefficients(frequency_, q_);
}

// Set filter type using enumeration
void Filter::setFilterType(int filterType)
{
	filterType_ = filterType;
	calculateCoefficients(frequency_, q_);
}

// Set filter type using enum from filter.h
void Filter::makeLowPass()
{
	filterType_ = kLowPass;
	calculateCoefficients(frequency_, q_);
}

// Set filter type using enum from filter.h
void Filter::makeHighPass()
{
	filterType_ = kHighPass;
	calculateCoefficients(frequency_, q_);
}

// Set filter type using enum from filter.h
void Filter::makeBandPass()
{
	filterType_ = kBandPass;
	calculateCoefficients(frequency_, q_);
}

// set filter params (Fc, Q, type), -1 to skip setting
void Filter::setFilterParams(float frequency, float q, int filterType)
{
	if (frequency != -1)
		frequency_ = frequency;
	if (q != -1)
		q_ = q;
	if (filterType != -1)
		filterType_ = filterType;
	calculateCoefficients(frequency_, q_);
}
	
// Calculate coefficients
void Filter::calculateCoefficients(float frequency, float q)
{
	// y[n] = 1/b0 * (a0*x[n] + a1*x[n-1] + a2*x[n-2] - b1*y[n-1] - b2*y[n-2])

	// variables to calculate coefficients
	float wd = 2*M_PI*frequency; // normalised desired digital frequency
	// float wa = 2.0/T * tan(wd * T/2); //pre-warped analog frequency

	float w0 = wd; // debug line to toggle pre-warping (w0 should be either wa or wd)

	float qT2w2 = q*pow(T_,2)*pow(w0,2); // q * T^2 * w0^2 comes up pretty often

	//divide other coefficients by B0 so we won't have to divide each time we use the filter
	//division is pretty slow dude
	// y[n] coefficients
	invCoeffB0_ = 1.0 / (4*q + 2*w0*T_ + qT2w2);
	coeffB1_ = (2*qT2w2 - 8*q) * invCoeffB0_;
	coeffB2_ = (4*q + qT2w2 - 2*w0*T_) * invCoeffB0_;
	
	// x[n] coefficients
	switch (filterType_)
	{
	case kLowPass:
		coeffA0_ = coeffA2_ = (qT2w2) * invCoeffB0_;
		coeffA1_ = 2.0 * coeffA0_;
		break;
	case kHighPass:
		coeffA0_ = coeffA2_ = (4*q) * invCoeffB0_;
		coeffA1_ = -2 * coeffA0_;
		break;
	case kBandPass:
		coeffA0_ = (2*q*T_*w0) * invCoeffB0_;
		coeffA1_ = 0;
		coeffA2_ = -coeffA0_;
		break;
	}
	ready_ = true;
}
	
// Reset previous history of filter
void Filter::reset()
{
	lastX_[0] = lastX_[1] = 0;
	lastY_[0] = lastY_[1] = 0;
}
	
// Calculate the next sample of output
float Filter::process(float input)
{
	if(!ready_)
		return input;
	// y[n] = 1/b0 * (a0*x[n] + a1*x[n-1] + a2*x[n-2] - b1*y[n-1] - b2*y[n-2])
	
	//1/b0 already applied in coefficient calculation
	float out = input * coeffA0_ + lastX_[0] * coeffA1_ + lastX_[1] * coeffA2_
    			- lastY_[0] * coeffB1_ - lastY_[1] * coeffB2_;

	// save previous inputs and outputs for next iteration
    lastX_[1] = lastX_[0];
    lastX_[0] = input;
    lastY_[1] = lastY_[0];
    lastY_[0] = out;
    
    return out;
}

// calculate Frequency Response Function and send to GUI
void Filter::updateFrfGraph(Gui& gui, int bufferId)
{
	// calculate FRF from 0 to 20000 Hz
	// Y(w) / X(w) = sum_n(an(exp(-jwn))) / sum_n(bn(exp(-jwn)))
	for (int i = 0; i < FRF_GRAPH_N; i++)
	{
		
		// linear scale between 0 and 2pi
		float w = M_PI * float(i)/(FRF_GRAPH_N - 1);
		// exponential scale between 0 and 2pi
		// float w = powf(1+M_PI,float(i)/(FRF_GRAPH_N - 1))-1;
		
		// complex variables for jw and 2jw
		std::complex<float> jw = 1i * w;
		std::complex<float> jw2 = 2* 1i * w;
		
		// variables for complex exponentials (std::exp adapts to imaginary input)
		std::complex<float> expa1 = coeffA1_ * std::exp(jw);
		std::complex<float> expa2 = coeffA2_ * std::exp(jw2);
		std::complex<float> expb1 = coeffB1_ * std::exp(jw);
		std::complex<float> expb2 = coeffB2_ * std::exp(jw2);
		
		// magnitude of sum of exponentials for numerator and denomerator
		float sumAMag = abs(std::complex<float>(coeffA0_) + expa1 + expa2);
		// all coefficients have already been normalized by B0, so here it is 1.
		float sumBMag = abs(std::complex<float>(1) + expb1 + expb2);
		
		// calculate FRF magnitude
		frf_[i] = sumAMag / sumBMag;
		
		// convert to decibels
		if (frf_[i] > 0.001)
			frf_[i] = 20 * log10(frf_[i]);
		// set threshold for minimum decibel values (avoid log(0))
		else
			frf_[i] = -60;

		//convert to range [-60, 20] to [0, 1] (0.0125 is dividing by 80)
		frf_[i] = (frf_[i] + 60) * 0.0125;
	}
	
	//send to GUI
	gui.sendBuffer(bufferId, frf_);
}
	
// Destructor
Filter::~Filter()
{
	// Nothing to do here
}