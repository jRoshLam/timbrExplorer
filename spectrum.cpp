/***** spectrum.cpp *****/
#include "spectrum.h"


Spectrum::Spectrum() : Spectrum(44100.0, 440.0) {}

Spectrum::Spectrum(float sampleRate, float frequency) :
fmSynth_(sampleRate, frequency)
{
	sampleRate_ = sampleRate;
	frequency_ = frequency;
	
	// fmSynth_ = FreqMod(sampleRate_, frequency_);
	
	spectrum_ = MAX_SPECTRUM / 2;
	updateSpectrum(MAX_SPECTRUM / 2);
}

void Spectrum::setSampleRate(float f)
{
	sampleRate_ = f;
	fmSynth_.setSampleRate(sampleRate_);
}

void Spectrum::reset()
{
	fmSynth_.reset();
}

int Spectrum::getSpectrum()
{
	return spectrum_;
}

Operator Spectrum::getDebugOperator()
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

void Spectrum::updateSpectrum(int spectrum)
{
	if (spectrum == spectrum_)
		return;
		
	spectrum_ = spectrum;
		
	// All harmonics + Noise
	
	// All Harmonics (even/odd ratio)
	if (spectrum_ >= 200)
	{
		float inharmonicity = 1 + (spectrum_ - 200) * .0002;
		amps_ = {4, 0, 0, 0};
		fRatios_ = {inharmonicity, inharmonicity, inharmonicity, inharmonicity};
		opWaves_ = {kWaveSaw, kWaveSaw, kWaveSaw, kWaveSine};
		fmSynth_.setSpectrum(amps_, fRatios_, opWaves_);
		fmSynth_.setAlgorithm(kFmConfigAM);
	}
	else if (spectrum_ >= 100)
	{
		// kFmConfigAM divides total amplitude by 4, compensate here
		float evenOddRatio = (spectrum_ - 100) * 0.04;
		float oddEvenRatio = (4 - evenOddRatio);
		
		amps_ = {evenOddRatio, oddEvenRatio, 0, 0};
		fRatios_ = {1, 1, 1, 1};
		opWaves_ = {kWaveSaw, kWaveSquare, kWaveSine, kWaveSine};
		fmSynth_.setSpectrum(amps_, fRatios_, opWaves_);
		fmSynth_.setAlgorithm(kFmConfigAM);
		
		//FM that currently isn't working
		// algorithm double stack 22 already halves each stack, multiply amplitudes by 2 to compensate
		// fmSynth_.setFrequencyRatios(1, 1, 1, 2);
		// fmSynth_.setAmplitudes(evenOddRatio, 1.5, oddEvenRatio, 1.5);
		// fmSynth_.setAlgorithm(kFmConfigDoubleStack22);
	}
	else if (spectrum_ >= 50)
	{
		float evenOddRatio = (spectrum_ - 50) * 0.08;
		float oddEvenRatio = (4 - evenOddRatio);
		
		amps_ = {oddEvenRatio, evenOddRatio, 0, 0};
		fRatios_ = {1, 1, 1, 1};
		opWaves_ = {kWaveTriangle, kWaveSquare, kWaveSine, kWaveSine};
		fmSynth_.setSpectrum(amps_, fRatios_, opWaves_);
		fmSynth_.setAlgorithm(kFmConfigAM);
		
		//FM that currently isn't working
		// algorithm double stack 22 already halves each stack, multiply amplitudes by 2 to compensate
		// fmSynth_.setFrequencyRatios(1, 1, 1, 2);
		// fmSynth_.setAmplitudes(evenOddRatio, 1.5, oddEvenRatio, 1.5);
		// fmSynth_.setAlgorithm(kFmConfigDoubleStack22);
	}
	else if (spectrum_ >= 40)
	{
		float evenOddRatio = (spectrum_ - 40) * 0.4;
		float oddEvenRatio = (4 - evenOddRatio);
		
		amps_ = {evenOddRatio, oddEvenRatio, 0, 0};
		fRatios_ = {1, 1, 1, 1};
		opWaves_ = {kWaveTriangle, kWaveSine, kWaveSine, kWaveSine};
		fmSynth_.setSpectrum(amps_, fRatios_, opWaves_);
		fmSynth_.setAlgorithm(kFmConfigAM);
		
		//FM that currently isn't working
		// algorithm double stack 22 already halves each stack, multiply amplitudes by 2 to compensate
		// fmSynth_.setFrequencyRatios(1, 1, 1, 2);
		// fmSynth_.setAmplitudes(evenOddRatio, 1.5, oddEvenRatio, 1.5);
		// fmSynth_.setAlgorithm(kFmConfigDoubleStack22);
	}
	
	// Odd Harmonics (less odd harmonics?)
	
	// off harmonics
	else if (spectrum_ < 40 && spectrum_ >= 30)
	{
		// timpani
		// fmSynth_.setFrequencyRatios(1, 1.5, 1.98, 2.44);
		// fmSynth_.setAmplitudes(1, 0.8, 0.6, 0.4);
		amps_ = {1, 0.8, 0.6, 0.4};
		fRatios_ = {1, 1.5, 1.98, 2.44};
		opWaves_ = {kWaveSine, kWaveSine, kWaveSine, kWaveSine};
		fmSynth_.setSpectrum(amps_, fRatios_, opWaves_);
		fmSynth_.setAlgorithm(kFmConfigAM);
	}
	
	else if (spectrum_ < 30 && spectrum_ >= 20)
	{
		// marimba: 1st, 4th and 9(.2)th harmonic
		// fmSynth_.setFrequencyRatios(1, 4, 9.2, 1);
		// fmSynth_.setAmplitudes(1, 0.8, 0.8, 0);
		amps_ = {1, 0.8, 0.8, 0};
		fRatios_ = {1, 4, 9.2, 1};
		opWaves_ = {kWaveSine, kWaveSine, kWaveSine, kWaveSine};
		fmSynth_.setSpectrum(amps_, fRatios_, opWaves_);
		fmSynth_.setAlgorithm(kFmConfigAM);
	}
	
	else if (spectrum_ < 20 && spectrum_ >= 10)
	{
		// Xylophone: 1st, 3rd harmonic
		// fmSynth_.setFrequencyRatios(1, 3, 1, 1);
		// fmSynth_.setAmplitudes(1, 0.8, 0, 0);
		amps_ = {1, 0.8, 0, 0};
		fRatios_ = {1, 3, 1, 1};
		opWaves_ = {kWaveSine, kWaveSine, kWaveSine, kWaveSine};
		fmSynth_.setSpectrum(amps_, fRatios_, opWaves_);
		fmSynth_.setAlgorithm(kFmConfigAM);
	}
	
	// Sine Wave/glockenspiel
	// if (spectrum_ < 5)
	else
	{
		amps_ = {4, 0, 0, 0};
		fRatios_ = {1, 1, 1, 1};
		opWaves_ = {kWaveSine, kWaveSine, kWaveSine, kWaveSine};
		fmSynth_.setSpectrum(amps_, fRatios_, opWaves_);
		// fmSynth_.setFrequencyRatios(1, 1, 1, 1);
		// fmSynth_.setAmplitudes(4, 0, 0, 0);
		fmSynth_.setAlgorithm(kFmConfigAM);
	}
	
}

void Spectrum::setFrequency(float frequency)
{
	if (frequency == frequency_)
		return;
	frequency_ = frequency;
	fmSynth_.setFrequency(frequency_);
	
}

float Spectrum::process()
{
	return fmSynth_.process();
}


