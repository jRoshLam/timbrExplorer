#include <Bela.h>
#include <libraries/Trill/Trill.h>
#include <libraries/OnePole/OnePole.h>
#include <libraries/Gui/Gui.h>
#include <libraries/GuiController/GuiController.h>
#include <libraries/Midi/Midi.h>
#include <libraries/Scope/Scope.h>
#include "note.h"
#include "articulation.h"

// Trill ==============================================================
//------------ CHANGE TRILL ADDRESSES HERE -----------------
int gSpecTrillAddress = 40; 
int gDynTrillAddress = 41;
//------------ CHANGE TRILL ADDRESSES HERE -----------------
// Trill objects
Trill spectrumTrill, dynamicsTrill;
// Horizontal and vertical position for Trill sensors
float gSpecTouchPosition[2] = { 0.5 , 0.5 };
float gDynTouchPosition[2] = { 0.5 , 0.5 };
// Last position for Trill sensors that was above threshold
float gCurrentSpecPosition[2] = {0.5, 0.5};
float gCurrentDynPosition[2] = {0.5, 0.5};
// Raw Touch size
float gSpecTouchSize = 0.0;
float gDynTouchSize = 0.0;
// Filtered Touch Size
float gSpecCurrentTouch = 0;
float gDynCurrentTouch = 0;
// Threshold for registered touch
float gTouchThreshold = 0.1;
// One Pole filters objects declaration
OnePole specFilt, brightFilt, articFilt, envFilt, specTouchFilt, dynTouchFilt;
// Sleep time for auxiliary task
unsigned int gTaskSleepTime = 12000;

// GUI and IDE ==========================================================
// browser-based GUI to adjust parameters
Gui gui;
GuiController controller;
// Time period (in seconds) after which data will be sent to the GUI
float gGuiPeriod = 0.015;
// Oscilloscope
Scope gScope;

// FFT variables and functions
AuxiliaryTask gFFTTask;
AuxiliaryTask gGraphTask;
// fft callback to be made into an auxiliary task
void process_fft_background(void*);
void process_graphs_background(void*);

// Timbre ==========================================================
// Note object
// Initialized as global object with default constructor
Note gDevNote;
// Timbre parameters and initial values
int timbreBuffer[8] = {0};
int timbreDim[4] = {0};
int gSpectrum = MAX_SPECTRUM / 2 - 1;
int gBrightness = MAX_BRIGHTNESS / 2 - 1;
int gArticulation = MAX_ARTICULATION / 2 - 1;
int gEnvelope = MAX_ENVELOPE / 2 - 1;

// // Debug Spectrum Object
// Spectrum gDevSpecDens;
// AM
// float gDevFmRatios[NUM_OPERATORS] = {1, 3, 5, 7};
// float gDevFmAmps[NUM_OPERATORS] = {.9, .8, .78, .76};
// int gDevFmConfig = kFmConfigAM;
// FM
// float gDevFmRatios[NUM_OPERATORS] = {1, 3, 1, 2};
// float gDevFmAmps[NUM_OPERATORS] = {0, 0, .99, 0};
// int gDevFmConfig = kFmConfigDoubleStack31;
// Debug variable for metronome notes 
// unsigned int gFrameCount = 0;

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
		if (gSpecTouchSize > gTouchThreshold)
		{
			gSpecTouchPosition[0] = spectrumTrill.compoundTouchHorizontalLocation();
			gSpecTouchPosition[1] = spectrumTrill.compoundTouchLocation();
		}
		
		dynamicsTrill.readI2C();
		gDynTouchSize = dynamicsTrill.compoundTouchSize();
		if (gDynTouchSize > gTouchThreshold)
		{
			gDynTouchPosition[0] = dynamicsTrill.compoundTouchHorizontalLocation();
			gDynTouchPosition[1] = dynamicsTrill.compoundTouchLocation();
		}
		
		usleep(gTaskSleepTime);
	}
}

void process_fft_background(void*)
{
	gDevNote.outputFft(gui);
	// rt_printf("background process called\n");
}
void process_graphs_background(void*)
{
	gDevNote.updateGraphs(gui);
	// rt_printf("background process called\n");
}


bool setup(BelaContext *context, void *userData)
{
	// Trill setup============================================================
	// Setup Trill Squares on i2c bus 1, using the default mode
	if(spectrumTrill.setup(1, Trill::SQUARE, gSpecTrillAddress) != 0) {
		fprintf(stderr, "Unable to initialise Spectrum Trill Square\n");
		return false;
	}
	if(dynamicsTrill.setup(1, Trill::SQUARE, gDynTrillAddress) != 0) {
		fprintf(stderr, "Unable to initialise Dynamics Trill Square\n");
		return false;
	}

	spectrumTrill.printDetails();
	dynamicsTrill.printDetails();

	// Set and schedule auxiliary task for reading sensor data from the I2C bus
	Bela_runAuxiliaryTask(loop);

	// Setup low pass filters for smoothing frequency, amplitude and panning
	// All filters Cut-off frequency = 1Hz
	specFilt.setup(1, context->audioSampleRate/context->audioFrames);
	brightFilt.setup(1, context->audioSampleRate/context->audioFrames);
	articFilt.setup(1, context->audioSampleRate/context->audioFrames);
	envFilt.setup(1, context->audioSampleRate/context->audioFrames);
	specTouchFilt.setup(1, context->audioSampleRate/context->audioFrames);
	dynTouchFilt.setup(1, context->audioSampleRate/context->audioFrames);


	// Note setup ============================================================
	// Note object setup
	// We can't call the custom constructor since some class members have const references (non-copyable)
	gDevNote.setSampleRate(context->audioSampleRate);
	if (!gDevNote.initMidi())
		return false;
	// Initial Timbre values
	timbreDim[0] = MAX_SPECTRUM / 2 - 1;
	timbreDim[1] = MAX_BRIGHTNESS / 2 - 1;
	timbreDim[2] = MAX_ARTICULATION / 2 - 1;
	timbreDim[3] = MAX_ENVELOPE / 2 - 1;
	gDevNote.setSpectrum(timbreDim[0]);
	gDevNote.setEnvelope(timbreDim[1]);
	gDevNote.setArticulation(timbreDim[2]);
	gDevNote.setBrightness(timbreDim[3]);
	// gDevNote.setSpectrum(gSpectrum);
	// gDevNote.setEnvelope(gEnvelope);
	// gDevNote.setArticulation(gArticulation);
	// gDevNote.setBrightness(gBrightness);
	
	
	// GUI setup =============================================================
	// Setup Oscilloscope and GUI
	gScope.setup(1, context->audioSampleRate);
	gui.setup(context->projectName);
	
	//timbreSpaceInfo, update flag and value for each dimension, alternating
	gui.setBuffer('f', 8); // buffer ID 0
	// advMode, articulation Q, DSR info
	gui.setBuffer('f', kACBufferSize); // buffer ID 1
	// FM Spectrum - alg, amps, ratios, shapes
	gui.setBuffer('f', 2 + NUM_OPERATORS * 3); // buffer ID 2
	
	// FFT Setup
	gFFTTask = Bela_createAuxiliaryTask(&process_fft_background, 90, "fft-calculation");
	gGraphTask = Bela_createAuxiliaryTask(&process_graphs_background, 80, "graph-calculation");

	// Debug setup============================================================
	// //debug spectrum object setup
	// gDevSpecDens.updateSpectrum(20);
	// gDevSpecDens.setFrequencyRatios(gDevFmRatios);
	// gDevSpecDens.setAmplitudes(gDevFmAmps);
	// gDevSpecDens.setAlgorithm(gDevFmConfig);
	
	// // Set up the slider GUI (requires html resource)
	// controller.setup(&gui, "Timbre Controller");	
	// // Arguments: name, default, minimum, maximum, increment
	// controller.addSlider("Spectrum", MAX_SPECTRUM/2, 0, MAX_SPECTRUM, 1);
	// controller.addSlider("Brightness", MAX_BRIGHTNESS/2, 0, MAX_BRIGHTNESS, 1);
	// controller.addSlider("Articulation", MAX_ARTICULATION/2, 0, MAX_ARTICULATION, 1);
	// controller.addSlider("Envelope", MAX_ENVELOPE/2, 0, MAX_ENVELOPE, 1);
	
	return true;
}

void render(BelaContext *context, void *userData)
{
	// frame count for sending data to GUI
	static unsigned int frameCount = 0;
	
	// DEBUG GUI for setting timbre parameters with sliders
	// spectrum = int(controller.getSliderValue(0));
	// brightness = int(controller.getSliderValue(1));
	// articulation = int(controller.getSliderValue(2));
	// envelope = int(controller.getSliderValue(3));
	// gDevNote.setSpectrum(spectrum);
	// gDevNote.setEnvelope(envelope);
	// gDevNote.setArticulation(articulation);
	// gDevNote.setBrightness(brightness);
	
	//set timbre parameters using the GUI (unless)
	DataBuffer& buffer = gui.getDataBuffer(kGtBTimbreParams);
	float* data = buffer.getAsFloat();
	for (int i = 0; i < 4; i++)
	{
		if (data[2*i] == 1) {
			timbreDim[i] = int(data[2*i+1]);
			switch(i) 
			{
				case 0: gDevNote.setSpectrum(timbreDim[0]); break;
				case 1: gDevNote.setBrightness(timbreDim[1]); break;
				case 2: gDevNote.setArticulation(timbreDim[2]); break;
				case 3: gDevNote.setEnvelope(timbreDim[3]); break;
			}
		}
	}

	// Set timbre parameters using Trill Touch Sensors
	// LPF touch size to smooth data
	gSpecCurrentTouch = specTouchFilt.process(gSpecTouchSize);
	gDynCurrentTouch = dynTouchFilt.process(gDynTouchSize);
	// Only update Timbre if filtered touch size is above threshold
	if (gSpecCurrentTouch > gTouchThreshold)
	{
		// Save position to send to GUI
		gCurrentSpecPosition[0] = gSpecTouchPosition[0];
		gCurrentSpecPosition[1] = gSpecTouchPosition[1];
		
		//filter positions and map them to corresponding parameter range
		timbreDim[0] = int(map(specFilt.process(gSpecTouchPosition[1]), 0, 1, 0, MAX_SPECTRUM-1));
		timbreDim[1] = int(map(brightFilt.process(gSpecTouchPosition[0]), 0, 1, 0, MAX_BRIGHTNESS-1));
		
		gDevNote.setSpectrum(timbreDim[0]);
		gDevNote.setBrightness(timbreDim[1]);
		
		// //filter positions and map them to corresponding parameter range
		// gSpectrum = int(map(specFilt.process(gSpecTouchPosition[1]), 0, 1, 0, MAX_SPECTRUM-1));
		// gBrightness = int(map(brightFilt.process(gSpecTouchPosition[0]), 0, 1, 0, MAX_BRIGHTNESS-1));
		
		// gDevNote.setSpectrum(gSpectrum);
		// gDevNote.setBrightness(gBrightness);
		timbreBuffer[0] = 1;
		timbreBuffer[1] = timbreDim[0];
		timbreBuffer[2] = 1;
		timbreBuffer[3] = timbreDim[1];
	}
	// Only update Timbre if filtered touch size is above threshold
	if (gDynCurrentTouch > gTouchThreshold)
	{
		// Save position to send to GUI
		gCurrentDynPosition[0] = gDynTouchPosition[0];
		gCurrentDynPosition[1] = gDynTouchPosition[1];
		
		//filter positions and map them to corresponding parameter range
		timbreDim[2] = int(map(articFilt.process(gDynTouchPosition[1]), 0, 1, 0, MAX_ARTICULATION-1));
		timbreDim[3] = int(map(envFilt.process(gDynTouchPosition[0]), 0, 1, 0, MAX_ENVELOPE-1));

		gDevNote.setArticulation(timbreDim[2]);
		gDevNote.setEnvelope(timbreDim[3]);
		
		// //filter positions and map them to corresponding parameter range
		// gArticulation = int(map(articFilt.process(gDynTouchPosition[1]), 0, 1, 0, MAX_ARTICULATION-1));
		// gEnvelope = int(map(envFilt.process(gDynTouchPosition[0]), 0, 1, 0, MAX_ENVELOPE-1));

		// gDevNote.setArticulation(gArticulation);
		// gDevNote.setEnvelope(gEnvelope);
		timbreBuffer[4] = 1;
		timbreBuffer[5] = timbreDim[2];
		timbreBuffer[6] = 1;
		timbreBuffer[7] = timbreDim[3];
	}
	
	// advanced controls
	DataBuffer& advBuffer = gui.getDataBuffer(kGtBAdvControls);
	float* advData = advBuffer.getAsFloat();
	
	gDevNote.setAdvMode(advData[kACIAdvMode]);
	gDevNote.setAdvControls(advData);
	
	// advanced FM spectrum buffer and control
	DataBuffer& advFmBuffer = gui.getDataBuffer(kGtBAdvSpectrum);
	float* advFmData = advFmBuffer.getAsFloat();
	gDevNote.updateAdvSpectrum(advFmData);
	
	
	// debug statements
	if ((int(context->audioFramesElapsed / context->audioFrames) % (5*int(context->audioSampleRate / context->audioFrames))) == 0)
	{
		// rt_printf("touch size: %f\n", gCurrentTouch);
		// rt_printf("sliders: Spectrum: %d, Brightness: %d\n", gDevNote.spectrum(), gDevNote.brightness());
		// rt_printf("sliders: articulation: %d, envelope: %d\n", gDevNote.articulation(), gDevNote.envelope());
		// gDevNote.printTimbreParameters();
	}
	
	float out = 0;
	for (unsigned int n = 0; n < context->audioFrames; n++)
	{
		// Send positions to GUI at fixed intervals
		if(frameCount >= gGuiPeriod*context->audioSampleRate)
		{
			gui.sendBuffer(kBtGTimbreParams, timbreBuffer);
			for (int i = 0; i < 4; i++)
				timbreBuffer[2*i] = 0;
			gDevNote.sendToGui(gui);
			Bela_scheduleAuxiliaryTask(gGraphTask);
			Bela_scheduleAuxiliaryTask(gFFTTask);

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
		// out = gDevSpecDens.process();
		
		// call note process
		out = gDevNote.process(gui, true);
		// log output to oscilloscope
		gScope.log(out);
		
		//check note fft
		// if (gDevNote.checkFftReady())
		// 	Bela_scheduleAuxiliaryTask(gFFTTask);
		
		// Write output to all audio out channels
		for (unsigned int i = 0; i < context->audioOutChannels; i++)
			audioWrite(context, n, i, out);
	}
}

void cleanup(BelaContext *context, void *userData)
{
	
}