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
	
	filterQ_ = 1;
	velocityQ_ = 1;
	advMode_ = false;
	midiLink_ = true;
	
	// All-pass setup
	allPass_ = true;
	// by default all-pass is middle 10% of articulation range
	allPassZone_ = 0.05;
	brThresholdLP_ = int(MAX_BRIGHTNESS * (0.5 - allPassZone_));
	brThresholdHP_ = int(MAX_BRIGHTNESS * (0.5 + allPassZone_));
	
	// min and max low-pass and high-pass Fc values based on human hearing range
	minLPFc_ = 50;
	maxLPFc_ = 20000;
	minHPFc_ = 0;
	maxHPFc_ = 15000;
	
	brightness_ = MAX_BRIGHTNESS / 2;
	
	// calculate Fc's for lookup table
	initBrightnessTable();

	// setup Filter objects
	for (unsigned int n = 0; n < NUMBER_OF_FILTERS; n++)
	{
		brFilters_[n].setSampleRate(sampleRate_);
		brFilters_[n].setQ(filterQ_);
	}
}

// calculate Fc's for lookup table to convert from brightness to Fc
void Brightness::initBrightnessTable()
{
	// reserve memory for brightness lookup table
	brightToFcTable_.reserve(MAX_BRIGHTNESS);
	for (unsigned int br = 0; br < MAX_BRIGHTNESS; br++)
	{
		// if brightness is in low-pass zone (lower end of range)
		if (br <= brThresholdLP_)
		{
			// Fc scales exponentially with brightness
			brightToFcTable_[br] = minLPFc_ + pow(maxLPFc_ - minLPFc_, float(br)/float(brThresholdLP_));
		}
		// if brightness is in high-pass zone (upper end of range)
		else if (br >= brThresholdHP_)
		{
			// Fc scales exponentially with brightness
			brightToFcTable_[br] = minHPFc_ + pow(maxHPFc_ - minHPFc_, float(br - brThresholdHP_)/float(MAX_BRIGHTNESS - brThresholdHP_));
		}
		// otherwise we are in all-pass zone and the Fc will not be used
		else
		{
			brightToFcTable_[br] = 0;
		}
	}
}

// set sample rate of brightness and its Filter objects
void Brightness::setSampleRate(float frequency)
{
	sampleRate_ = frequency;
	for (unsigned int n = 0; n < NUMBER_OF_FILTERS; n++)
		brFilters_[n].setSampleRate(sampleRate_);
}

// Getters
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

// reset Filter objects
void Brightness::reset()
{
	for (unsigned int n = 0; n < NUMBER_OF_FILTERS; n++)
	{
		brFilters_[n].reset();
	}
}

// Set frequency (Fc is relative to frequency)
void Brightness::setFrequency(float frequency)
{
	frequency_ = frequency;
}

// set frequency of current note and filter resonance. Update filter
void Brightness::setMidiIn(float frequency, float qFactor)
{
	// if frequency and Q factor both have not changed, nothing to do
	if (frequency_ == frequency && velocityQ_ == qFactor)
		return;
	frequency_ = frequency;
	velocityQ_ = qFactor;
	// if there is no midi link, only change the params, don't change the filter
	if (!midiLink_)
		return;
	
	filterQ_ = velocityQ_;
	// add transition zone for High-Pass where cutoff frequency starts at 0
	// this smoothes out the sound when making the transition
	if(brightness_ >= brThresholdHP_ && brightness_ < brThresholdHP_+10)
		filterFc_ = 0.1 * (brightness_ - brThresholdHP_) * (frequency_ + brightToFcTable_[brightness_]);
	// otherwise, brightness zone should be relative to note frequency (Marozeau)
	else
		filterFc_ = frequency_ + brightToFcTable_[brightness_];
		
	//update filter
	if (!allPass_)
		for (unsigned int n = 0; n < NUMBER_OF_FILTERS; n++)
		{
			// set filter params (Fc, Q, type), -1 to skip setting
			brFilters_[n].setFilterParams(filterFc_, filterQ_, filterType_);
		}
}

// Toggle enable for advanced controls
void Brightness::setAdvMode(bool advMode)
{
	advMode_ = advMode;
	midiLink_ = false;
}

// Update midiLink boolean and Q factor, as necessary
void Brightness::setAdvControls(float midiLink, float qFactor)
{
	// if we're not in advanced mode, exit function
	if (!advMode_)
		return;
	// if the input midi link is true, update as necessary and then exit function
	if (midiLink == 1) {
		// if we have just turned midiLink back on, update filter
		if(!midiLink_) {
			midiLink_ = true;
			filterQ_ = velocityQ_;
			// add transition zone for High-Pass where cutoff frequency starts at 0
			// this smoothes out the sound when making the transition
			if(brightness_ >= brThresholdHP_ && brightness_ < brThresholdHP_+10)
				filterFc_ = 0.1 * (brightness_ - brThresholdHP_) * (frequency_ + brightToFcTable_[brightness_]);
			// otherwise, brightness zone should be relative to note frequency (Marozeau)
			else
				filterFc_ = frequency_ + brightToFcTable_[brightness_];
				
			//update filter
			if (!allPass_)
				for (unsigned int n = 0; n < NUMBER_OF_FILTERS; n++)
				{
					// set filter params (Fc, Q, type), -1 to skip setting
					brFilters_[n].setFilterParams(filterFc_, filterQ_, filterType_);
				}
		}
		return;
	}
	bool updateFilter = false;
	// if we're here, input midiLink = 0 and
	// if midi link has just turned off (midiLink_ still true), we need to update Fc
	if (midiLink_) {
		updateFilter = true;
		midiLink_ = false;
		// add transition zone for High-Pass where cutoff frequency starts at 0
		// this smoothes out the sound when making the transition
		if(brightness_ >= brThresholdHP_ && brightness_ < brThresholdHP_+10)
			filterFc_ = 0.1 * (brightness_ - brThresholdHP_) * (brightToFcTable_[brightness_]);
		// otherwise, brightness zone should be relative to note frequency (Marozeau)
		else
			filterFc_ = brightToFcTable_[brightness_];
	}

	// if we're here, midiLink is off. If we have a new Q factor from the gui, update filter
	if (filterQ_ != qFactor) {
		updateFilter = true;
		filterQ_ = qFactor;
	}
	// if we are not in all pass mode and either the Fc or Q changed, update filter
	if (updateFilter && !allPass_)
		for (unsigned int n = 0; n < NUMBER_OF_FILTERS; n++)
			brFilters_[n].setFilterParams(filterFc_, filterQ_, filterType_);
}

// Update filter Fc based on new brightness value
void Brightness::updateBrightness(int brightness)
{
	// If brightness hasn't changed, skip calculations
	if (brightness_ == brightness)
		return;

	brightness_ = brightness;

	float targetFc;
	// if midiLink_ is true, the fundamental frequency_ is added to the Fc
	// (brightness is relative to note frequency, Marozeau & deChevigne)
	if (midiLink_)
		targetFc = frequency_ + brightToFcTable_[brightness_];
	// otherwise, simply use the lookup table value
	else
		targetFc = brightToFcTable_[brightness_];
	
	// add transition zone for High-Pass where cutoff frequency starts at 0
	// this smoothes out the sound when making the transition
	if(brightness_ >= brThresholdHP_ && brightness_ < brThresholdHP_+10)
		filterFc_ = 0.1 * (brightness_ - brThresholdHP_) * targetFc;
	// if not in transition zone, no need to make further changes
	else
		filterFc_ = targetFc;
	
	//check brightness_ if low-pass
	if (brightness_ <= brThresholdLP_)
	{
		filterType_ = kLowPass;
		allPass_ = false;
	}
	//check brightness_ if high-pass
	else if (brightness_ >= brThresholdHP_)
	{
		filterType_ = kHighPass;
		allPass_ = false;
	}
	// If not low-pass or high-pass, all-pass
	else
		allPass_ = true;

	//update filter
	if (!allPass_)
		for (unsigned int n = 0; n < NUMBER_OF_FILTERS; n++)
		{
			// set filter params (Fc, Q, type), -1 to skip setting
			brFilters_[n].setFilterParams(filterFc_, filterQ_, filterType_);
		}
}

// Apply brightness filters to input sample
float Brightness::process(float sampleIn)
{
	//check all-pass boolean (if true, do not apply filter)
	if (!allPass_)
		for (unsigned int n = 0; n < NUMBER_OF_FILTERS; n++)
			sampleIn = brFilters_[n].process(sampleIn);
	
	return sampleIn;
}

// update FRF graph and send it to the GUI
void Brightness::updateFrfGraph(Gui& gui, int bufferId)
{
	// if all-pass, graph y=0.75 (equivalent amplitude of 1 in our converted decibel scale)
	if (allPass_)
		gui.sendBuffer(bufferId, 0.75);
	// otherwise, calculate FRF graph and send it.
	else
		brFilters_[0].updateFrfGraph(gui, bufferId);
}
