/***** env_adsr.cpp *****/
#include <cmath>
#include "envelope.h"

// Constructor
Envelope::Envelope() : Envelope(44100.0) {}

// Constructor specifying a sample rate
Envelope::Envelope(float sampleRate)
{
	noteOn_ = false;
	sampleRate_ = sampleRate;
	envAdsr_ = Adsr(sampleRate_);
	
	// set duration to constant
	setDuration(1);
	cdMinAttackTime_ = 5;
	cdMaxAttackTime_ = 291;
	cdMinDecayTime_ = 5;
	cdMaxDecayTime_ = 291;
	
	sustainThresholdRatio_ = 0.4;
	sustainThreshold_ = int(sustainThresholdRatio_ * MAX_ENVELOPE);
	
	initEnvelopeTables();
	
	envelope_ = MAX_ENVELOPE / 2;
	updateEnvelope(MAX_ENVELOPE / 2);
}

void Envelope::initEnvelopeTables()
{
	envToAttackTable_.reserve(MAX_ENVELOPE);
	envToDecayTable_.reserve(MAX_ENVELOPE);
	for (unsigned int env = 0; env < MAX_ENVELOPE; env++)
	{
		envToAttackTable_[env] = (cdMinAttackTime_ + powf(cdMaxAttackTime_ - cdMinAttackTime_, float(env)/float(MAX_ENVELOPE))) * 0.001;
		envToDecayTable_[env] = (cdMaxDecayTime_ - powf(cdMaxDecayTime_ - cdMinDecayTime_, float(env)/float(MAX_ENVELOPE))) * 0.001;
	}
}

void Envelope::setSampleRate(float f)
{
	sampleRate_ = f;
	envAdsr_.setSampleRate(sampleRate_);
}

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

bool Envelope::isNoteOn()
{
	return noteOn_;
}

void Envelope::setDuration(float duration)
{
	duration_ = duration;
	if (duration_ != 0)
		constantDuration_ = true;
	else
		constantDuration_ = false;
		
	envAdsr_.setDuration(duration_);
}

void Envelope::updateEnvelope(int envelope)
{
	if (envelope_ == envelope)
		return;
	envelope_ = envelope;
	
	envAdsr_.setAttack(envToAttackTable_[envelope_]);
	envAdsr_.setDecay(envToDecayTable_[envelope_]);
	
	if (envelope >= sustainThreshold_)
	{
		setDuration(0);
		envAdsr_.setSustain(0.9);
	}
	else
	{
		setDuration(1);
	}

}

float Envelope::process(bool noteOn)
{
	float amplitude = envAdsr_.process(noteOn);
	int state = envAdsr_.getState();
	if (state != kADSRStateOff && state != kADSRStateOffDebounce && state != kADSRStateButtonHeldOff)
		noteOn_ = true;
	else
		noteOn_= false;
	
	return amplitude;
}