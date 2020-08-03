/***** note.h *****/
#ifndef NOTE_H
#define NOTE_H

#include <libraries/Midi/Midi.h>
#include <cmath>
#include "spectrum.h"
#include "brightness.h"
#include "articulation.h"
#include "envelope.h"

#define NUM_MIDI_NOTES 128

class Note
{
public:
	// Constructor
	Note();
	
	// Constructor specifying sample rate
	Note(float sampleRate, float frequency);
	
	void setSampleRate(float f);
	
	bool initMidi();
	
	// Getters
	int spectrum();
	int brightness();
	int articulation();
	int envelope();
	
	// Set timbre parameters
	void setSpectrum(int spectrum);
	void setBrightness(int brightness);
	void setArticulation(int articulation);
	void setEnvelope(int envelope);
	
	void setFrequency(float frequency);

	// play
	float process(bool noteOn);
	
	// dev tools
	float squareWaveDev();
	void printTimbreParameters();
	
private:
	float sampleRate_;
	float frequency_;
	bool noteOn_;
	
	// Timbre Dimensions
	Spectrum spectrum_;
	Brightness brightness_;
	Articulation articulation_;
	Envelope envelope_;
	
	// Device for handling MIDI messages
	Midi midi_;
	const char* midiPort0_ = "hw:1,0,0";
	// const char* gMidiPort1 = "hw:1,0,1";
	bool midiNoteOn_;

	float midiToFreqTable_[NUM_MIDI_NOTES] = {0};
	
	//dev tools
	int frameCount_;
	float framePeriod_;
};

#endif