/***** note.h *****/
#ifndef NOTE_H
#define NOTE_H

#include <Bela.h>
#include <libraries/Midi/Midi.h>
#include <libraries/ne10/NE10.h>
#include <libraries/Fft/Fft.h>
#include <cmath>
#include "spectrum.h"
#include "brightness.h"
#include "articulation.h"
#include "envelope.h"

#define NUM_MIDI_NOTES 128
#define FFT_BUFFER_N 1024
#define FFT_OUT_N FFT_BUFFER_N * 0.5
#define FFT_HOP_SIZE 4096

// number of simultaneous notes (polyphony)
#define NUM_VOICES 1

// enumerator to index bela to GUI buffers
enum belaToGuiBuffers {
	kBtGTimbreParams = 0,
	kBtGMidi,
	kBtGSpecFft,
	kBtGBrightFrf,
	kBtGArticulation,
	kBtGEnvelope,
	kBtGOutFft,
	kBtGFMAlg,
	kBtGFMRatios,
	kBtGFMAmps,
	kBtGFMShapes
};

// enumerator to index GUI to BELA buffers
enum guiToBelaBuffers {
	kGtBTimbreParams = 0,
	kGtBAdvControls,
	kGtBAdvSpectrum
};

// enumerator to index the contents of the advControls buffer
enum advControlIndicex {
	kACIAdvMode = 0,
	kACIBrMidiLink,
	kACIBrQ,
	kACIArQ,
	kACIDecay,
	kACISustain,
	kACIRelease,
	kACBufferSize
};


class Note
{
public:
	// Constructor
	Note();
	
	// Constructor specifying sample rate
	Note(float sampleRate, float frequency);
	
	// Destructor
	~Note();
	
	// Set sample rate
	void setSampleRate(float frequency);
	
	// initialize communication with MIDI device
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
	
	// Toggle enable for advanced controls
	void setAdvMode(float advMode);
	
	// Set advanced controls
	void setAdvControls(float* controlData);
	
	// update FM spectrum
	void updateAdvSpectrum(float* fmBuffer);
	
	// Set frequency of note and brightness q factor
	void setMidiIn(float frequency, float qFactor, int indx);

	// get next audio sample
	float process(Gui& gui, bool noteOn);
	
	// run fft on output
	// check if a buffer is full and an fft is ready to be run
	// check this every frame. If it returns true, schedule the auxiliary task
	bool checkFftReady();
	// Copy data from buffer and use NE10 to process
	void outputFft(Gui& gui);
	// Check and update Brightness and Articulation Graphs
	void updateGraphs(Gui& gui);
	// Send GUI information
	void sendToGui(Gui& gui);
	
	// dev tools
	float squareWaveDev(); // return a square wave
	void printTimbreParameters(); // print debug statements
	
private:
	float sampleRate_; // sample rate
	float frequency_; // note frequency
	float qFactor_; // brightness q factor
	float velocity_; // note's midi velocity (affects brightness resonance)
	bool noteOn_; // whether note is on
	
	// Timbre Dimensions
	Spectrum spectrum_;
	Brightness brightness_;
	Articulation articulation_;
	Envelope envelope_;
	
	// polyphony
	std::vector<float> frequencies_; // note frequency
	std::vector<float> qFactors_; // brightness q factor
	std::vector<float> velocities_; // note's midi velocity (affects brightness resonance)
	std::vector<bool> midiNoteOns_;
	std::vector<bool> noteOns_;
	std::vector<Spectrum> spectrums_;
	std::vector<Brightness> brightnesses_;
	std::vector<Articulation> articulations_;
	std::vector<Envelope> envelopes_;
	
	// boolean for toggling advanced mode
	bool advMode_;
	
	// Object for handling MIDI messages
	Midi midi_;
	// MIDI port
	//------------ CHANGE MIDI PORT HERE -----------------
	const char* midiPort0_ = "hw:1,0,0";
	//------------ CHANGE MIDI PORT HERE -----------------
	bool midiNoteOn_; // whether a MIDI key is depressed
	

	// table to convert MIDI numbers to frequencies
	float midiToFreqTable_[NUM_MIDI_NOTES] = {0};
	float velocityToQTable_[NUM_MIDI_NOTES] = {0};
	
	ne10_fft_cpx_float32_t* outFftNeInput_ = nullptr;
    ne10_fft_cpx_float32_t* outFftNeOutput_ = nullptr;
    ne10_fft_cfg_float32_t outFftcfg_;
    std::vector<float> outFftInputBuffer_;
	std::vector<float> outFftOutputBuffer_;
	std::vector<float> fftWindowBuffer_;
	int outFftWritePtr_;
	int outFftNeWritePtr_;
	int outFftSampleCounter_;
	bool outFftReady_;
	
	Spectrum fftSpectrum_;
	float fftSpectrumFreq_;
	ne10_fft_cpx_float32_t* specFftNeInput_ = nullptr;
    ne10_fft_cpx_float32_t* specFftNeOutput_ = nullptr;
    ne10_fft_cfg_float32_t specFftcfg_;
    std::vector<float> specFftInputBuffer_;
	std::vector<float> specFftOutputBuffer_;
	int specFftWritePtr_;
	int specFftNeWritePtr_;
	int specFftSampleCounter_;
	bool specFftReady_;
	
	//dev tools
	int frameCount_; // frame counter for square wave
	float framePeriod_; // one frame in seconds
};

#endif