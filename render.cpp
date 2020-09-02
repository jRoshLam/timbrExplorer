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
// graph calculation callback to be made into an auxiliary task
void process_graphs_background(void*);

// Timbre ==========================================================
// Note object
// Initialized as global object with default constructor
Note gDevNote;
// Array for storing current timbre values
// {spectrum, brightness, articulation, envelope}
int gTimbreDim[4] = {0};
// Buffer to send to GUI to update dimension values
// {spFlag, spectrum, brFlag, brightness, arFlag, articulation, enFlag, envelope}
int gTimbreBuffer[8] = {0};

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

// wrapper function to feed to Bela_createAuxiliaryTask
void process_fft_background(void*)
{
	gDevNote.outputFft(gui);
}

// wrapper function to feed to Bela_createAuxiliaryTask
void process_graphs_background(void*)
{
	gDevNote.updateGraphs(gui);
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
	gTimbreDim[0] = MAX_SPECTRUM / 2 - 1;
	gTimbreDim[1] = MAX_BRIGHTNESS / 2 - 1;
	gTimbreDim[2] = MAX_ARTICULATION / 2 - 1;
	gTimbreDim[3] = MAX_ENVELOPE / 2 - 1;
	gDevNote.setSpectrum(gTimbreDim[0]);
	gDevNote.setEnvelope(gTimbreDim[1]);
	gDevNote.setArticulation(gTimbreDim[2]);
	gDevNote.setBrightness(gTimbreDim[3]);
	
	
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
	
	return true;
}

void render(BelaContext *context, void *userData)
{
	// Update Timbre =============================================================
	// frame count for sending data to GUI
	static unsigned int frameCount = 0;
	
	// set timbre parameters using the GUI
	DataBuffer& buffer = gui.getDataBuffer(kGtBTimbreParams);
	float* data = buffer.getAsFloat();
	// loop through update buffer and only update if the update flag is high
	for (int i = 0; i < 4; i++)
	{
		// check update flag
		if (data[2*i] == 1) {
			gTimbreDim[i] = int(data[2*i+1]);
			// set corresponding dimension depending on i
			switch(i) 
			{
				case 0: gDevNote.setSpectrum(gTimbreDim[0]); break;
				case 1: gDevNote.setBrightness(gTimbreDim[1]); break;
				case 2: gDevNote.setArticulation(gTimbreDim[2]); break;
				case 3: gDevNote.setEnvelope(gTimbreDim[3]); break;
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
		gTimbreDim[0] = int(map(specFilt.process(gSpecTouchPosition[1]), 0, 1, 0, MAX_SPECTRUM-1));
		gTimbreDim[1] = int(map(brightFilt.process(gSpecTouchPosition[0]), 0, 1, 0, MAX_BRIGHTNESS-1));

		gDevNote.setSpectrum(gTimbreDim[0]);
		gDevNote.setBrightness(gTimbreDim[1]);

		// update send buffer for spectrum and brightness
		// set update flags to 1 to tell gui to update
		gTimbreBuffer[0] = 1;
		gTimbreBuffer[1] = gTimbreDim[0];
		gTimbreBuffer[2] = 1;
		gTimbreBuffer[3] = gTimbreDim[1];
	}
	// Only update Timbre if filtered touch size is above threshold
	if (gDynCurrentTouch > gTouchThreshold)
	{
		// Save position to send to GUI
		gCurrentDynPosition[0] = gDynTouchPosition[0];
		gCurrentDynPosition[1] = gDynTouchPosition[1];
		
		//filter positions and map them to corresponding parameter range
		gTimbreDim[2] = int(map(articFilt.process(gDynTouchPosition[1]), 0, 1, 0, MAX_ARTICULATION-1));
		gTimbreDim[3] = int(map(envFilt.process(gDynTouchPosition[0]), 0, 1, 0, MAX_ENVELOPE-1));

		gDevNote.setArticulation(gTimbreDim[2]);
		gDevNote.setEnvelope(gTimbreDim[3]);

		// update send buffer for articulation and envelope
		// set update flags to 1 to tell gui to update
		gTimbreBuffer[4] = 1;
		gTimbreBuffer[5] = gTimbreDim[2];
		gTimbreBuffer[6] = 1;
		gTimbreBuffer[7] = gTimbreDim[3];
	}
	
	// Advanced Controls =============================================================
	// retrieve advanced buffer and convert to float data
	DataBuffer& advBuffer = gui.getDataBuffer(kGtBAdvControls);
	float* advData = advBuffer.getAsFloat();
	
	// update advanced mode and controls for the note object
	gDevNote.setAdvMode(advData[kACIAdvMode]);
	gDevNote.setAdvControls(advData);
	
	// advanced FM spectrum buffer and control - retrieve, convert, update
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
	
	// Audio Block Loop ==========================================================
	float out = 0;
	for (unsigned int n = 0; n < context->audioFrames; n++)
	{
		// Send buffers to GUI at fixed intervals
		if(frameCount >= gGuiPeriod*context->audioSampleRate)
		{
			//send timbre paramters
			gui.sendBuffer(kBtGTimbreParams, gTimbreBuffer);
			//after sending timbre buffer always reset update flags to 0
			for (int i = 0; i < 4; i++)
				gTimbreBuffer[2*i] = 0;
			// send assorted envelope and spectrum info to the GUI
			gDevNote.sendToGui(gui);
			// schedule task to calculate brightness FRF and articulation graph and then send them to the GUI
			Bela_scheduleAuxiliaryTask(gGraphTask);
			// schedule tasks to calculate raw and final spectrum FFTs and send them to the GUI
			Bela_scheduleAuxiliaryTask(gFFTTask);

			frameCount = 0;
		}
		frameCount ++;
		
		// call note process
		out = gDevNote.process(gui, true);
		// log output to oscilloscope
		gScope.log(out);
		
		// Write output to all audio out channels
		for (unsigned int i = 0; i < context->audioOutChannels; i++)
			audioWrite(context, n, i, out);
	}
}

void cleanup(BelaContext *context, void *userData)
{
	
}