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
	
	// Set sample rate
	void setSampleRate(float frequency);
	
	// Initialize lookup table to convert from brightness to Fc
	void initBrightnessTable();
	
	// Getters
	int getBrightness();
	float getFc();
	float getQ();
	
	// reset Brightness filter
	void reset();
	
	// Set frequency of current note
	void setFrequency(float frequency);
	
	// Set frequency of current ntoe and resonance of filter
	void setMidiIn(float frequency, float qFactor);
	
	// update brightness filter
	void updateBrightness(int brightness);
	
	// Toggle enable for advanced controls
	void setAdvMode(bool advMode);
	
	// advanced control behavior
	void setAdvControls(float enable, float qFactor);
	
	// Apply fitler to input sample
	float process(float sampleIn);
	
	// update FRF graph, wrapper for Filter function
	void updateFrfGraph(Gui& gui, int bufferId);
	
private:
	// Filter array
	// As of right now, only one filter object is used. Should probably revert this from an array.
	Filter brFilters_[NUMBER_OF_FILTERS];

	// lookup table matching brightness to Fc
	std::vector<float> brightToFcTable_;
	
	// current Brightness Value and brightness thresholds
	int brightness_, brThresholdLP_, brThresholdHP_;
	
	// boolean for toggling advanced mode
	bool advMode_;
	bool midiLink_;
	
	// whether or not to apply brightness filter
	bool allPass_;
	// Size of all-pass dead zone at the center of the brightness range
	float allPassZone_;
	
	// Filter parameters
	float frequency_, velocityQ_;
	float sampleRate_, filterType_, filterQ_, filterFc_;
	
	// Minimum and Maximum Fc values for both low pass and high pass conditions
	float minLPFc_, maxLPFc_, minHPFc_, maxHPFc_;
};

#endif