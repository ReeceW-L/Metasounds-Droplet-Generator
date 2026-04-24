#pragma once

#include "HAL/Platform.h"

namespace DSPProcessing
{
	class FDroplet
	{
	public:
		void Init(float InSampleRate);

		//Trigger
		void Trigger(float InFrequency, float InReleaseTime);

		//Randomisation
		float GenerateRandomInterval();
		float GenerateRandomFreq();

		float GenerateNoise();

		void SetRainIntensity(float Intensity);
		void SetSurface(float Surface);

		float FilterProcess(float In, float Cutoff);

		void ProcessAudioBuffer(float* OutBuffer, int32 NumSamples);
		bool IsActive() const { return bActive; }

		float RainIntensity = 1.0f;
		float DropletSurface = 0.5f;

	private:
		float SampleRate = 48000.0f;
		float SamplesUntilTrigger = 0.0f;

		//Oscillator
		float Phase = 0.0f;
		float PhaseIncrement = 0.0f;

		//Envelopes
		float AmpEnv = 0.0f;
		float PitchEnv = 0.0f;
		float AmpRelease = 0.05f;

		//Filter Params
		float Buf0 = 0.0f;
		float Buf1 = 0.0f;

		//Noise Component
		float NoiseComponent = 0.0f;

		bool bActive = false;

		
	};
}
