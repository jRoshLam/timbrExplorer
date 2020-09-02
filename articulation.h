/***** articulation.h *****/
#ifndef ARTICULATION_H
#define ARTICULATION_H

#include <libraries/Gui/Gui.h>
#include <vector>
#include "filter.h"

#define MAX_ARTICULATION 256
#define ARTICULATION_GRAPH_N 60

class Articulation
{
public:
	// Constructor
	Articulation();
	
	// Constructor with custom sampleRate
	Articulation(float sampleRate);
	
	// Set sample rate
	void setSampleRate(float frequency);
	
	// Set frequency of note (Fc is relative to this frequency)
	void setFrequency(float frequency);
	
	// Initialize lookup table matching articulation to baseFc
	void initArticulationTable();
	
	// reset articulation to starting Fc
	void reset();
	
	// Getters
	int getArticulation();
	float getSampleRate();
	int getFilterType();
	float getBaseFc();
	float getFilterFc();
	
	// Toggle enable for advanced controls
	void setAdvMode(bool advMode);
	
	// Set ADvanced Controls: Filter Q
	void setAdvControls(float q);
	
	// Change how the filter Fc changes
	void updateArticulation(int articulation);
	
	// Update Fc and apply filter
	float process(float sampleIn);
	
	// calculate graph
	void updateFcGraph(Gui& gui, int bufferId);
	
private:

	Filter articuFilter_; // Filter object
	
	// Lookup table to convert articulation to pre-calculated baseFc
	std::vector<float> articToBaseFcTable_;
	
	// Current articulation value and thresholds for low pass and high pass behavior
	int articulation_, arThresholdLP_, arThresholdHP_;
	
	// boolean for toggling advanced mode
	bool advMode_;

	// articulation graph y-values
	float fcGraph_[ARTICULATION_GRAPH_N] = {0};
	
	float frequency_; // frequency of current note
	
	// all-pass variables
	bool allPass_; // whether or not articulation is in all-pass zone
	float allPassZone_; // size of all-pass dead zone
	
	// parameters for how Fc changes
	float minArticTime_, maxArticTime_; // min and max times for articulation to end
	float baseFc_; // exponential base to multiply deltaFc by each frame
	float deltaFc_; // exponentially accumulated value added to min or max Fc and note frequency
	float minFc_, maxFc_; // minimum and maximum cut off frequency
	
	// Filter parameters
	int filterType_; // type of filter, low-pass or high-pass
	float sampleRate_, filterQ_; // sample rate and Q-factor
	// low pass:  Fc = noteFrequency + (minFc + deltaFc)
	// high pass: Fc = noteFrequency + (maxFc - deltaFc)
	float filterFc_; // cut off frequency 
	
};

#endif