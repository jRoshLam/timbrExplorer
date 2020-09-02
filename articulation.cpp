/***** articulation.cpp *****/
#include "articulation.h"
#include <cmath>

// Constructor
Articulation::Articulation() : Articulation(44100.0) {}

// Constructor with custom sampleRate
Articulation::Articulation(float sampleRate)
{
	sampleRate_ = sampleRate;
	
	// Set default values (will likely be overwritten elsewhere)
	frequency_ = 0;
	articulation_ = MAX_ARTICULATION / 2;

	// All pass setup
	allPass_ = false;
	// by default all-pass is middle 10% of articulation range
	allPassZone_ = 0.05;
	arThresholdLP_ = int(MAX_ARTICULATION * (0.5 - allPassZone_));
	arThresholdHP_ = int(MAX_ARTICULATION * (0.5 + allPassZone_));
	
	// min and max articulation time in milliseconds
	minArticTime_ = 5;
	maxArticTime_ = 600;

	// Fc initialization
	baseFc_ = 0;
	deltaFc_ = 0;
	// min and max Fc in Hz, based off of human hearing range.
	minFc_ = 50;
	maxFc_ = 20000;
	
	// Initialize filter parameters 
	filterQ_ = 1.0; // Q-factor fixed at 1.0
	// these are overwritten by updateArticulation()
	filterType_ = kLowPass;
	filterFc_ = minFc_;
	
	// Initialize Filter object
	articuFilter_ = Filter(sampleRate_);
	
	initArticulationTable();
}

// Initialize lookup table matching articulation to baseFc
void Articulation::initArticulationTable()
{
	float articTime = 0;
	articToBaseFcTable_.reserve(MAX_ARTICULATION); // reserve size for vector
	for (unsigned int ar = 0; ar < MAX_ARTICULATION; ar++)
	{
		// if articulation is in low-pass zone (lower end of range)
		if (ar <= arThresholdLP_)
		{
			// Articulation time is longer the farther it is from the middle of the range, calculated on logarithmic scale
			// articTime = (minArticTime_ + powf(maxArticTime_ - minArticTime_, float(arThresholdLP_ - ar)/float(arThresholdLP_))) * 0.001;
			articTime = (minArticTime_ + ((maxArticTime_ - minArticTime_) * float(arThresholdLP_ - ar)/float(arThresholdLP_))) * 0.001;
			// for low-pass, BaseFc calculated to get from minFc to maxFc
			articToBaseFcTable_[ar] = powf((maxFc_ - minFc_), 1 / (sampleRate_ * articTime));
		}
		// if articulation is in high-pass zone (higher end of range)
		else if (ar >= arThresholdHP_)
		{
			// Articulation time is longer the farther it is from the middle of the range, calculated on logarithmic scale
			// articTime = (minArticTime_ + powf(maxArticTime_ - minArticTime_, float(ar - arThresholdHP_)/float(MAX_ARTICULATION - arThresholdHP_))) * 0.001;
			articTime = (minArticTime_ + ((maxArticTime_ - minArticTime_) * float(ar - arThresholdHP_)/float(MAX_ARTICULATION - arThresholdHP_))) * 0.001;
			// for low-pass, BaseFc calculated to get from maxFc to minFc (inverted)
			articToBaseFcTable_[ar] = 1 / powf((maxFc_ - minFc_), 1 / (sampleRate_ * articTime));
		}
		// otherwise, we are in all-pass zone (middle of range). These values can be anything, won't be used.
		else
		{
			articToBaseFcTable_[ar] = 0;
		}
	}
}

// Set sample rate, accordingly changes sampleRate of Filter object
void Articulation::setSampleRate(float frequency)
{
	sampleRate_ = frequency;
	articuFilter_.setSampleRate(sampleRate_);
}

// Set frequency, used to set it to that of current note
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

// Toggle enable for advanced controls
void Articulation::setAdvMode(bool advMode)
{
	advMode_ = advMode;
	
	// Set default Q value if not in advanced mode
	if (!advMode_)
	{
		filterQ_ = 1.0;
	}
}

// Set advanced controls for articulation, filter Q
void Articulation::setAdvControls(float q)
{
	if (advMode_)
	{
		filterQ_ = q;
	}
}

// Change how the filter Fc changes according to new articulation
void Articulation::updateArticulation(int articulation)
{
	// if articulation hasn't actualy changed, don't bother running calculations
	if (articulation_ == articulation)
		return;
	
	// Update articulation
	articulation_ = articulation;
	
	// Update slope
	baseFc_ = articToBaseFcTable_[articulation_];
	
	//check articulation_ if low-pass
	if (articulation_ <= arThresholdLP_)
	{
		filterType_ = kLowPass;
		allPass_ = false;
	}
	//check articulation_ if high-pass
	else if (articulation_ >= arThresholdHP_)
	{
		filterType_ = kHighPass;
		allPass_ = false;
	}
	// otherwise we must be in the all-pass zone
	else
		allPass_ = true;
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
	articuFilter_.setFilterParams(filterFc_, filterQ_, filterType_);
	
	// Apply filter
	return articuFilter_.process(sampleIn);
}

// calculate Articulation graph and send it to the GUI
void Articulation::updateFcGraph(Gui& gui, int bufferId)
{
	// if we are in allPass mode, graph horizontal line at y=1
	if (allPass_)
	{
		gui.sendBuffer(bufferId, 1);
		return;
	}
	
	//otherwise, calculate the articulation curve as an array of y-values
	for (int i = 0; i < ARTICULATION_GRAPH_N; i++)
	{
		// graph Fc change over the maximum artiulation time
		// in order to work with the base factor, the time must be in units of audio frames
		float tt = ((float(i) / ARTICULATION_GRAPH_N) * maxArticTime_ * 0.001) * sampleRate_;
		if (filterType_ == kLowPass)
		{
			float fc = pow(baseFc_, tt);
			if (fc < 20000)
				// divide by 20000 to keep between 0 and 1
				fcGraph_[i] = fc * 0.00005;
			else
				// if Fc >= 20000 set y = 1
				fcGraph_[i] = 1;
		}
		else if (filterType_ == kHighPass)
			// don't need to divide by 20000 since it's already between 0 and 1 
			fcGraph_[i] = pow(baseFc_, tt);
	}

	// send calculated graph buffer to the gui.
	gui.sendBuffer(bufferId, fcGraph_, ARTICULATION_GRAPH_N);
}