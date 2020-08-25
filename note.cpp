/***** note.cpp *****/
#include "note.h"

// Constructor
Note::Note() : Note(44100.0, 440.0) {}

// Constructor specifying a sample rate
Note::Note(float sampleRate, float frequency) :
spectrum_(sampleRate, frequency),
outFftInputBuffer_(FFT_BUFFER_N),
outFftOutputBuffer_(FFT_OUT_N),
fftWindowBuffer_(FFT_BUFFER_N),
fftSpectrum_(sampleRate, frequency),
specFftInputBuffer_(FFT_BUFFER_N),
specFftOutputBuffer_(FFT_OUT_N)
{
	sampleRate_ = sampleRate;
	frequency_ = frequency;
	velocity_ = 0;
	noteOn_ = false;
	fftSpectrumFreq_ = 440;

	// Initialize timbre parameter objects
	// Spectrum constructor not called here since it is not copyable
	brightness_ = Brightness(sampleRate_, frequency_);
	articulation_ = Articulation(sampleRate_);
	envelope_ = Envelope(sampleRate_);
	
	//polyphony
	// for (int i = 0; i < NUM_VOICES; i++)
	// {
	// 	frequencies_.push_back(frequency);
	// 	qFactors_.push_back(1);
	// 	velocities_.push_back(0);
	// 	midiNoteOns_.push_back(false);
	// 	noteOns_.push_back(false);
	// 	spectrums_.push_back(Spectrum(sampleRate, frequency));
	// 	brightnesses_.push_back(Brightness(sampleRate_, frequency_));
	// 	articulations_.push_back(Articulation(sampleRate_));
	// 	envelopes_.push_back(Envelope(sampleRate_));
	// }
	
	// FFT variables and setup
	outFftReady_ = false;
	outFftWritePtr_ = 0;
	outFftNeWritePtr_ = 0;
	outFftSampleCounter_ = 0;
	outFftNeInput_ = (ne10_fft_cpx_float32_t*) NE10_MALLOC (FFT_BUFFER_N * sizeof (ne10_fft_cpx_float32_t));
	outFftNeOutput_ = (ne10_fft_cpx_float32_t*) NE10_MALLOC (FFT_BUFFER_N * sizeof (ne10_fft_cpx_float32_t));
	outFftcfg_ = ne10_fft_alloc_c2c_float32_neon (FFT_BUFFER_N);
	// Calculate a Hann window
	for(int n = 0; n < FFT_BUFFER_N; n++) {
		fftWindowBuffer_[n] = 0.5f * (1.0f - cosf(2.0f * M_PI * n / (float)(FFT_BUFFER_N - 1)));
	}
	fftSpectrum_.setFrequency(fftSpectrumFreq_);
	specFftReady_ = false;
	specFftWritePtr_ = 0;
	specFftNeWritePtr_ = 0;
	specFftSampleCounter_ = 0;
	specFftNeInput_ = (ne10_fft_cpx_float32_t*) NE10_MALLOC (FFT_BUFFER_N * sizeof (ne10_fft_cpx_float32_t));
	specFftNeOutput_ = (ne10_fft_cpx_float32_t*) NE10_MALLOC (FFT_BUFFER_N * sizeof (ne10_fft_cpx_float32_t));
	specFftcfg_ = ne10_fft_alloc_c2c_float32_neon (FFT_BUFFER_N);
	
	//debug variables for square wave timing
	frameCount_ = 0;
	framePeriod_ = int(sampleRate / frequency_);
}
Note::~Note()
{
	NE10_FREE(outFftNeInput_);
	NE10_FREE(outFftNeOutput_);
	NE10_FREE(outFftcfg_);
	NE10_FREE(specFftNeInput_);
	NE10_FREE(specFftNeOutput_);
	NE10_FREE(specFftcfg_);
}

// Set sample rate of object and all timbre objects
void Note::setSampleRate(float sampleRate)
{
	sampleRate_ = sampleRate;
	framePeriod_ = int(sampleRate / frequency_); // for debug square wave
	
	spectrum_.setSampleRate(sampleRate_);
	brightness_.setSampleRate(sampleRate_);
	articulation_.setSampleRate(sampleRate_);
	envelope_.setSampleRate(sampleRate_);
	
	// for (int i = 0; i < NUM_VOICES; i++)
	// {
	// 	spectrums_[i].setSampleRate(sampleRate_);
	// 	brightnesses_[i].setSampleRate(sampleRate_);
	// 	articulations_[i].setSampleRate(sampleRate_);
	// 	envelopes_[i].setSampleRate(sampleRate_);
	// }
	
	fftSpectrum_.setSampleRate(sampleRate_);
}

// CALL IN SETUP
bool Note::initMidi()
{
	// Initialize MIDI object
	if(midi_.readFrom(midiPort0_) < 0) {
		rt_printf("Unable to read from MIDI port %s\n", midiPort0_);
		return false;
	}
	midi_.writeTo(midiPort0_);
	midi_.enableParser(true);
	
	// Initialize midi to frequency table
	for (int i = 0; i < NUM_MIDI_NOTES; i++)
	{
		midiToFreqTable_[i] = powf(2.0, (i - 69)/12.0) * 440.0;
		//maximum velocity is 128
		if (i <= 40) // 0-40
			velocityToQTable_[i] = .707;
		else if (i <= 80) // 41-80
			velocityToQTable_[i] = .707 + (i - 40) * 0.007325;
		else // 81 - 128
			velocityToQTable_[i] = 1 + (i - 80) * 0.0375;
	}	
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

// Setters for Timbre Parameters
void Note::setSpectrum(int spectrum)
{
	spectrum_.updateSpectrum(spectrum);
	fftSpectrum_.updateSpectrum(spectrum);
	
	// for (int i = 0; i < NUM_VOICES; i++)
	// 	spectrums_[i].updateSpectrum(spectrum);
}
void Note::setBrightness(int brightness)
{
	brightness_.updateBrightness(brightness);
	// for (int i = 0; i < NUM_VOICES; i++)
	// 	brightnesses_[i].updateBrightness(brightness);
}
void Note::setArticulation(int articulation)
{
	articulation_.updateArticulation(articulation);
	// for (int i = 0; i < NUM_VOICES; i++)
	// 	articulations_[i].updateArticulation(articulation);
}
void Note::setEnvelope(int envelope)
{
	envelope_.updateEnvelope(envelope);
	// for (int i = 0; i < NUM_VOICES; i++)
	// 	envelopes_[i].updateEnvelope(envelope);
}

// Toggle enable for advanced controls
void Note::setAdvMode(float advMode)
{
	// if input is 0, amBool is false, otherwise it's true
	bool amBool = !(advMode == 0);
	// Don't update if advMode_ hasn't actually changed
	if (advMode_ == amBool)
		return;
	advMode_ = amBool;
	
	//update advanced mode for all timbre dimensions
	spectrum_.setAdvMode(advMode_);
	brightness_.setAdvMode(advMode_);
	articulation_.setAdvMode(advMode_);
	envelope_.setAdvMode(advMode_);
	fftSpectrum_.setAdvMode(advMode_);
	
	// for (int i = 0; i < NUM_VOICES; i++)
	// {
	// 	spectrums_[i].setAdvMode(advMode_);
	// 	brightnesses_[i].setAdvMode(advMode_);
	// 	articulations_[i].setAdvMode(advMode_);
	// 	envelopes_[i].setAdvMode(advMode_);
	// }
}

// Set advanced controls
void Note::setAdvControls(float* controlData)
{
	if (!advMode_)
		return;
	
	brightness_.setAdvControls(controlData[kACIBrMidiLink], controlData[kACIBrQ]);
	articulation_.setAdvControls(controlData[kACIArQ]);
	envelope_.setAdvControls(controlData[kACIDecay], controlData[kACISustain], controlData[kACIRelease]);
	
	// for (int i = 0; i < NUM_VOICES; i++)
	// {
	// 	brightnesses_[i].setAdvControls(controlData[kACIBrMidiLink], controlData[kACIBrQ]);
	// 	articulations_[i].setAdvControls(controlData[kACIArQ]);
	// 	envelopes_[i].setAdvControls(controlData[kACIDecay], controlData[kACISustain], controlData[kACIRelease]);
	// }
}

// update FM spectrum
void Note::updateAdvSpectrum(float* fmBuffer)
{
	if (!advMode_)
		return;
	
	spectrum_.updateAdvSpectrum(fmBuffer);
	fftSpectrum_.updateAdvSpectrum(fmBuffer);
	
	// for (int i = 0; i < NUM_VOICES; i++)
	// 	spectrums_[i].updateAdvSpectrum(fmBuffer);
}

// Set Frequency of the note and relevant timbre dimensions
void Note::setMidiIn(float frequency, float qFactor, int indx)
{
	if (frequency_ == frequency && qFactor_ == qFactor)
		return;
	frequency_ = frequency;
	qFactor_ = qFactor;
	spectrum_.setFrequency(frequency_);
	brightness_.setMidiIn(frequency_, qFactor_);
	articulation_.setFrequency(frequency_);
	
	// polyphony
	// frequencies_[indx] = frequency;
	// qFactors_[indx] = qFactor;
	// spectrums_[indx].setFrequency(frequency);
	// brightnesses_[indx].setMidiIn(frequency, qFactor);
	// articulations_[indx].setFrequency(frequency);
}

// Run full signal chain of note, triggered by MIDI
float Note::process(Gui& gui, bool noteOn)
{
	// Check MIDI messages
	while (midi_.getParser()->numAvailableMessages() > 0)
	{
		// retrieve MIDI message
		MidiChannelMessage message;
		message = midi_.getParser()->getNextChannelMessage();
		
		// A MIDI "note on" message type might actually hold a real
		// note onset (e.g. key press), or it might hold a note off (key release).
		// The latter is signified by a velocity of 0.
		if(message.getType() == kmmNoteOn) {
			message.prettyPrint();
			int noteNumber = message.getDataByte(0);
			int velocity = message.getDataByte(1);
			float noteFrequency = midiToFreqTable_[noteNumber];
			float qFactor = velocityToQTable_[velocity];
			
			// Velocity of 0 is really a note off
			if (velocity == 0 && noteFrequency == frequency_)
			{
				midiNoteOn_ = false;
				gui.sendBuffer(kBtGMidi, 0);
			}
			else if (!noteOn_)
			{
				// last input is meant for polyphony (not used here)
				setMidiIn(noteFrequency, qFactor, 0);
				int midiBuffer[2] = {noteNumber, velocity};
				gui.sendBuffer(kBtGMidi, midiBuffer);
				midiNoteOn_ = true;
			}
		}
		else if(message.getType() == kmmNoteOff) {
			// We can also encounter the "note off" message type which is the same
			// as "note on" with a velocity of 0.
			int noteNumber = message.getDataByte(0);
			float noteFrequency = midiToFreqTable_[noteNumber];
			if (noteFrequency == frequency_)
			{
				midiNoteOn_ = false;
				gui.sendBuffer(kBtGMidi, 0);
			}
		}
	}
	
	// use envelope to see if we are producing sound, input midiNoteOn
	float amplitude = envelope_.process(midiNoteOn_);
	
	// Alternatively, have envelope controlled by noteOn input (debug)
	// float amplitude = envelope_.process(noteOn);
	
	float out = 0;
	
	// if note just turned on (was previously off)
	if (!noteOn_ && envelope_.isNoteOn())
	{
		//reset as necessary
		brightness_.reset();
		articulation_.reset();
	}
	
	// even if input midiNoteOn_ is off, the envelope may still be playing
	noteOn_ = envelope_.isNoteOn();

	// obtain waveform value
	if (noteOn_)
	{
		// // debug square wave instead of spectrum
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
	
	// add output to fft buffer
	outFftInputBuffer_[outFftWritePtr_] = out;
	if (++outFftWritePtr_ >= FFT_BUFFER_N)
		outFftWritePtr_ = 0;
	if (++outFftSampleCounter_ >= FFT_HOP_SIZE)
	{
		outFftSampleCounter_ = 0;
		outFftReady_ = true;
	}
	
	//add fftSpectrum output to fft buffer
	specFftInputBuffer_[specFftWritePtr_] = fftSpectrum_.process();
	if (++specFftWritePtr_ >= FFT_BUFFER_N)
		specFftWritePtr_ = 0;
	if (++specFftSampleCounter_ >= FFT_HOP_SIZE)
	{
		specFftSampleCounter_ = 0;
		specFftReady_ = true;
	}
	return out;
}

// // Run full signal chain of note, triggered by MIDI
// float Note::process(Gui& gui, bool noteOn)
// {
// 	// Check MIDI messages
// 	while (midi_.getParser()->numAvailableMessages() > 0)
// 	{
// 		// retrieve MIDI message
// 		MidiChannelMessage message;
// 		message = midi_.getParser()->getNextChannelMessage();
		
// 		// A MIDI "note on" message type might actually hold a real
// 		// note onset (e.g. key press), or it might hold a note off (key release).
// 		// The latter is signified by a velocity of 0.
// 		if(message.getType() == kmmNoteOn) {
// 			message.prettyPrint();
// 			int noteNumber = message.getDataByte(0);
// 			int velocity = message.getDataByte(1);
// 			float noteFrequency = midiToFreqTable_[noteNumber];
// 			float qFactor = velocityToQTable_[velocity];
			
// 			for (int i = 0; i < NUM_VOICES; i++)
// 			{
// 				// Velocity of 0 is really a note off
// 				if (velocity == 0 && noteFrequency == frequencies_[i])
// 				{
// 					midiNoteOns_[i] = false;
// 					// gui.sendBuffer(kBtGMidi, 0);
// 				}
// 				// if it's a true noteOn and we find an unused note, turn it on and
// 				// exit the loop
// 				else if (!noteOns_[i])
// 				{
// 					setMidiIn(noteFrequency, qFactor, i);
// 					// int midiBuffer[2] = {noteNumber, velocity};
// 					// gui.sendBuffer(kBtGMidi, midiBuffer);
// 					midiNoteOns_[i] = true;
// 					break;
// 				}
// 				// if the frequency matches a note that's already on, also exit loop
// 				else if (noteOns_[i] && noteFrequency == frequencies_[i])
// 				{
// 					setMidiIn(noteFrequency, qFactor, i);
// 					midiNoteOns_[i] = true;
// 					break;
// 				}
// 			}
// 		}
// 		else if(message.getType() == kmmNoteOff) {
// 			// We can also encounter the "note off" message type which is the same
// 			// as "note on" with a velocity of 0.
// 			int noteNumber = message.getDataByte(0);
// 			float noteFrequency = midiToFreqTable_[noteNumber];
// 			for (int i = 0; i < NUM_VOICES; i++)
// 				if (noteFrequency == frequencies_[i])
// 				{
// 					midiNoteOns_[i] = false;
// 					// gui.sendBuffer(kBtGMidi, 0);
// 				}
// 		}
// 	}
	
// 	// use envelope to see if we are producing sound, input midiNoteOn
// 	float amplitude[NUM_VOICES];
// 	float out = 0;
// 	float out_i;
// 	for (int i = 0; i < NUM_VOICES; i++)
// 	{
// 		amplitude[i] = envelopes_[i].process(midiNoteOns_[i]);
		
// 		// if note just turned on (was previously off)
// 		if (!noteOns_[i] && envelopes_[i].isNoteOn())
// 		{
// 			//reset as necessary
// 			brightnesses_[i].reset();
// 			articulations_[i].reset();
// 		}
		
// 		// even if input midiNoteOn_ is off, the envelope may still be playing
// 		noteOns_[i] = envelopes_[i].isNoteOn();
	
// 		// obtain waveform value
// 		if (noteOns_[i])
// 		{
// 			// // debug square wave instead of spectrum
// 			// out = squareWaveDev();
			
// 			// Get basic FM-generated waveform
// 			out_i = spectrums_[i].process();
			
// 			// apply brightness filter
// 			out_i = brightnesses_[i].process(out_i);
			
// 			// apply articulation filter
// 			out_i = articulations_[i].process(out_i);
			
// 			// apply volume envelope
// 			out += out_i * amplitude[i];
// 		}
// 	}
		
// 	// add output to fft buffer
// 	outFftInputBuffer_[outFftWritePtr_] = out;
// 	if (++outFftWritePtr_ >= FFT_BUFFER_N)
// 		outFftWritePtr_ = 0;
// 	if (++outFftSampleCounter_ >= FFT_HOP_SIZE)
// 	{
// 		outFftSampleCounter_ = 0;
// 		outFftReady_ = true;
// 	}
		
// 	//add fftSpectrum output to fft buffer
// 	specFftInputBuffer_[specFftWritePtr_] = fftSpectrum_.process();
// 	if (++specFftWritePtr_ >= FFT_BUFFER_N)
// 		specFftWritePtr_ = 0;
// 	if (++specFftSampleCounter_ >= FFT_HOP_SIZE)
// 	{
// 		specFftSampleCounter_ = 0;
// 		specFftReady_ = true;
// 	}
	
// 	return out;
// }

bool Note::checkFftReady()
{
	return (outFftReady_ && specFftReady_);
}

void Note::outputFft(Gui& gui)
{
	if (outFftReady_)
	{
		outFftReady_ = false;
		outFftNeWritePtr_ = (outFftWritePtr_ + 1) % FFT_BUFFER_N;
		// Copy data from circular buffer to NE10 buffer
		for(int n = 0; n < FFT_BUFFER_N; n++)
		{
			outFftNeInput_[n].r = (ne10_float32_t) outFftInputBuffer_[outFftNeWritePtr_] * fftWindowBuffer_[n];
			outFftNeInput_[n].i = 0;
	
			outFftNeWritePtr_++;
			if(outFftNeWritePtr_ >= FFT_BUFFER_N)
				outFftNeWritePtr_ = 0;
		}
		
		// Run FFT
		ne10_fft_c2c_1d_float32_neon (outFftNeOutput_, outFftNeInput_, outFftcfg_, 0);
		
		// normalizing factor for fft
		float normFactor = 4.0 / float(FFT_BUFFER_N);
		// Copy FFT amplitude to buffer to send to GUI
		for(int n = 0; n < FFT_OUT_N; n++)
		{
			//calculate amplitude of FFT
			outFftOutputBuffer_[n] = sqrtf(outFftNeOutput_[n].r * outFftNeOutput_[n].r + outFftNeOutput_[n].i * outFftNeOutput_[n].i);
			// normalize
			outFftOutputBuffer_[n] = outFftOutputBuffer_[n] * normFactor;
			// then convert to decibels
			if (outFftOutputBuffer_[n] > 0.001)
				outFftOutputBuffer_[n] = 20 * log10(outFftOutputBuffer_[n]);
			else
				outFftOutputBuffer_[n] = -60;
			//convert to range [-60, 20] to [0, 1] (0.0125 is dividing by 80)
			outFftOutputBuffer_[n] = (outFftOutputBuffer_[n] + 60) * 0.0125;
		}
		// send to gui
		gui.sendBuffer(kBtGOutFft, outFftOutputBuffer_);
	}
	if (specFftReady_)
	{
		specFftReady_ = false;
		specFftNeWritePtr_ = (specFftWritePtr_ + 1) % FFT_BUFFER_N;
		// Copy data from circular buffer to NE10 buffer
		for(int n = 0; n < FFT_BUFFER_N; n++)
		{
			specFftNeInput_[n].r = (ne10_float32_t) specFftInputBuffer_[specFftNeWritePtr_] * fftWindowBuffer_[n];
			specFftNeInput_[n].i = 0;
	
			specFftNeWritePtr_++;
			if(specFftNeWritePtr_ >= FFT_BUFFER_N)
				specFftNeWritePtr_ = 0;
		}
		
		// Run FFT
		ne10_fft_c2c_1d_float32_neon (specFftNeOutput_, specFftNeInput_, specFftcfg_, 0);

		// normalizing factor for fft
		float normFactor = 4.0 / float(FFT_BUFFER_N);
		// Copy FFT amplitude to buffer to send to GUI
		for(int n = 0; n < FFT_OUT_N; n++)
		{
			//calculate amplitude of FFT
			specFftOutputBuffer_[n] = sqrtf(specFftNeOutput_[n].r * specFftNeOutput_[n].r + specFftNeOutput_[n].i * specFftNeOutput_[n].i);
			// normalize
			specFftOutputBuffer_[n] = specFftOutputBuffer_[n] * normFactor;
			//then convert to decibels
			if (specFftOutputBuffer_[n] > 0.001)
				specFftOutputBuffer_[n] = 20 * log10(specFftOutputBuffer_[n]);
			else
				specFftOutputBuffer_[n] = -60;
			//convert to range [-60, 20] to [0, 1] (0.0125 is dividing by 80)
			specFftOutputBuffer_[n] = (specFftOutputBuffer_[n] + 60) * 0.0125;
			
		}
		// send to gui
		gui.sendBuffer(kBtGSpecFft, specFftOutputBuffer_);
	}
}

void Note::updateGraphs(Gui& gui)
{
	brightness_.updateFrfGraph(gui, kBtGBrightFrf);
	articulation_.updateFcGraph(gui, kBtGArticulation);
}

// Send GUI information
void Note::sendToGui(Gui& gui)
{
	envelope_.sendToGui(gui, kBtGEnvelope);
	spectrum_.sendAlg(gui, kBtGFMAlg);
	spectrum_.sendRatios(gui, kBtGFMRatios);
	spectrum_.sendAmps(gui, kBtGFMAmps);
	spectrum_.sendShapes(gui, kBtGFMShapes);
}



// Debug function that returns a constant square wave.
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

// debug print statements
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

	// rt_printf("Operator tableLength: %f, curent table: %f\n",
	// 	spectrum_.getDebugOperator().tableLength(), spectrum_.getDebugOperator().debugValue());
	// rt_printf("sine wave tableLength: %d, test value: %f\n",
	// 	spectrum_.getWaveTableSize(), spectrum_.getDebugWaveValue());
}