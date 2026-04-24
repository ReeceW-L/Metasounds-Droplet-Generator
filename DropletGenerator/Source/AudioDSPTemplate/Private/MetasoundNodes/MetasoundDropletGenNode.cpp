#include "MetasoundNodes/MetasoundDropletGenNode.h"

#define LOCTEXT_NAMESPACE "DSPTemplate_DropletGenerator"

namespace DSPTemplate
{
	using namespace Metasound;



	namespace VolumeNode
	{
		//Input params
		
		METASOUND_PARAM(InParamNameAmplitude,  "Amplitude", "The amount of amplitude to apply to the input signal.")
		METASOUND_PARAM(InParamNameRainIntensity, "RainIntensity", "Parameter to change droplet frequency and intensity.")
		METASOUND_PARAM(InParamNameSurface, "Surface", "Parameter to change droplet surface.")
		
		//Output params
		METASOUND_PARAM(OutParamNameAudio, "Out", "Audio output.")
	}

	//Constructor - initializes variables
	FDropletOperator::FDropletOperator(const FOperatorSettings& InSettings, const FFloatReadRef& InAmplitude, const FFloatReadRef& InRainIntensity, const FFloatReadRef& InSurface)
		
		: AudioOutput(FAudioBufferWriteRef::CreateNew(InSettings))
		, Amplitude(InAmplitude)
		, RainIntensity(InRainIntensity)
		, Surface(InSurface)
	{
		SampleRate = InSettings.GetSampleRate();
		
		
	}

	//Node Metadate for registration 
	const FNodeClassMetadata& DSPTemplate::FDropletOperator::GetNodeInfo()
	{
		auto InitNodeInfo = []() -> FNodeClassMetadata
		{
			FNodeClassMetadata Info;

			Info.ClassName         = { TEXT("UE"), TEXT("DropletGenerator"), TEXT("Audio") };
			Info.MajorVersion      = 1;
			Info.MinorVersion      = 0;
			Info.DisplayName       = LOCTEXT("DSPTemplate_DropletGenerator", "DropletGenerator");
			Info.Description       = LOCTEXT("DSPTemplate_DropletGeneratorDescription", "Generates droplet sfx.");
			Info.Author            = "Put your name here!.";
			Info.PromptIfMissing   = PluginNodeMissingPrompt;
			Info.DefaultInterface  = GetVertexInterface();
			Info.CategoryHierarchy = { LOCTEXT("DSPTemplate_DropletGeneratorCategory", "Utils") };

			return Info;
		};

		static const FNodeClassMetadata Info = InitNodeInfo();

		return Info;
	}

	//Input bindings to be used in execute
	void FDropletOperator::BindInputs(FInputVertexInterfaceData& InOutVertexData)
	{
		using namespace VolumeNode;

		InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InParamNameRainIntensity), RainIntensity);
		InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InParamNameAmplitude),  Amplitude);
		InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InParamNameSurface), Surface);
	}

	//Output bindings to be used in execute
	void FDropletOperator::BindOutputs(FOutputVertexInterfaceData& InOutVertexData)
	{
		using namespace VolumeNode;

		InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(OutParamNameAudio), AudioOutput);
	}

	const FVertexInterface& FDropletOperator::GetVertexInterface()
	{
		using namespace VolumeNode;

		static const FVertexInterface Interface(
			FInputVertexInterface(
				
				TInputDataVertex<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameRainIntensity), 1.0f),
				TInputDataVertex<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameAmplitude), 1.0f),
				TInputDataVertex<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameSurface), 1.0f)
			),

			FOutputVertexInterface(
				TOutputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNameAudio))
			)
		);

		return Interface;
	}

	TUniquePtr<IOperator> FDropletOperator::CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults)
	{
		using namespace VolumeNode;

		FFloatReadRef InAmplitude = InParams.InputData.GetOrCreateDefaultDataReadReference<float>(METASOUND_GET_PARAM_NAME(InParamNameAmplitude), InParams.OperatorSettings);
		FFloatReadRef InRainIntensity = InParams.InputData.GetOrCreateDefaultDataReadReference<float>(METASOUND_GET_PARAM_NAME(InParamNameRainIntensity), InParams.OperatorSettings);
		FFloatReadRef InSurface = InParams.InputData.GetOrCreateDefaultDataReadReference<float>(METASOUND_GET_PARAM_NAME(InParamNameSurface), InParams.OperatorSettings);

		return MakeUnique<FDropletOperator>(InParams.OperatorSettings, InAmplitude, InRainIntensity, InSurface);
	}

	void FDropletOperator::Execute()
	{
		float* OutputAudio = AudioOutput->GetData();
		const int32 NumSamples = AudioOutput->Num();

		AudioOutput->Zero();

		DropletDSPProcessor.SetRainIntensity(*RainIntensity);
		DropletDSPProcessor.SetSurface(*Surface);

		//Generate audio
		DropletDSPProcessor.ProcessAudioBuffer(OutputAudio, NumSamples);

		
		//Applies gain (reused from original template)
		const float Gain = *Amplitude;
		if (Gain != 1.0f)
		{
			for (int32 i = 0; i < NumSamples; ++i)
			{
				OutputAudio[i] *= Gain;
			}
		}
	}

	//Reset
	void FDropletOperator::Reset(const IOperator::FResetParams& InParams)
	{
		AudioOutput->Zero();
		DropletDSPProcessor.Init(SampleRate);

		
	}

	METASOUND_REGISTER_NODE(FDropletNode)
}

#undef LOCTEXT_NAMESPACE
