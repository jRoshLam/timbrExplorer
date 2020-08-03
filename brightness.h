/***** brightness.h *****/
#ifndef BRIGHTNESS_H
#define BRIGHTNESS_H

#include <vector>
#include "filter.h"

#define FILTER_ORDER 2
#define NUMBER_OF_FILTERS FILTER_ORDER/2
#define MAX_BRIGHTNESS 256

class Brightness
{
public:
	// Constructor
	Brightness();
	
	// Constructor specifying sample rate
	Brightness(float sampleRate, float frequency);
	
	void setSampleRate(float f);
	
	void initBrightnessTable();
	
	int getBrightness();
	float getFc();
	float getQ();
	
	// reset
	void reset();
	
	void setFrequency(float frequency);
	
	// update brightness filter
	void updateBrightness(int brightness);
	
	// Apply fitler to input sample
	float process(float sampleIn);
	
private:
	// Filter array
	Filter brFilters_[NUMBER_OF_FILTERS];
	
	std::vector<float> brightToFcTable_;
	// float brightToFcTable_[MAX_BRIGHTNESS];
	
	// current Brightness Value and brightness thresholds
	int brightness_, brThresholdLP_, brThresholdHP_;
	
	// whether or not brightness filter should just be an all pass
	bool allPass_;
	float allPassZone_;
	
	// Filter parameters
	float frequency_;
	float sampleRate_, filterType_, filterQ_, filterFc_;
	
	// Minimum and Maximum Fc values
	float minLPFc_, maxLPFc_, minHPFc_, maxHPFc_;
};

#endif