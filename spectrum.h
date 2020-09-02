/***** spectralDensity.h *****/
#ifndef SPECTRUM_H
#define SPECTRUM_H

#include "freqMod.h"
#include <libraries/Gui/Gui.h>
#include <vector>

#define MAX_SPECTRUM 256

class Spectrum
{
public:
	Spectrum(); // Default constructor
	Spectrum(float sampleRate, float frequency); // Constructor with parameters
	
	void setSampleRate(float frequency); // Set sample rate
	
	void reset(); // reset freqMod obect and its operators
	
	// Debug Getters
	int getSpectrum();
	const Operator& getDebugOperator();
	int getWaveTableSize();
	float getDebugWaveValue();
	
	// Toggle enable for advanced controls
	void setAdvMode(bool advMode);
	
	// update FM spectrum based on buffer from GUI (advanced mode)
	void updateAdvSpectrum(float* fmBuffer);
	
	// Update FeqMod object based on Spectrum value
	void updateSpectrum(int spectrum);
	
	// Set fundamental frequency of spectrum
	void setFrequency(float frequency);
	
	// Retrieve next signal value
	float process();
	
	//send FM information to GUI
	void sendAlg(Gui& gui, int bufferId);
	void sendRatios(Gui& gui, int bufferId);
	void sendAmps(Gui& gui, int bufferId);
	void sendShapes(Gui& gui, int bufferId);
	
private:
	// FreqMod object
	FreqMod fmSynth_;
	
	int spectrum_;
	// boolean for toggling advanced mode
	bool advMode_;
	
	float sampleRate_; // sample rate
	float frequency_; // fundamental frequency
	
	// vectors holding operator parameters to be used by the FreqMod object
	int fmAlg_;
	std::vector<float> amps_;
	std::vector<float> fRatios_;
	std::vector<int> opWaves_;
	// whether or not to update the FmGui, 1 for yes, 0 for no
	int updateFmGui_;
};

#endif