/***** spectrum.cpp *****/
#include "spectrum.h"

// Default constructor
Spectrum::Spectrum() : Spectrum(44100.0, 440.0) {}

// Custom constructor
// fmSynth must be initialized here since it freqMod objects
// can't be copied (contains const references)
Spectrum::Spectrum(float sampleRate, float frequency) :
fmSynth_(sampleRate, frequency)
{
	sampleRate_ = sampleRate;
	frequency_ = frequency;
	
	fmAlg_ = kFmConfigAdd;
	updateFmGui_ = 1;
	
	// default spectrum value
	spectrum_ = MAX_SPECTRUM / 2;
	updateSpectrum(MAX_SPECTRUM / 2);
}

// set sample rate and that of the FreqMod object
void Spectrum::setSampleRate(float frequency)
{
	sampleRate_ = frequency;
	fmSynth_.setSampleRate(sampleRate_);
}

// reset FreqMod object and its operators
void Spectrum::reset()
{
	fmSynth_.reset();
}

// Debug Getters
int Spectrum::getSpectrum()
{
	return spectrum_;
}
const Operator& Spectrum::getDebugOperator()
{
	return fmSynth_.getDebugOperator();
}
int Spectrum::getWaveTableSize()
{
	return fmSynth_.getWaveTableSize();
}
float Spectrum::getDebugWaveValue()
{
	return fmSynth_.getDebugWaveValue();
}

// Toggle enable for advanced controls
void Spectrum::setAdvMode(bool advMode)
{
	advMode_ = advMode;
}

// Update Frequency Modulation parameters using buffer from GUI
void Spectrum::updateAdvSpectrum(float* fmBuffer)
{
	// first element is 0 or 1 for whetehre or not to update
	if (fmBuffer[0] == 0)
		return;
	
	// second element is algorithm
	fmAlg_ = fmBuffer[1];
	fmSynth_.setAlgorithm(fmAlg_);
	
	// next elements are fRatio, amplitude, and shape, alternating for each operator
	for (int i = 0; i < NUM_OPERATORS; i++)
	{
		fRatios_[i] = fmBuffer[2+3*i];
		amps_[i] = fmBuffer[3+3*i];
		opWaves_[i] = fmBuffer[4+3*i];
	}
	
	// update freqMod object
	fmSynth_.setSpectrum(amps_, fRatios_, opWaves_);
}

// Update FreqMod object based on new spectrum value
void Spectrum::updateSpectrum(int spectrum)
{
	// If spectrum hasn't changed skip the update
	if (spectrum == spectrum_)
		return;

	spectrum_ = spectrum;
	// by default behavior, the algorithm is always additive synthesis
	fmAlg_ = kFmConfigAdd;
	updateFmGui_ = 1;
		
	// All harmonics + Noise
	// TBA? ;)
	
	// All harmonics with inharmonicity factor 200-255
	if (spectrum_ >= 200)
	{
		// Add increasing inharmonicity as spectrum goes above 200
		float inharmonicity = 1 + (spectrum_ - 200) * .0002;
		// Only first operator
		amps_ = {4, 0, 0, 0};
		fRatios_ = {inharmonicity, inharmonicity, inharmonicity, inharmonicity};
		// saw waves have all harmonics
		opWaves_ = {kWaveSaw, kWaveSaw, kWaveSaw, kWaveSine};
		fmSynth_.setSpectrum(amps_, fRatios_, opWaves_);
		fmSynth_.setAlgorithm(fmAlg_);
	}
	// All Harmonics (even/odd ratio) 100-199
	else if (spectrum_ >= 100)
	{
		// kFmConfigAM divides total amplitude by 4, must compensate
		float evenOddRatio = (spectrum_ - 100) * 0.04;
		float oddEvenRatio = (4 - evenOddRatio);
		
		// change balance between sawtooth (all harmonics) and square wave (odd harmonics)
		// don't need other 2 operators
		amps_ = {evenOddRatio, oddEvenRatio, 0, 0};
		fRatios_ = {1, 1, 1, 1};
		opWaves_ = {kWaveSaw, kWaveSquare, kWaveSine, kWaveSine};
		fmSynth_.setSpectrum(amps_, fRatios_, opWaves_);
		fmSynth_.setAlgorithm(fmAlg_);
		
		//FM alternative
		// algorithm double stack 22 already halves each stack, multiply amplitudes by 2 to compensate
		// fmSynth_.setFrequencyRatios(1, 1, 1, 2);
		// fmSynth_.setAmplitudes(evenOddRatio, 1.5, oddEvenRatio, 1.5);
		// fmSynth_.setAlgorithm(kFmConfigDoubleStack22);
	}
	// Odd harmonics 50-99
	else if (spectrum_ >= 50)
	{
		// kFmConfigAM divides total amplitude by 4, must compensate
		float evenOddRatio = (spectrum_ - 50) * 0.08;
		float oddEvenRatio = (4 - evenOddRatio);
		
		// Change balance between square wave (odd harmonics) and Triangle (quieter odd harmonics)
		// don't need other 2 operators
		amps_ = {oddEvenRatio, evenOddRatio, 0, 0};
		fRatios_ = {1, 1, 1, 1};
		// square wave operator is same as in even-odd zone for smooth transition
		opWaves_ = {kWaveTriangle, kWaveSquare, kWaveSine, kWaveSine};
		fmSynth_.setSpectrum(amps_, fRatios_, opWaves_);
		fmSynth_.setAlgorithm(fmAlg_);
	}
	// Even less odd harmonics 40-10
	else if (spectrum_ >= 40)
	{
		float evenOddRatio = (spectrum_ - 40) * 0.4;
		float oddEvenRatio = (4 - evenOddRatio);
		
		// Change balance between triangle wave (odd harmonics) and sine (no harmonics)
		// don't need other 2 operators
		amps_ = {evenOddRatio, oddEvenRatio, 0, 0};
		fRatios_ = {1, 1, 1, 1};
		// triangle wave operator is same as in even-odd zone for smooth transition
		opWaves_ = {kWaveTriangle, kWaveSine, kWaveSine, kWaveSine};
		fmSynth_.setSpectrum(amps_, fRatios_, opWaves_);
		fmSynth_.setAlgorithm(fmAlg_);
	}
	
	// assorted off harmonics
	// timpani 1st, 1.5th, 1.98th, 2.44th harmonics 30-39
	else if (spectrum_ < 40 && spectrum_ >= 30)
	{
		amps_ = {1, 0.8, 0.6, 0.4};
		fRatios_ = {1, 1.5, 1.98, 2.44};
		opWaves_ = {kWaveSine, kWaveSine, kWaveSine, kWaveSine};
		fmSynth_.setSpectrum(amps_, fRatios_, opWaves_);
		fmSynth_.setAlgorithm(fmAlg_);
	}
	// marimba: 1st, 4th and 9(.2)th harmonic 20-29
	else if (spectrum_ < 30 && spectrum_ >= 20)
	{
		amps_ = {1, 0.8, 0.8, 0};
		fRatios_ = {1, 4, 9.2, 1};
		opWaves_ = {kWaveSine, kWaveSine, kWaveSine, kWaveSine};
		fmSynth_.setSpectrum(amps_, fRatios_, opWaves_);
		fmSynth_.setAlgorithm(fmAlg_);
	}
	// Xylophone: 1st, 3rd harmonic 10-19
	else if (spectrum_ < 20 && spectrum_ >= 10)
	{
		amps_ = {1, 0.8, 0, 0};
		fRatios_ = {1, 3, 1, 1};
		opWaves_ = {kWaveSine, kWaveSine, kWaveSine, kWaveSine};
		fmSynth_.setSpectrum(amps_, fRatios_, opWaves_);
		fmSynth_.setAlgorithm(fmAlg_);
	}
	
	// Sine Wave/glockenspiel
	else
	{
		// kFmConfigAM divides total amplitude by 4, must compensate
		amps_ = {4, 0, 0, 0};
		fRatios_ = {1, 1, 1, 1};
		opWaves_ = {kWaveSine, kWaveSine, kWaveSine, kWaveSine};
		fmSynth_.setSpectrum(amps_, fRatios_, opWaves_);
		fmSynth_.setAlgorithm(fmAlg_);
	}
	
}

// set fundamental freqency of spectrum
void Spectrum::setFrequency(float frequency)
{
	if (frequency == frequency_)
		return;
	frequency_ = frequency;
	fmSynth_.setFrequency(frequency_);
	
}

// get next signal value to play
float Spectrum::process()
{
	return fmSynth_.process();
}

// send current FM algorithm to GUI
void Spectrum::sendAlg(Gui& gui, int bufferId)
{
	int fmControlBuffer[2] = {updateFmGui_, fmAlg_};
	gui.sendBuffer(bufferId, fmControlBuffer);
	updateFmGui_ = 0;
}

// send current operator frequency ratios to the GUI
void Spectrum::sendRatios(Gui& gui, int bufferId)
{
	gui.sendBuffer(bufferId, fRatios_);
}

// send current operator amplitudes to the GUI
void Spectrum::sendAmps(Gui& gui, int bufferId)
{
	gui.sendBuffer(bufferId, amps_);
}

// send current operator waveshapes to the GUI
void Spectrum::sendShapes(Gui& gui, int bufferId)
{
	gui.sendBuffer(bufferId, opWaves_);
}