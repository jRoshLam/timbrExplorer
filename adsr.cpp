/***** adsr.cpp *****/
/*
Envelope Function - envelope should always be constant duration?
Parameter has a logarithmic effect (McAdams 1995)
*/
#include "adsr.h"

#define DEBOUNCE_MS 20

// Constructor
Adsr::Adsr() : Adsr(44100.0) {}

// Constructor specifying a sample rate
Adsr::Adsr(float sampleRate)
{
	setSampleRate(sampleRate);

	// Set defaults
	duration_ = 0;
	attackTime_ = 0.01;
	decayTime_ = 0.01;
	sustainlvl_ = 0.5;
	releaseTime_ = 0.01;
	
	// ADSR is in off state to begin with
	adsrLevel_ = 0.0;
	currentState_ = kADSRStateOff;
	
	// Initialize Debounce counter and conditions
	debounceCounter_ = 0;
	debounceInterval_ = int(DEBOUNCE_MS * sampleRate_ / 1000.0); // 20 ms
}

// Set the sample rate, used for all calculations
void Adsr::setSampleRate(float frequency)
{
	sampleRate_ = frequency;	
}

// Set Attack
void Adsr::setAttack(float attackTime)
{
	attackTime_ = attackTime;
	invAttackSamples_ = 1 / (attackTime_ * sampleRate_);
}

// Set Decay
void Adsr::setDecay(float decayTime)
{
	decayTime_ = decayTime;
	invDecaySamples_ = 1 / (decayTime_ * sampleRate_);
}

// Set Sustain
void Adsr::setSustain(float sustainlvl)
{
	sustainlvl_ = sustainlvl;
}

// Set Release
void Adsr::setRelease(float releaseTime)
{
	releaseTime_ = releaseTime;
	invReleaseSamples_ = 1 / (releaseTime_ * sampleRate_);
}

// Return current state
int Adsr::getState()
{
	return currentState_;
}

//set fixed duration (if 0, duration is not fixed)
//fixed duration sounds have no sustain
void Adsr::setDuration(float duration)
{
	duration_ = duration;
	if (duration_ != 0)
	{
		sustainlvl_ = 0;
	}
	
}

//getters
float Adsr::getAttack()
{
	return attackTime_;
}
float Adsr::getDecay()
{
	return decayTime_;
}
float Adsr::getSustain()
{
	return sustainlvl_;
}
float Adsr::getRelease()
{
	return releaseTime_;
}

// update and return ADSR level. Manage state as necessary
// input: whether input trigger (ex. button, midi key) is currently pressed
float Adsr::process(bool noteOn)
{
	switch(currentState_)
	{
		case(kADSRStateOff): {
			// Action: do nothing (output 0)
			// Transition: look for note on to go to attack state
			if (noteOn) {
				currentState_ = kADSRStateAttack;
				adsrIncrement_ = (1.0 - adsrLevel_) / (attackTime_ * sampleRate_);
				// rt_printf("adsr state change, newState: %d || level: %f || incr: %f\n", currentState_, adsrLevel_, adsrIncrement_);
			}
			break;
		}
		case(kADSRStateAttack): {
			// Action: increment envelope until it reaches 1.0
			// Transition: check if envelope has reached 1.0 and change to decay
			if (adsrLevel_ >= 1.0) {
				adsrLevel_ = 1.0;
				currentState_ = kADSRStateDecay;
				adsrIncrement_ = (sustainlvl_ - 1.0) / (decayTime_ * sampleRate_);
				// rt_printf("adsr state change, newState: %d || level: %f || incr: %f\n", currentState_, adsrLevel_, adsrIncrement_);
			}
			adsrLevel_ += adsrIncrement_;
			break;
		}
		case(kADSRStateDecay): {
			// Action: decrement envelope until it reaches the sustain level
			// Transition: check if envelope has reached the sustain level and change to sustain
			if (adsrLevel_ <= sustainlvl_) {
				if (duration_ != 0 && noteOn) {
					currentState_ = kADSRStateButtonHeldOff;
				}
				else if (duration_ != 0 && !noteOn) {
					currentState_ = kADSRStateOffDebounce;
					debounceCounter_ = 0;
				}
				else 
					currentState_ = kADSRStateSustain;
				// rt_printf("adsr state change, newState: %d || level: %f || incr: %f\n", currentState_, adsrLevel_, adsrIncrement_);
			}
			adsrLevel_ += adsrIncrement_;
			break;
		}
		case(kADSRStateSustain): {
			// Action: hold a constant level (don't change envelope value)
			// Transition: look for note off and go to release state
			if (!noteOn) {
				currentState_ = kADSRStateReleaseDebounce;
				debounceCounter_ = 0;
				adsrIncrement_ = (0 - adsrLevel_) / (releaseTime_ * sampleRate_);
				// rt_printf("adsr state change, newState: %d || level: %f || incr: %f\n", currentState_, adsrLevel_, adsrIncrement_);
			}
			break;
		}
		case(kADSRStateReleaseDebounce): {
			// Action: decrement envelope until it returns to 0 (don't let it go below 0)
			// Transition: look for envelope <= 0 and go to off state
			// don't look for note on until done debouncing
			if (debounceCounter_ > debounceInterval_) {
				currentState_ = kADSRStateRelease;
				// rt_printf("adsr state change, newState: %d || level: %f || incr: %f\n", currentState_, adsrLevel_, adsrIncrement_);
			}
			if (adsrLevel_ <= 0) {
				adsrLevel_ = 0.0;
				currentState_ = kADSRStateOff;
				// rt_printf("adsr state change, newState: %d || level: %f || incr: %f\n", currentState_, adsrLevel_, adsrIncrement_);
			}
			adsrLevel_ += adsrIncrement_;
			debounceCounter_++;
			break;
		}
		case(kADSRStateRelease): {
			// Action: decrement envelope until it returns to 0 (don't let it go below 0)
			// Transition: look for envelope <= 0 and go to off state
			// Transition: look for note on and go back to attack state
			if (adsrLevel_ <= 0) {
				adsrLevel_ = 0.0;
				currentState_ = kADSRStateOff;
				// rt_printf("adsr state change, newState: %d || level: %f || incr: %f\n", currentState_, adsrLevel_, adsrIncrement_);
			}
			if (noteOn) {
				currentState_ = kADSRStateAttack;
				// rt_printf("adsr state change, newState: %d || level: %f || incr: %f\n", currentState_, adsrLevel_, adsrIncrement_);
				adsrIncrement_ = (1.0 - adsrLevel_) / (attackTime_ * sampleRate_);
			}
			adsrLevel_ += adsrIncrement_;
			break;
		}
		case(kADSRStateButtonHeldOff): {
			// Action: do nothing
			// Transition: look for noteOff, go to Off Debounce state
			if (!noteOn) {
				currentState_ = kADSRStateOffDebounce;
				// rt_printf("adsr state change, newState: %d || level: %f || incr: %f\n", currentState_, adsrLevel_, adsrIncrement_);
				debounceCounter_ = 0;
			}
			break;
		}
		case(kADSRStateOffDebounce): {
			// Action: do nothing
			// Transition: wait for debounce counter to expire, go to off state
			if (debounceCounter_ > debounceInterval_) 
				currentState_ = kADSRStateOff;
			debounceCounter_++;
			break;
		}
	}
	return adsrLevel_;
}