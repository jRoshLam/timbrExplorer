/***** envelope.h *****/
#ifndef ADSR_H
#define ADSR_H

#include <Bela.h>

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
	// bool testVar = false;

	// Constructor
	Adsr();
	
	// Constructor specifying a sample rate
	Adsr(float sampleRate);
	
	// Set the sample rate, used for all calculations
	void setSampleRate(float rate);
	
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
	
	// Get current Envelope Amplitude
	float process(bool noteOn);
	
	

private:
	
	int currentState_;
	float adsrLevel_;
	float adsrIncrement_;
	int debounceCounter_;
	int debounceInterval_;
	
	float sampleRate_;
	float attackTime_, decayTime_, sustainlvl_, releaseTime_;
	float invAttackSamples_, invDecaySamples_, invReleaseSamples_;
	float duration_;
};

#endif