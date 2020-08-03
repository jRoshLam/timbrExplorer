/***** note.cpp *****/
#include <Bela.h>
#include "note.h"

// Constructor
Note::Note() : Note(44100.0, 440.0) {}

// Constructor specifying a sample rate
Note::Note(float sampleRate, float frequency) :
spectrum_(sampleRate, frequency)
{
	sampleRate_ = sampleRate;
	frequency_ = frequency;
	frameCount_ = 0;
	framePeriod_ = int(sampleRate / frequency_);
	noteOn_ = false;

	brightness_ = Brightness(sampleRate_, frequency_);
	articulation_ = Articulation(sampleRate_);
	envelope_ = Envelope(sampleRate_);
}

void Note::setSampleRate(float sampleRate)
{
	sampleRate_ = sampleRate;
	framePeriod_ = int(sampleRate / frequency_);
	
	// spectrum_.setSampleRate();
}

// CALL IN SETUP
bool Note::initMidi()
{
	//TODO: ensure valid midi port
	
	// Initialize MIDI object
	if(midi_.readFrom(midiPort0_) < 0) {
		rt_printf("Unable to read from MIDI port %s\n", midiPort0_);
		return false;
	}
	midi_.writeTo(midiPort0_);
	midi_.enableParser(true);
	
	// Initialize midi frequency table
	for (int i = 0; i < NUM_MIDI_NOTES; i++)
		midiToFreqTable_[i] = powf(2.0, (i - 69)/12.0) * 440.0;
	
	// int i = 60;
	// rt_printf("MIDI note %d frequency: %f, should be %f\n", i, midiToFreqTable_[i], powf(2.0, (i - 69)/12.0) * 440.0);
		
	return true;
}

// Getters
int Note::spectrum()
{
	return spectrum_.getSpectrum();
}
int Note::brightness()
{
	return brightness_.getBrightness();
}
int Note::articulation()
{
	return articulation_.getArticulation();
}
int Note::envelope()
{
	return envelope_.getEnvelope();
}

void Note::setSpectrum(int spectrum)
{
	spectrum_.updateSpectrum(spectrum);
}
void Note::setBrightness(int brightness)
{
	brightness_.updateBrightness(brightness);
}
void Note::setArticulation(int articulation)
{
	articulation_.updateArticulation(articulation);
}
void Note::setEnvelope(int envelope)
{
	envelope_.updateEnvelope(envelope);
}

void Note::setFrequency(float frequency)
{
	if (frequency_ == frequency)
		return;
	frequency_ = frequency;
	
	spectrum_.setFrequency(frequency_);
	brightness_.setFrequency(frequency_);
}

float Note::process(bool noteOn)
{
	// Check MIDI messages
	while (midi_.getParser()->numAvailableMessages() > 0)
	{
		MidiChannelMessage message;
		message = midi_.getParser()->getNextChannelMessage();
		// message.prettyPrint();		// Print the message data
		
		// A MIDI "note on" message type might actually hold a real
		// note onset (e.g. key press), or it might hold a note off (key release).
		// The latter is signified by a velocity of 0.
		if(message.getType() == kmmNoteOn) {
			message.prettyPrint();
			int noteNumber = message.getDataByte(0);
			int velocity = message.getDataByte(1);
			float noteFrequency = midiToFreqTable_[noteNumber];
			
			// Velocity of 0 is really a note off
			if (velocity == 0 && noteFrequency == frequency_)
			{
				midiNoteOn_ = false;
			}
			else if (!noteOn_)
			{
				setFrequency(noteFrequency);
				midiNoteOn_ = true;
			}
			
			// rt_printf("Frequency: %f, Amplitude: %f\n", gOscillator.frequency(), gAmplitude);
		}
		else if(message.getType() == kmmNoteOff) {
			// We can also encounter the "note off" message type which is the same
			// as "note on" with a velocity of 0.
			int noteNumber = message.getDataByte(0);
			float noteFrequency = midiToFreqTable_[noteNumber];
			if (noteFrequency == frequency_) {
				midiNoteOn_ = false;
			}
		}
	}
	
	// check envelope to see if we are producing sound
	float amplitude = envelope_.process(midiNoteOn_);
	
	// float amplitude = envelope_.process(noteOn);
	
	float out = 0;
	
	// if note just turned on
	if (!noteOn_ && envelope_.isNoteOn())
	{
		//reset as necessary
		// spectrum_.reset();
		brightness_.reset();
		articulation_.reset();
	}
	
	// even if input noteOn is off, the envelope may still be playing
	noteOn_ = envelope_.isNoteOn();

	// obtain waveform value
	if (noteOn_)
	{
		// out = squareWaveDev();
		// Get basic FM-generated waveform
		out = spectrum_.process();
		
		// apply brightness filter
		out = brightness_.process(out);
		
		// apply articulation filter
		out = articulation_.process(out);
		
		// apply volume envelope
		out = out * amplitude;
	}
	
	return out;
}


float Note::squareWaveDev()
{
	float out;
	if (frameCount_ < framePeriod_ / 2)
		out = 0;
	else
		out = 1;

	frameCount_++;
	if (frameCount_ > framePeriod_)
		frameCount_ = 0;
		
	return out;
}

// debug statements
void Note::printTimbreParameters()
{
	// rt_printf("test_print1test_print1test_print1test_print1test_print1test_print1 %f %f\n", 1.00, 2.00);
	// rt_printf("test_print2test_print2test_print2test_print2test_print2test_print2 %f %f\n", 3.00, 4.00);
	
	// rt_printf("Brightness filter Fc: %f, Filter Q: %f\n", 1.00, 2.00);
	// rt_printf("Envelope Attack Time: %f, Decay Time: %f\n", 3.00, 4.00);
	
	// rt_printf("Brightness filter Fc: %f, Filter Q: %f\n", brightness_.getFc(), brightness_.getQ());
	// rt_printf("Envelope Attack Time: %f, Decay Time: %f\n", envelope_.getAttackTime(), envelope_.getDecayTime());
	
	// float minArticTime_ = 5;
	// float maxArticTime_ = 600;
	// int arThresholdHP_ = 140;
	// int ar = 170;
	// float articTime = articTime = (minArticTime_ + powf(maxArticTime_ - minArticTime_, float(ar - arThresholdHP_)/float(MAX_ARTICULATION - arThresholdHP_))) * 0.001;
	// rt_printf("articTime is %f, should be 0.0102\n", articTime);
	// float maxFc_ = 20000;
	// float minFc_ = 50;
	// float baseFc = 1 / powf((maxFc_ - minFc_), 1 / (sampleRate_ * articTime));
	// rt_printf("baseFc is %f, should be .97823\n", baseFc);
	// rt_printf("Articulation sampleRate: %f, filterType: %d, baseFc: %f, filterFc: %f\n", 
	// 	articulation_.getSampleRate(), articulation_.getFilterType(), articulation_.getBaseFc(), articulation_.getFilterFc());

	rt_printf("Operator tableLength: %f, curent table: %f\n",
		spectrum_.getDebugOperator().tableLength(), spectrum_.getDebugOperator().debugValue());
	// rt_printf("sine wave tableLength: %d, test value: %f\n",
	// 	spectrum_.getWaveTableSize(), spectrum_.getDebugWaveValue());
}