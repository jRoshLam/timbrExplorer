/***** envelope.h *****/
#ifndef ADSR_H
#define ADSR_H

#include <Bela.h>

// Enumerator for ADSR states
enum {
	kADSRStateOff = 0,
	kADSRStateAttack,
	kADSRStateDecay,
	kADSRStateSustain,
	kADSRStateRelease,
	kADSRStateReleaseDebounce,
	kADSRStateButtonHeldOff,
	kADSRStateOffDebounce,
};

class Adsr
{
public:

	// Constructor
	Adsr();
	
	// Constructor specifying a sample rate
	Adsr(float sampleRate);
	
	// Set the sample rate, used for all calculations
	void setSampleRate(float frequency);
	
	// Set Attack
	void setAttack(float attackms);
	
	// Set Decay
	void setDecay(float decayms);
	
	// Set Sustain
	void setSustain(float sustainlvl);
	
	// Set Release
	void setRelease(float releasems);
	
	//set fixed duration (if 0, duration is not fixed)
	//fixed duration sounds have no sustain
	void setDuration(float duration);
	
	//return the current state of the ADSR
	int getState();
	
	//getters
	float getAttack();
	float getDecay();
	float getSustain();
	float getRelease();
	
	// Get current Envelope Amplitude
	float process(bool noteOn);

private:
	
	int currentState_; // current enumerated ADSR state
	float adsrLevel_; // current output level of ADSR
	float adsrIncrement_; // Current amount to incremenet level by
	int debounceCounter_; // frame counter for debounce timing
	int debounceInterval_; // debounce interval in number of frames
	
	float sampleRate_;
	// ADSR
	float attackTime_, decayTime_, sustainlvl_, releaseTime_;
	// inverse variables so we can use multiply instead of division (faster)
	float invAttackSamples_, invDecaySamples_, invReleaseSamples_;
	float duration_; // determines whether or not we have a fixed duration (0 sustain)
};

#endif