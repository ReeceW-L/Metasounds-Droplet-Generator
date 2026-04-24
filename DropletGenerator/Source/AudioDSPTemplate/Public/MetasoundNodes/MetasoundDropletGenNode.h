#pragma once

#include "DSPProcessing/DropletDSP.h"
#include "MetasoundEnumRegistrationMacro.h"
#include "MetasoundParamHelper.h"
#include "MetasoundExecutableOperator.h"
#include "MetasoundFacade.h"

namespace DSPTemplate
{
	class FDropletOperator : public Metasound::TExecutableOperator<FDropletOperator>
	{
	public:
		//This is the constructor for the operator
		FDropletOperator(
			const Metasound::FOperatorSettings& InSettings,
			const Metasound::FFloatReadRef& InAmplitude,
			const Metasound::FFloatReadRef& InRainIntensity,
			const Metasound::FFloatReadRef& InSurface);

		static const Metasound::FNodeClassMetadata& GetNodeInfo();

		virtual void BindInputs(Metasound::FInputVertexInterfaceData& InOutVertexData) override;
		virtual void BindOutputs(Metasound::FOutputVertexInterfaceData& InOutVertexData) override;

		static const Metasound::FVertexInterface& GetVertexInterface();
		static TUniquePtr<Metasound::IOperator> CreateOperator(
			const Metasound::FBuildOperatorParams& InParams,
			Metasound::FBuildResults& OutResults);

		void Execute();
		void Reset(const IOperator::FResetParams& InParams);

	private:
		//Parameter Input
		Metasound::FFloatReadRef RainIntensity;
		Metasound::FFloatReadRef Surface;

		//Audio Output
		Metasound::FAudioBufferWriteRef AudioOutput;

		//This is where the DSP processing class is made
		DSPProcessing::FDroplet DropletDSPProcessor;

		//Output Gain
		Metasound::FFloatReadRef Amplitude;

		float SampleRate;
	};

	using FDropletNode = Metasound::TNodeFacade<FDropletOperator>;
}