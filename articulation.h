/***** articulation.h *****/
#ifndef ARTICULATION_H
#define ARTICULATION_H

#include <vector>
#include "filter.h"

#define MAX_ARTICULATION 256

class Articulation
{
public:
	// Constructor
	Articulation();
	
	// Constructor with custom sampleRate
	Articulation(float sampleRate);
	
	void setSampleRate(float f);
	
	void setFrequency(float frequency);
	
	void initArticulationTable();
	
	void reset();
	
	// Getters
	int getArticulation();
	float getSampleRate();
	int getFilterType();
	float getBaseFc();
	float getFilterFc();
	
	// Change how the filter Fc changes
	void updateArticulation(int articulation);
	
	// Update Fc and apply filter
	float process(float sampleIn);
	
private:
	Filter articuFilter_;
	
	std::vector<float> articToBaseFcTable_;
	
	int articulation_, arThresholdLP_, arThresholdHP_;
	
	float frequency_;
	
	// all-pass variables
	bool allPass_;
	float allPassZone_;
	
	// filter slope details
	float minArticTime_, maxArticTime_;
	float baseFc_, minFc_, maxFc_, deltaFc_;
	
	// Filter parameters
	int filterType_;
	float sampleRate_, filterQ_, filterFc_;
	
};

#endif