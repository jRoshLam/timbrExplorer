/***** midi.cpp *****/
#include <cmath>
#include "midi.h"

// Device for handling MIDI messages
Midi gMidi;
const char* gMidiPort0 = "hw:1,0,0";
const char* gMidiPort1 = "hw:1,0,1";
const int NUM_MIDI_NOTES = 128;
float midiToFreqTable[NUM_MIDI_NOTES] = {0};

bool initMidi()
{
	// Initialise the MIDI device
	if(gMidi.readFrom(gMidiPort0) < 0) {
		rt_printf("Unable to read from MIDI port %s\n", gMidiPort0);
		return false;
	}
	gMidi.writeTo(gMidiPort0);
	gMidi.enableParser(true);
	
	// Initialize midi frequency table
	for (unsigned int i = 0; i < NUM_MIDI_NOTES; i++)
		midiToFreqTable[0] = powf(2.0, (i - 69)/12.0) * 440.0;
		
	return true;
}

void checkMidi()
{
	while (gMidi.getParser()->numAvailableMessages() > 0)
	{
		MidiChannelMessage message;
		message = gMidi.getParser()->getNextChannelMessage();
		message.prettyPrint();		// Print the message data
		
		// A MIDI "note on" message type might actually hold a real
		// note onset (e.g. key press), or it might hold a note off (key release).
		// The latter is signified by a velocity of 0.
		if(message.getType() == kmmNoteOn) {
			int noteNumber = message.getDataByte(0);
			int velocity = message.getDataByte(1);
			
			// Velocity of 0 is really a note off
			if(velocity == 0) {
				// noteOff(noteNumber);
			}
			else {
				// noteOn(noteNumber, velocity);
			}
			
			// rt_printf("Frequency: %f, Amplitude: %f\n", gOscillator.frequency(), gAmplitude);
		}
		else if(message.getType() == kmmNoteOff) {
			// We can also encounter the "note off" message type which is the same
			// as "note on" with a velocity of 0.
			int noteNumber = message.getDataByte(0);
			
			// noteOff(noteNumber);
		}
	}
}