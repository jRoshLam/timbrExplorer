// ECS7012P Music and Audio Programming
// School of Electronic Engineering and Computer Science
// Queen Mary University of London
// Spring 2020

// Operator.h: header file for wavetable operator class

#include <array>
#include <vector>

#define WAVETABLE_SIZE 512

// Dummy table for default constructor
static const std::vector<std::vector<float>> dummyTable;

class Operator {
public:
	Operator();	// Default constructor
	Operator(float sampleRate, const std::vector<std::vector<float>>& tableData); // Constructor with arguments
	void setup(int tableIndex, float frequency); // Set parameters
	
	// set sample rate
	void setSampleRate(float frequency);
	
	// reset phase
	void reset();
	
	void setAmplitude(float amplitude);	// Set the operator amplitude
	void setFrequency(float frequency);	// Set the operator frequency
	void setTable(int waveShapeEnum);		// Set the opeartor wavetable
	void setParameters(float amplitude, float frequency, int waveShapeEnum);
	
	// Getters for debugging
	float amplitude();			// Get the operator amplitude
	float modAmplitude();		// Get the operator modulator amplitude
	float frequency();			// Get the operator frequency
	float tableLength();		// Get the length of current wavetable
	float phaseIncr();			// Get the current phase increment
	float lengthXinvSampleRate(); 
	float debugValue();			// Get a debug wave sample
	
	// obtain modulation value to add to a carrier phase
	float modulationPhase(float modulation);
	
	// Get the next sample and update the phase
	float process(float modulation);
	
	~Operator(); // Destructor

private:
	float sampleRate_;			// Sample rate of the audio
	
	// Const reference to vector of wavetables (vector of vectors)
	// Use a const reference so each operator doesn't need to make a copy of the vector
	const std::vector<std::vector<float>>& table_;

	// operator parameters
	int currentTable_;			// Index of current table
	float tableLength_;			// Length of the wavetable
	float amplitude_;			// Normal amplitude of operator
	float modAmplitude_;		// Amplitude if operator is modulating
	float frequency_;			// Frequency of the operator
	
	// Commonly needed inverted vlues to be used for multiplication instead of division
	float lengthXinvSampleRate_;// Length / sampleRate
	float invTwoPi_;			// 1 / (2*pi)
	
	// Operator state information
	float phase_;				// Phase of the operator
	float phaseIncr_;			// Amount to increment pahse by each frame (determined by frequency)
	float lastOutput_;			// Operator's output from previous frame
	
	
};