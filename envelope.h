/***** envelope.h *****/
#ifndef ENVELOPE_H
#define ENVELOPE_H

#include <vector>
#include <libraries/Gui/Gui.h>
#include "adsr.h"

#define MAX_ENVELOPE 256

class Envelope
{
public:
	// Constructor
	Envelope();
	
	// Constructor specifying sample rate
	Envelope(float sampleRate);
	
	// set sample rate of object to that of the project
	void setSampleRate(float frequency);
	
	// Initialize look up tables to convert envelope to attack and decay times
	void initEnvelopeTables();
	
	// Getters
	int getEnvelope();
	float getAttackTime();
	float getDecayTime();
	
	// Set the duration of the envelope
	void setDuration(float duration);
	
	// Toggle enable for advanced controls
	void setAdvMode(bool advMode);
	
	// Set Advanced Controls: Decay, Sustain, Release
	void setAdvControls(float decay, float sustain, float release);
	
	// update envelope based on new envelope parameter
	void updateEnvelope(int envelope);
	
	// whether or not envelope is on
	bool isNoteOn();
	
	// retrieve the current value of the envelope
	float process(bool noteOn);
	
	void sendToGui(Gui& gui, int bufferId);
	
private:
	// ADSR object
	Adsr envAdsr_;
	// Graph of ADSR, alternating x,y,x,y,x,y...
	float adsrGraph_[10];
	
	// Envelope Timbre dimension value
	int envelope_;
	// boolean for toggling advanced mode
	bool advMode_;
	
	// lookup tables to convert envelope to attack and decay times
	std::vector<float> envToAttackTable_;
	std::vector<float> envToDecayTable_;
	
	// default sustain level
	float sustain_;
	
	float sampleRate_;
	
	// Parameters to determine if notes are impulsive or sustained
	float sustainThresholdRatio_; // threshold as a ratio of the envelope range 
	int sustainThreshold_; // sustain ratio as an envelope value
	
	bool noteOn_; // Whether or not note is on (ADSR state not off)
	bool constantDuration_; // bool whether or not there is a sustain
	// minimum and maximum attack and decay times in milliseconds
	float minAttackTime_, maxAttackTime_, minDecayTime_,maxDecayTime_;
};

#endif