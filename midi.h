/***** midi.h *****/
#include <libraries/Midi/Midi.h>

bool initMidi();

// check for available midi messages
// If NoteOn, trigger new sound
void checkMidi();