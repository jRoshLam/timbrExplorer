/***** articulation.cpp *****/
#include "articulation.h"
#include <cmath>

// Constructor
Articulation::Articulation() : Articulation(44100.0) {}

// Constructor with custom sampleRate
Articulation::Articulation(float sampleRate)
{
	sampleRate_ = sampleRate;
	
	frequency_ = 0;
	
	articulation_ = MAX_ARTICULATION / 2;
	allPass_ = 0;
	allPassZone_ = 0.05;
	
	arThresholdLP_ = int(MAX_ARTICULATION * (0.5 - allPassZone_));
	arThresholdHP_ = int(MAX_ARTICULATION * (0.5 + allPassZone_));
	
	baseFc_ = 0;
	deltaFc_ = 0;
	minFc_ = 50;
	maxFc_ = 20000;
	
	// articulation time in milliseconds
	minArticTime_ = 5;
	maxArticTime_ = 1000;
	
	filterType_ = kLowPass;
	filterQ_ = 1.0;
	filterFc_ = minFc_;
	
	articuFilter_ = Filter(sampleRate_);
	
	initArticulationTable();
}

void Articulation::initArticulationTable()
{
	// float steepest_slope = powf((maxFc_ - minFc_), 1 / (sampleRate_ * 0.005));
	// float shallowest_slope = (maxFc_ - minFc_) / (sampleRate_ * 0.6);
	// float slopeDiff = (steepest_slope - shallowest_slope);
	float articTime = 0;
	articToBaseFcTable_.reserve(MAX_ARTICULATION);
	for (unsigned int ar = 0; ar < MAX_ARTICULATION; ar++)
	{
		if (ar <= arThresholdLP_)
		{
			articTime = (minArticTime_ + powf(maxArticTime_ - minArticTime_, float(arThresholdLP_ - ar)/float(arThresholdLP_))) * 0.001;
			// articTime = (maxArticTime_ - powf(maxArticTime_ - minArticTime_, float(ar)/float(arThresholdLP_))) * 0.001;
			articToBaseFcTable_[ar] = powf((maxFc_ - minFc_), 1 / (sampleRate_ * articTime));
		}
		else if (ar >= arThresholdHP_)
		{
			// articTime = (minArticTime_ + powf(maxArticTime_ - minArticTime_, float(ar - arThresholdHP_)/float(MAX_ARTICULATION - arThresholdHP_))) * 0.001;
			articTime = (minArticTime_ + powf(maxArticTime_ - minArticTime_, float(ar - arThresholdHP_)/float(MAX_ARTICULATION - arThresholdHP_))) * 0.001;
			// articToBaseFcTable_[ar] = articTime;
			articToBaseFcTable_[ar] = 1 / powf((maxFc_ - minFc_), 1 / (sampleRate_ * articTime));
		}
		else
		{
			articToBaseFcTable_[ar] = 0;
		}
	}
}

void Articulation::setSampleRate(float f)
{
	sampleRate_ = f;
	articuFilter_.setSampleRate(sampleRate_);
}

void Articulation::setFrequency(float frequency)
{
	frequency_ = frequency;
}

// reset to intital Fc (min if lowpass, max if highpass)
void Articulation::reset()
{
	articuFilter_.reset();
	if (filterType_ == kLowPass)
	{
		deltaFc_ = 1;
		filterFc_ = 0;
	}
	else if (filterType_ == kHighPass)
	{
		deltaFc_ = maxFc_ - minFc_;
		filterFc_ = maxFc_;
	}
}

// Getters
int Articulation::getArticulation()
{
	return articulation_;
}
float Articulation::getSampleRate()
{
	return sampleRate_;
}
int Articulation::getFilterType()
{
	return filterType_;
}
float Articulation::getBaseFc()
{
	return baseFc_;
}
float Articulation::getFilterFc()
{
	return filterFc_;
}

// Change how the filter Fc changes
void Articulation::updateArticulation(int articulation)
{
	if (articulation_ == articulation)
		return;
	
	articulation_ = articulation;
	if (articulation_ > arThresholdLP_ && articulation_ < arThresholdHP_)
		allPass_ = true;
	else
		allPass_ = false;
	
	// Update slope
	baseFc_ = articToBaseFcTable_[articulation_];
	
	//check articulation_ if low-pass
	if (articulation_ <= arThresholdLP_)
		filterType_ = kLowPass;

	//check articulation_ if high-pass
	if (articulation_ >= arThresholdHP_)
		filterType_ = kHighPass;
		
	// reset();
	
}

// Update Fc and apply filter
float Articulation::process(float sampleIn)
{
	// Determine whether filter should be applied
	// Check all-pass condition (neutral articulation)
	if (allPass_)
		return sampleIn;
	
	// Check if articulation has completed
	if (filterType_ == kLowPass && filterFc_ >= maxFc_)
		return sampleIn;
	else if (filterType_ == kHighPass && filterFc_ <= minFc_+1)
		return sampleIn;
	
	// Otherwise, update and apply filter
	// Update FilterFc
	deltaFc_ *= baseFc_;
	filterFc_ = frequency_ + minFc_ + deltaFc_;
	articuFilter_.setFilterParams(filterFc_, -1, filterType_);
	
	// Apply filter
	return articuFilter_.process(sampleIn);
	// return sampleIn;
}