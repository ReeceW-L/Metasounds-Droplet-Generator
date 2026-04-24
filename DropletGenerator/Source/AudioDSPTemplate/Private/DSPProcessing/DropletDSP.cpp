#include "DSPProcessing/DropletDSP.h"
#include "Math/UnrealMathUtility.h"

namespace DSPProcessing
{
	//Clamps Rain Intensity to prevent overloads
	void FDroplet::SetRainIntensity(float Intensity)
	{
		RainIntensity = FMath::Clamp(Intensity, 0.0f, 1.5f);
	}

	//Clamps Surface value to below 1 to prevent filter feeding back 
	void FDroplet::SetSurface(float Surface)
	{
		DropletSurface = FMath::Clamp(Surface, 0.0f, 0.98f);
	}
	
	void FDroplet::Init(float InSampleRate)
	{
		SampleRate = InSampleRate;

		//Initialize filter buffers
		Buf0 = 0.0f;
		Buf1 = 0.0f;

		NoiseComponent = 0.0f;
	}

	//Generates random time interval between droplets 
	float FDroplet::GenerateRandomInterval() 
	{ 
		float IntensityReverse = 1.0f - RainIntensity;
		return FMath::FRandRange(0.1f, IntensityReverse); 
	}

	//Generates a random frequency for the droplet when trigger is called
	float FDroplet::GenerateRandomFreq()
	{
		// Base ranges
		float const MinSoft = 400.0f;
		float const MaxSoft = 1600.0f;

		float const MinHard = 1000.0f;
		float const MaxHard = 1200.0f;
		// Interpolate ranges based on surface
		float MinFreq = FMath::Lerp(MinSoft, MinHard, DropletSurface);
		float MaxFreq = FMath::Lerp(MaxSoft, MaxHard, DropletSurface);

		return FMath::FRandRange(MinFreq, MaxFreq);
	}

	//Triggers envelopes and resets sine wave
	//Currently only allows generation of 1 droplet at a time - needs to be expanded
	void FDroplet::Trigger(float InFrequency, float InReleaseTime)
	{
		Phase = 0.0f;
		AmpEnv = 1.0f;
		PitchEnv = 1.0f;

		AmpRelease = FMath::Max(0.001f, InReleaseTime);
		PhaseIncrement = (2.0f * PI * InFrequency) / SampleRate;

		Buf0 = 0.0f;
		Buf1 = 0.0f;

		bActive = true;
	}

	//Resonant Filter for surface modulation taken from https://www.musicdsp.org/en/latest/Filters/29-resonant-filter.html
	float FDroplet::FilterProcess(float In, float Cutoff)
	{
		float f = 2.0f * FMath::Sin(PI * Cutoff / SampleRate);
		if (f > 0.9f) f = 0.9f;

		// Surface parameter controls resonance
		float fb = FMath::Lerp(0.3f, 0.65f, DropletSurface);

		Buf0 += f * (In - Buf0 + fb * (Buf0 - Buf1));
		Buf1 += f * (Buf0 - Buf1);

		Buf0 = FMath::Clamp(Buf0, -3.0f, 3.0f);
		Buf1 = FMath::Clamp(Buf1, -3.0f, 3.0f);

		return Buf1;
	}

	float FDroplet::GenerateNoise()
	{
		float NoiseAmount = FMath::Lerp(0.05f, 0.4f, DropletSurface);
		float Noise = FMath::FRandRange(-1.0f, 1.0f);

		float NoiseEnv = AmpEnv * AmpEnv;
		return Noise * NoiseAmount * NoiseEnv;
	}

	//This is where the audio gen be happening
	void FDroplet::ProcessAudioBuffer(float* OutBuffer, int32 NumSamples)
	{
		float SurfaceDecayScale = FMath::Lerp(0.5f, 1.8f, DropletSurface);
		const float AmpDecay = 1.0f / (AmpRelease * SurfaceDecayScale * SampleRate);

		float PitchDecayScale = FMath::Lerp(0.5f, 2.5f, DropletSurface);
		const float PitchDecay = 1.0f / (0.02f * PitchDecayScale * SampleRate);

		for (int32 i = 0; i < NumSamples; ++i)
		{
			{
				//Count down to trigger
				SamplesUntilTrigger--;

				if (SamplesUntilTrigger <= 0.0f)
				{
					Trigger(GenerateRandomFreq(), AmpRelease);
					SamplesUntilTrigger = (SampleRate * GenerateRandomInterval());
				}
			}

			//Droplet Generate
			if (!bActive) {
				OutBuffer[i] = 0.0f;
				continue;
			}

			float PitchMod = PitchEnv * PitchEnv * PitchEnv;
			float Tone = FMath::Sin(Phase + (PitchMod * 40.0f));

			//Note on surface parameter, 0.0 is meant for soft surfaces, 1.0 for hard surfaces
			NoiseComponent = GenerateNoise();

			float Curve = FMath::Lerp(4.0f, 1.5f, DropletSurface);
			float Amp = FMath::Pow(AmpEnv, Curve);

			float Sample = (Tone * Amp + NoiseComponent) * 0.7f;

			 OutBuffer[i] = FilterProcess(Sample, 3000.0f);

			//Increment sine wave
			Phase += PhaseIncrement;
			if (Phase > 2.0f * PI)
				Phase -= 2.0f * PI;

			//Decay Envelopes
			AmpEnv -= AmpDecay;
			PitchEnv -= PitchDecay;

			//Turn off the droplet when amp is finished
			if (AmpEnv <= 0.0f)
			{
				bActive = false;
			}
		}
	}
}
