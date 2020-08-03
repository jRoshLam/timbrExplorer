/***** filter.cpp *****/
#include <cmath>
#include "brightness.h"

// Constructor
Brightness::Brightness() : Brightness(44100.0, 440.0) {}

// Constructor specifying a sample rate
Brightness::Brightness(float sampleRate, float frequency)
{
	sampleRate_ = sampleRate;
	frequency_ = frequency;
	allPassZone_ = 0.05;
	filterQ_ = 1;
	
	brThresholdLP_ = int(MAX_BRIGHTNESS * (0.5 - allPassZone_));
	brThresholdHP_ = int(MAX_BRIGHTNESS * (0.5 + allPassZone_));
	
	minLPFc_ = 50;
	maxLPFc_ = 20000;
	minHPFc_ = 0;
	maxHPFc_ = 15000;
	
	brightness_ = MAX_BRIGHTNESS / 2;
	
	initBrightnessTable();
	
	for (unsigned int n = 0; n < NUMBER_OF_FILTERS; n++)
	{
		brFilters_[n].setSampleRate(sampleRate_);
		brFilters_[n].setQ(filterQ_);
	}
}

void Brightness::initBrightnessTable()
{
	brightToFcTable_.reserve(MAX_BRIGHTNESS);
	for (unsigned int br = 0; br < MAX_BRIGHTNESS; br++)
	{
		if (br <= brThresholdLP_)
		{
			brightToFcTable_[br] = minLPFc_ + pow(maxLPFc_ - minLPFc_, float(br)/float(brThresholdLP_));
		}
		else if (br >= brThresholdHP_)
		{
			brightToFcTable_[br] = minHPFc_ + pow(maxHPFc_ - minHPFc_, float(br - brThresholdHP_)/float(MAX_BRIGHTNESS - brThresholdHP_));
		}
		else
		{
			brightToFcTable_[br] = 0;
		}
	}
}

void Brightness::setSampleRate(float f)
{
	sampleRate_ = f;
	for (unsigned int n = 0; n < NUMBER_OF_FILTERS; n++)
		brFilters_[n].setSampleRate(sampleRate_);
}

int Brightness::getBrightness()
{
	return brightness_;
}
float Brightness::getFc()
{
	return filterFc_;
}

float Brightness::getQ()
{
	return filterQ_;
}

void Brightness::reset()
{
	for (unsigned int n = 0; n < NUMBER_OF_FILTERS; n++)
	{
		brFilters_[n].reset();
	}
}

void Brightness::setFrequency(float frequency)
{
	frequency_ = frequency;
}

void Brightness::updateBrightness(int brightness)
{
	if (brightness_ == brightness)
		return;

	brightness_ = brightness;
	//check brightness_ for all-pass condition condition
	if (brightness_ > brThresholdLP_ && brightness_ < brThresholdHP_)
		allPass_ = true;
	else
		allPass_ = false;

	// add transition zone for High-Pass where cutoff frequency starts at 0
	if(brightness_ >= brThresholdHP_ && brightness_ < brThresholdHP_+10)
		filterFc_ = 0.1 * (brightness_ - brThresholdHP_) * (frequency_ + brightToFcTable_[brightness_]);
	// otherwise, brightness zone should be relative to note frequency (Marozeau)
	else
		filterFc_ = frequency_ + brightToFcTable_[brightness_];
	//check brightness_ if low-pass
	if (brightness_ <= brThresholdLP_)
		filterType_ = kLowPass;

	//check brightness_ if high-pass
	if (brightness_ >= brThresholdHP_)
		filterType_ = kHighPass;
	
	//update filter
	if (!allPass_)
		for (unsigned int n = 0; n < NUMBER_OF_FILTERS; n++)
		{
			// set filter params (Fc, Q, type), -1 to skip setting
			brFilters_[n].setFilterParams(filterFc_, -1, filterType_);
		}
}

float Brightness::process(float sampleIn)
{
	//check all-pass boolean (if true, do not apply filter)
	if (!allPass_)
		for (unsigned int n = 0; n < NUMBER_OF_FILTERS; n++)
			sampleIn = brFilters_[n].process(sampleIn);
	
	return sampleIn;
}

