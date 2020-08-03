/***** envelope.h *****/
#ifndef ENVELOPE_H
#define ENVELOPE_H

#include <vector>
#include "adsr.h"

#define MAX_ENVELOPE 256

class Envelope
{
public:
	// Constructor
	Envelope();
	
	// Constructor specifying sample rate
	Envelope(float sampleRate);
	
	void setSampleRate(float f);
	
	// Initialize look up tables to convert envelops to attack and decay times
	void initEnvelopeTables();
	
	int getEnvelope();
	float getAttackTime();
	float getDecayTime();
	
	// Set the duration of the envelope
	void setDuration(float duration);
	
	// update envelope based on new envelope parameter
	void updateEnvelope(int envelope);
	
	// whether or not envelope is on
	bool isNoteOn();
	
	// retrieve the current value of the envelope
	float process(bool noteOn);
	
private:
	Adsr envAdsr_;
	
	int envelope_;
	std::vector<float> envToAttackTable_;
	std::vector<float> envToDecayTable_;
	float sampleRate_;
	
	float sustainThresholdRatio_;
	int sustainThreshold_;
	
	bool noteOn_;
	bool constantDuration_;
	float duration_;
	float cdMinAttackTime_, cdMaxAttackTime_, cdMinDecayTime_,cdMaxDecayTime_;
};

#endif