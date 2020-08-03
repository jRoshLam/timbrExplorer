#include <Bela.h>
#include <libraries/Trill/Trill.h>
#include <libraries/OnePole/OnePole.h>
#include <libraries/Gui/Gui.h>
#include <libraries/GuiController/GuiController.h>
#include <libraries/Midi/Midi.h>
#include <libraries/Scope/Scope.h>
#include "note.h"
#include "articulation.h"

// Trill object
Trill spectrumTrill, dynamicsTrill;
// Horizontal and vertical position for Trill sensor
float gSpecTouchPosition[2] = { 0.5 , 0.5 };
float gCurrentSpecPosition[2] = {0.5, 0.5};
float gDynTouchPosition[2] = { 0.5 , 0.5 };
float gCurrentDynPosition[2] = {0.5, 0.5};
// Touch size
float gSpecTouchSize = 0.0;
float gSpecCurrentTouch = 0;
float gDynTouchSize = 0.0;
float gDynCurrentTouch = 0;
float gTouchThreshold = 0.1;

// One Pole filters objects declaration
OnePole specFilt, brightFilt, articFilt, envFilt, specTouchFilt, dynTouchFilt;

// Sleep time for auxiliary task
unsigned int gTaskSleepTime = 12000;

// browser-based GUI to adjust parameters
Gui gui;
GuiController controller;
// Time period (in seconds) after which data will be sent to the GUI
float gGuiPeriod = 0.015;

// Oscilloscope
Scope gScope;

// Note object
Note gDevNote;

// Timbre parameters and initial values
int gSpectrum = MAX_SPECTRUM / 2 - 1;
int gBrightness = MAX_BRIGHTNESS / 2 - 1;
int gArticulation = MAX_ARTICULATION / 2 - 1;
int gEnvelope = MAX_ENVELOPE / 2 - 1;


Spectrum gDevSpecDens;

//AM
// float gDevFmRatios[NUM_OPERATORS] = {1, 3, 5, 7};
// float gDevFmAmps[NUM_OPERATORS] = {.9, .8, .78, .76};
// int gDevFmConfig = kFmConfigAM;

//
float gDevFmRatios[NUM_OPERATORS] = {1, 3, 1, 2};
float gDevFmAmps[NUM_OPERATORS] = {0, 0, .99, 0};
int gDevFmConfig = kFmConfigDoubleStack31;

unsigned int gFrameCount = 0;

/*
 * Function to be run on an auxiliary task that reads data from the Trill sensor.
 * Here, a loop is defined so that the task runs recurrently for as long as the
 * audio thread is running.
 */
void loop(void*)
{
	while(!Bela_stopRequested())
	{
		// Read locations from Trill sensor
		spectrumTrill.readI2C();
		gSpecTouchSize = spectrumTrill.compoundTouchSize();
		// gSpecTouchPosition[0] = spectrumTrill.compoundTouchHorizontalLocation();
		// gSpecTouchPosition[1] = spectrumTrill.compoundTouchLocation();
		if (gSpecTouchSize > gTouchThreshold)
		{
			gSpecTouchPosition[0] = spectrumTrill.compoundTouchHorizontalLocation();
			gSpecTouchPosition[1] = spectrumTrill.compoundTouchLocation();
		}
		
		dynamicsTrill.readI2C();
		gDynTouchSize = dynamicsTrill.compoundTouchSize();
		// gDynTouchPosition[0] = dynamicsTrill.compoundTouchHorizontalLocation();
		// gDynTouchPosition[1] = dynamicsTrill.compoundTouchLocation();
		if (gDynTouchSize > gTouchThreshold)
		{
			gDynTouchPosition[0] = dynamicsTrill.compoundTouchHorizontalLocation();
			gDynTouchPosition[1] = dynamicsTrill.compoundTouchLocation();
		}
		
		usleep(gTaskSleepTime);
	}
}

bool setup(BelaContext *context, void *userData)
{
	// Trill setup============================================================
	
	// Setup a Trill Square on i2c bus 1, using the default mode and address
	if(spectrumTrill.setup(1, Trill::SQUARE, 40) != 0) {
		fprintf(stderr, "Unable to initialise Spectrum Trill Square\n");
		return false;
	}
	if(dynamicsTrill.setup(1, Trill::SQUARE, 41) != 0) {
		fprintf(stderr, "Unable to initialise Dynamics Trill Square\n");
		return false;
	}

	spectrumTrill.printDetails();
	dynamicsTrill.printDetails();
	
	// Set and schedule auxiliary task for reading sensor data from the I2C bus
	Bela_runAuxiliaryTask(loop);

	// Setup low pass filters for smoothing frequency, amplitude and panning
	specFilt.setup(1, context->audioSampleRate/context->audioFrames); // Cut-off frequency = 1Hz
	brightFilt.setup(1, context->audioSampleRate/context->audioFrames); // Cut-off frequency = 1Hz
	articFilt.setup(1, context->audioSampleRate/context->audioFrames); // Cut-off frequency = 1Hz
	envFilt.setup(1, context->audioSampleRate/context->audioFrames); // Cut-off frequency = 1Hz
	specTouchFilt.setup(1, context->audioSampleRate/context->audioFrames); // Cut-off frequency = 1Hz
	dynTouchFilt.setup(1, context->audioSampleRate/context->audioFrames); // Cut-off frequency = 1Hz
	
	// Other setup============================================================
	
	gDevNote.setSampleRate(context->audioSampleRate);
	if (!gDevNote.initMidi())
		return false;
	// gDevSpecDens = SpectralDensity(context->audioSampleRate, 440.0);
	
	// gDevSpecDens.updateSpectrum(20);
	
	// gDevSpecDens.setFrequencyRatios(gDevFmRatios);
	// gDevSpecDens.setAmplitudes(gDevFmAmps);
	// gDevSpecDens.setAlgorithm(gDevFmConfig);
	
	// Set up the GUI
	// Oscilloscope
	gScope.setup(1, context->audioSampleRate);
	gui.setup(context->projectName);
	
	// controller.setup(&gui, "Timbre Controller");	
	// // Arguments: name, default, minimum, maximum, increment
	// controller.addSlider("Spectrum", MAX_SPECTRUM/2, 0, MAX_SPECTRUM, 1);
	// controller.addSlider("Brightness", MAX_BRIGHTNESS/2, 0, MAX_BRIGHTNESS, 1);
	// controller.addSlider("Articulation", MAX_ARTICULATION/2, 0, MAX_ARTICULATION, 1);
	// controller.addSlider("Envelope", MAX_ENVELOPE/2, 0, MAX_ENVELOPE, 1);
	
	gDevNote.setSpectrum(gSpectrum);
	gDevNote.setEnvelope(gEnvelope);
	gDevNote.setArticulation(gArticulation);
	gDevNote.setBrightness(gBrightness);
	// gDevNote.setSpectrum(127);
	// gDevNote.setEnvelope(127);
	// gDevNote.setArticulation(127);
	// gDevNote.setBrightness(127);
	
	
	//development stuff
	std::array<float, WAVETABLE_SIZE> sineWaveTable_;
	float tableLength = (sineWaveTable_.size());
	// sineWaveTable_.reserve(WAVETABLE_SIZE);
	for (int i = 0; i < WAVETABLE_SIZE; i++)
	{
		sineWaveTable_[i] = float(WAVETABLE_SIZE - i)/WAVETABLE_SIZE;
	}
	rt_printf("wavetable_size: %f, sample value: %f\n", tableLength, sineWaveTable_.at(0));
	
	return true;
}

void render(BelaContext *context, void *userData)
{
	static unsigned int frameCount = 0;
	
	// spectrum = int(controller.getSliderValue(0));
	// brightness = int(controller.getSliderValue(1));
	// articulation = int(controller.getSliderValue(2));
	// envelope = int(controller.getSliderValue(3));
	// gDevNote.setSpectrum(spectrum);
	// gDevNote.setEnvelope(envelope);
	// gDevNote.setArticulation(articulation);
	// gDevNote.setBrightness(brightness);
	
	
	
	// Set Timbre parameters using Trill Touch Sensors
	gSpecCurrentTouch = specTouchFilt.process(gSpecTouchSize);
	gDynCurrentTouch = dynTouchFilt.process(gDynTouchSize);
	if (gSpecCurrentTouch > gTouchThreshold)
	{
		gCurrentSpecPosition[0] = gSpecTouchPosition[0];
		gCurrentSpecPosition[1] = gSpecTouchPosition[1];
		gSpectrum = int(map(specFilt.process(gSpecTouchPosition[1]), 0, 1, 0, MAX_SPECTRUM-1));
		gBrightness = int(map(brightFilt.process(gSpecTouchPosition[0]), 0, 1, 0, MAX_BRIGHTNESS-1));
		
		gDevNote.setSpectrum(gSpectrum);
		gDevNote.setBrightness(gBrightness);
	}
	if (gDynCurrentTouch > gTouchThreshold)
	{
		gCurrentDynPosition[0] = gDynTouchPosition[0];
		gCurrentDynPosition[1] = gDynTouchPosition[1];
		gArticulation = int(map(articFilt.process(gDynTouchPosition[1]), 0, 1, 0, MAX_ARTICULATION-1));
		gEnvelope = int(map(envFilt.process(gDynTouchPosition[0]), 0, 1, 0, MAX_ENVELOPE-1));
		
		gDevNote.setArticulation(gArticulation);
		gDevNote.setEnvelope(gEnvelope);
	}
	
	// debug statements
	if ((int(context->audioFramesElapsed / context->audioFrames) % (5*int(context->audioSampleRate / context->audioFrames))) == 0)
	{
		// rt_printf("touch size: %f\n", gCurrentTouch);
		rt_printf("sliders: Spectrum: %d, Brightness: %d\n", gDevNote.spectrum(), gDevNote.brightness());
		rt_printf("sliders: articulation: %d, envelope: %d\n", gDevNote.articulation(), gDevNote.envelope());
		// gDevNote.printTimbreParameters();
	}
	
	float out = 0;
	for (unsigned int n = 0; n < context->audioFrames; n++)
	{
		if(frameCount >= gGuiPeriod*context->audioSampleRate)
		{
			gui.sendBuffer(0, gCurrentSpecPosition);
			gui.sendBuffer(1, gSpecCurrentTouch);
			gui.sendBuffer(2, gCurrentDynPosition);
			gui.sendBuffer(3, gDynCurrentTouch);

			frameCount = 0;
		}
		frameCount ++;
		
		// if (gFrameCount < context->audioSampleRate)
		// 	out = gDevNote.process(true);
		// else
		// 	out = gDevNote.process(false);
		// // reset gFrameCount
		// if (++gFrameCount >= 2 * context->audioSampleRate)
		// 	gFrameCount = 0;
		out = gDevNote.process(true);
		// out = gDevSpecDens.process();
		gScope.log(out);
		
		for (unsigned int i = 0; i < context->audioOutChannels; i++)
			audioWrite(context, n, i, out);
	}
}

void cleanup(BelaContext *context, void *userData)
{
	
}






/* ===============================================================
DEBUG ZONE
=============================================================== */


