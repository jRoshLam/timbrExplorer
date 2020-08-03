// ECS7012P Music and Audio Programming
// School of Electronic Engineering and Computer Science
// Queen Mary University of London
// Spring 2020

// Operator.h: header file for wavetable operator class

#include <array>
#include <vector>

#define WAVETABLE_SIZE 512

static const std::vector<std::vector<float>> dummyTable;

class Operator {
public:
	Operator();													// Default constructor
	// Operator(float sampleRate, float *tableData, int tableLength); // Constructor with arguments
	// void setup(float sampleRate, float *tableData, int tableLength); // Set parameters
	Operator(float sampleRate, const std::vector<std::vector<float>>& tableData); // Constructor with arguments
	void setup(int tableIndex, float frequency); // Set parameters
	
	void setSampleRate(float f);
	
	void reset();
	
	void setAmplitude(float a);	// Set the operator amplitude
	void setFrequency(float f);	// Set the operator frequency
	void setTable(int i);		// Set the opeartor wavetable
	void setParameters(float amplitude, float frequency, int table);
	
	float amplitude();			// Get the operator amplitude
	float modAmplitude();		// Get the operator modulator amplitude
	float frequency();			// Get the operator frequency
	float tableLength();
	float phaseIncr();
	float lengthXinvSampleRate();
	float debugValue();
	
	float modulationPhase(float modulation);					// obtain modulation value to add to a carrier phase
	
	// float modulatedProcess(Operator modulator);	// process with a modulator operator
	
	float process(float modulation);			// Get the next sample and update the phase
	
	~Operator();				// Destructor

private:
	float sampleRate_;			// Sample rate of the audio
	// std::array<float, WAVETABLE_SIZE> *table_;	// Wavetable data
	const std::vector<std::vector<float>>& table_;
	// float *table_;
	int currentTable_;
	float tableLength_;			// Length of the wavetable
	float lengthXinvSampleRate_;
	float amplitude_;			// Normal amplitude of operator
	float modAmplitude_;		// Amplitude if operator is modulating
	float frequency_;			// Frequency of the operator
	
	float phase_;				// Phase of the operator
	float phaseIncr_;
	
	unsigned int lastFrameCount_;
	float lastOutput_;
	float invTwoPi;
	
};