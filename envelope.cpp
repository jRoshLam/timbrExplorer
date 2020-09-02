/***** env_adsr.cpp *****/
#include <cmath>
#include "envelope.h"

// Constructor
Envelope::Envelope() : Envelope(44100.0) {}

// Constructor specifying a sample rate
Envelope::Envelope(float sampleRate)
{
	sampleRate_ = sampleRate;
	
	// initialize ADSR object
	// Release time fixed at default value (10 ms))
	envAdsr_ = Adsr(sampleRate_);
	
	// initial values for the graph (always starts at 0,0)
	adsrGraph_[0] = 0; // 0,0
	adsrGraph_[1] = 0; // 0,1
	adsrGraph_[3] = 1; // 1,1
	adsrGraph_[5] = envAdsr_.getSustain(); // 2,1
	adsrGraph_[6] = 0.99; // 3,0 release is constant at 10ms
	adsrGraph_[7] = envAdsr_.getSustain(); // 3,1 sustain level
	adsrGraph_[8] = 1; // 4,0 end
	adsrGraph_[9] = 0; // 4,1
	
	// Note starts as off
	noteOn_ = false;
	
	// set duration to constant
	setDuration(1);
	minAttackTime_ = 5;
	maxAttackTime_ = 291;
	minDecayTime_ = 5;
	maxDecayTime_ = 291;
	sustain_ = 0.9;
	
	// bottom 40% of envelope range - no sustain
	sustainThresholdRatio_ = 0.4;
	sustainThreshold_ = int(sustainThresholdRatio_ * MAX_ENVELOPE);
	
	// initialize lookup tables
	initEnvelopeTables();
	
	// default envelope 
	envelope_ = 0;
	updateEnvelope(MAX_ENVELOPE / 2);
}

// initialize lookup tables to convert envelope values to attack and decay values
void Envelope::initEnvelopeTables()
{
	// reserve space for lookup tables
	envToAttackTable_.reserve(MAX_ENVELOPE);
	envToDecayTable_.reserve(MAX_ENVELOPE);
	for (unsigned int env = 0; env < MAX_ENVELOPE; env++)
	{
		// attack and decay are inversely proportional to each other and have a logarithmic relation to envelope
		envToAttackTable_[env] = (minAttackTime_ + powf(maxAttackTime_ - minAttackTime_, float(env)/float(MAX_ENVELOPE))) * 0.001;
		envToDecayTable_[env] = (maxDecayTime_ - powf(maxDecayTime_ - minDecayTime_, float(env)/float(MAX_ENVELOPE))) * 0.001;
	}
}

// set sample rate of of object and ADSR object
void Envelope::setSampleRate(float f)
{
	sampleRate_ = f;
	envAdsr_.setSampleRate(sampleRate_);
}

// Getters
int Envelope::getEnvelope()
{
	return envelope_;
}
float Envelope::getAttackTime()
{
	return envToAttackTable_[envelope_];
}
float Envelope::getDecayTime()
{
	return envToDecayTable_[envelope_];
}

// Returns whether or not note is currently on (ADSR not off)
bool Envelope::isNoteOn()
{
	return noteOn_;
}

// Set envelope to a constant duration (no sustain)
void Envelope::setDuration(float duration)
{
	if (duration != 0)
		constantDuration_ = true;
	else
		constantDuration_ = false;
		
	envAdsr_.setDuration(duration);
}

// Toggle enable for advanced controls
void Envelope::setAdvMode(bool advMode)
{
	advMode_ = advMode;
	if (!advMode_)
	{
		// retrieve decay time from lookup table
		envAdsr_.setDecay(envToDecayTable_[envelope_]);
		adsrGraph_[4] = envToAttackTable_[envelope_] + envToDecayTable_[envelope_]; // 2,0
		
		// check if we should enable sustain
		if (envelope_ >= sustainThreshold_)
		{
			// not constant duration, sustain fixed at 0.9
			setDuration(0);
			envAdsr_.setSustain(sustain_);
			adsrGraph_[5] = sustain_; // sustain level
			adsrGraph_[7] = sustain_; // sustain level
		}
		else
		{
			// constant duration (will set sustain to 0)
			setDuration(1);
			adsrGraph_[5] = 0; // sustain level
			adsrGraph_[7] = 0; // sustain level
		}
		
		// default release
		envAdsr_.setRelease(0.01);
		adsrGraph_[6] = 0.99;
		
	}
}

//set advanced parameters of the ADSR: decay, sustain, and release
void Envelope::setAdvControls(float decay, float sustain, float release)
{
	// If not in advanced mode, exit function
	if (!advMode_)
		return;
	
	// update both ADSR object and graph buffer
	// Update decay
	envAdsr_.setDecay(decay);
	adsrGraph_[4] = envAdsr_.getAttack() + decay;
	
	// Update sustain
	envAdsr_.setSustain(sustain);
	adsrGraph_[5] = sustain;
	adsrGraph_[7] = sustain;
	
	// Update release
	envAdsr_.setRelease(release);
	adsrGraph_[6] = 1 - release;
	
}

// Update attack and decay values based on new envelope value
void Envelope::updateEnvelope(int envelope)
{
	// If envelope hasn't changed, skip update
	if (envelope_ == envelope)
		return;

	envelope_ = envelope;
	
	// retrieve attack time from lookup table
	envAdsr_.setAttack(envToAttackTable_[envelope_]);
	adsrGraph_[2] = envToAttackTable_[envelope_]; // 1,0
	
	// default (non-advanced) behavior for decay, sustain, and release
	// Update both ADSR object and graph buffer
	if (!advMode_)
	{
		// retrieve decay time from lookup table
		envAdsr_.setDecay(envToDecayTable_[envelope_]);
		adsrGraph_[4] = envToAttackTable_[envelope_] + envToDecayTable_[envelope_]; // 2,0
		
		// check if we should enable sustain
		if (envelope_ >= sustainThreshold_)
		{
			// not constant duration, sustain fixed at 0.9
			setDuration(0);
			envAdsr_.setSustain(sustain_);
			adsrGraph_[5] = sustain_; // sustain level
			adsrGraph_[7] = sustain_; // sustain level
		}
		else
		{
			// constant duration (will set sustain to 0)
			setDuration(1);
			adsrGraph_[5] = 0; // sustain level
			adsrGraph_[7] = 0; // sustain level
		}
	}

}

// Run ADSR process and check ADSR state to update noteOn_ variable
float Envelope::process(bool noteOn)
{
	// update and retrieve ADSR level
	float amplitude = envAdsr_.process(noteOn);
	
	// get current state to update noteOn_ member
	int state = envAdsr_.getState();
	if (state != kADSRStateOff && state != kADSRStateOffDebounce && state != kADSRStateButtonHeldOff)
		noteOn_ = true;
	else
		noteOn_= false;
	
	// return ADSR level
	return amplitude;
}

// send ADSR graph buffer to the GUI
void Envelope::sendToGui(Gui& gui, int bufferId)
{
	gui.sendBuffer(bufferId, adsrGraph_);
}