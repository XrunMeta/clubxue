

#if WITH_EDITOR

#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Tests/AutomationCommon.h"
#include "Misc/Paths.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformProcess.h"
#include "Framework/Application/SlateApplication.h"

#include "ArmASR.h"

class FSetConsoleVariableLatentCommand : public IAutomationLatentCommand
{
public:

	FSetConsoleVariableLatentCommand(const FString& InConsoleVarName, float InValue)
		: ConsoleVarName(InConsoleVarName)
		, Value(InValue)
		, bHasSet(false)
	{}

	virtual bool Update() override
	{
		if (!bHasSet)
		{

			IConsoleVariable* ConsoleVar = IConsoleManager::Get().FindConsoleVariable(*ConsoleVarName);
			if (ConsoleVar)
			{

				ConsoleVar->Set(Value, ECVF_SetByConsole);
				UE_LOG(LogTemp, Log, TEXT("Set console variable '%s' to %f."), *ConsoleVarName, Value);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Console variable '%s' not found."), *ConsoleVarName);
			}
			bHasSet = true;
		}

		return true;
	}

private:
	FString ConsoleVarName;
	float Value;
	bool bHasSet;
};

class FTakeScreenshotLatentCommand : public IAutomationLatentCommand
{
public:

	FTakeScreenshotLatentCommand(const FString& InScreenshotName, float InDelay = 1.0f)
		: ScreenshotName(InScreenshotName)
		, Delay(InDelay)
		, bScreenshotRequested(false)
		, StartTime(0.0)
	{}

	virtual bool Update() override
	{
		if (!bScreenshotRequested)
		{

			StartTime = FPlatformTime::Seconds();

			FScreenshotRequest::RequestScreenshot(*(ScreenshotName + TEXT(".png")), false, false);
			bScreenshotRequested = true;

			UE_LOG(LogTemp, Log, TEXT("Screenshot requested: %s.png"), *ScreenshotName);
		}

		double ElapsedTime = FPlatformTime::Seconds() - StartTime;
		return (ElapsedTime > Delay);
	}

private:
	FString ScreenshotName;
	float Delay;
	bool bScreenshotRequested;
	double StartTime;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FArmASREnableTest,
	"ArmASR.PluginTests.EnablePluginTest",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext
	| EAutomationTestFlags::ServerContext | EAutomationTestFlags::CommandletContext
	| EAutomationTestFlags::EngineFilter | EAutomationTestFlags::NonNullRHI)

bool FArmASREnableTest::RunTest(const FString& Parameters)
{

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("ShowFlag.VisualizeTemporalUpscaler"), true));

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.AntiAliasingMethod"), 2));
	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.ArmASR.Enable"), false));

	const FString MapName = "/Game/_Game/ThirdPerson/ThirdPerson";
	if (!AutomationOpenMap(MapName))
	{
		AddError(FString::Printf(TEXT("Failed to open map %s"), *MapName));
		return false;
	}

	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(5.0f));

	TSharedPtr<SWindow> CurrentWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
	ADD_LATENT_AUTOMATION_COMMAND(FTakeEditorScreenshotCommand({ TEXT("ArmASR_EnablePluginTest_before.png"), CurrentWindow }));

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.ArmASR.Enable"), true));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FTakeEditorScreenshotCommand({ TEXT("ArmASR_EnablePluginTest_after.png"), CurrentWindow }));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FArmASRQualityPresetTest,
	"ArmASR.PluginTests.QualityPresetTest",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext
	| EAutomationTestFlags::ServerContext | EAutomationTestFlags::CommandletContext
	| EAutomationTestFlags::EngineFilter | EAutomationTestFlags::NonNullRHI)

bool FArmASRQualityPresetTest::RunTest(const FString& Parameters)
{

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("ShowFlag.VisualizeTemporalUpscaler"), true));

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.AntiAliasingMethod"), 2));
	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.ArmASR.Enable"), true));

	const FString MapName = "/Game/_Game/ThirdPerson/ThirdPerson";
	if (!AutomationOpenMap(MapName))
	{
		AddError(FString::Printf(TEXT("Failed to open map %s"), *MapName));
		return false;
	}

	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(5.0f));

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.ArmASR.ShaderQuality"), 1));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FTakeActiveEditorScreenshotCommand(TEXT("ArmASR_ShaderQualityTest_Quality.png")));

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.ArmASR.ShaderQuality"), 2));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FTakeActiveEditorScreenshotCommand(TEXT("ArmASR_ShaderQualityTest_Balanced.png")));

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.ArmASR.ShaderQuality"), 3));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FTakeActiveEditorScreenshotCommand(TEXT("ArmASR_ShaderQualityTest_Performance.png")));

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.ArmASR.ShaderQuality"), 4));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FTakeActiveEditorScreenshotCommand(TEXT("ArmASR_ShaderQualityTest_UltraPerformance.png")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FArmASRScreenPercentageTest,
	"ArmASR.PluginTests.UpscaleRatioTest",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext
	| EAutomationTestFlags::ServerContext | EAutomationTestFlags::CommandletContext
	| EAutomationTestFlags::EngineFilter | EAutomationTestFlags::NonNullRHI)

bool FArmASRScreenPercentageTest::RunTest(const FString& Parameters)
{

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("ShowFlag.VisualizeTemporalUpscaler"), true));

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.AntiAliasingMethod"), 2));
	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.ArmASR.Enable"), true));

	const FString MapName = "/Game/_Game/ThirdPerson/ThirdPerson";
	if (!AutomationOpenMap(MapName))
	{
		AddError(FString::Printf(TEXT("Failed to open map %s"), *MapName));
		return false;
	}

	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(5.0f));

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.ScreenPercentage"), 100));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FTakeActiveEditorScreenshotCommand(
		TEXT("ArmASR_ScreenPercentageTest_100.png")));

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.ScreenPercentage"), 50));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FTakeActiveEditorScreenshotCommand(
		TEXT("ArmASR_ScreenPercentageTest_50.png")));

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.ScreenPercentage"), 67));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FTakeActiveEditorScreenshotCommand(
		TEXT("ArmASR_ScreenPercentageTest_67.png")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FArmASRSharpnessTest,
	"ArmASR.PluginTests.SharpnessTest",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext
	| EAutomationTestFlags::ServerContext | EAutomationTestFlags::CommandletContext
	| EAutomationTestFlags::EngineFilter | EAutomationTestFlags::NonNullRHI)

bool FArmASRSharpnessTest::RunTest(const FString& Parameters)
{

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("ShowFlag.VisualizeTemporalUpscaler"), true));

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.AntiAliasingMethod"), 2));
	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.ArmASR.Enable"), true));

	const FString MapName = "/Game/_Game/ThirdPerson/ThirdPerson";
	if (!AutomationOpenMap(MapName))
	{
		AddError(FString::Printf(TEXT("Failed to open map %s"), *MapName));
		return false;
	}

	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(5.0f));

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.ArmASR.Sharpness"), 0.0f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FTakeActiveEditorScreenshotCommand(TEXT("ArmASR_SharpnessTest_0_0.png")));

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.ArmASR.Sharpness"), 0.24f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FTakeActiveEditorScreenshotCommand(TEXT("ArmASR_SharpnessTest_0_24.png")));

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.ArmASR.Sharpness"), 0.24f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FTakeActiveEditorScreenshotCommand(TEXT("ArmASR_SharpnessTest_0_24.png")));

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.ArmASR.Sharpness"), 0.63f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FTakeActiveEditorScreenshotCommand(TEXT("ArmASR_SharpnessTest_0_63.png")));

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.ArmASR.Sharpness"), 1.0f));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FTakeActiveEditorScreenshotCommand(TEXT("ArmASR_SharpnessTest_1_0.png")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FArmASRReactiveMaskTest,
	"ArmASR.PluginTests.ReactiveMaskTest",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext
	| EAutomationTestFlags::ServerContext | EAutomationTestFlags::CommandletContext
	| EAutomationTestFlags::EngineFilter | EAutomationTestFlags::NonNullRHI)

bool FArmASRReactiveMaskTest::RunTest(const FString& Parameters)
{

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("ShowFlag.VisualizeTemporalUpscaler"), true));

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.AntiAliasingMethod"), 2));
	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.ArmASR.Enable"), true));

	const FString MapName = "/Game/_Game/ThirdPerson/ThirdPerson";
	if (!AutomationOpenMap(MapName))
	{
		AddError(FString::Printf(TEXT("Failed to open map %s"), *MapName));
		return false;
	}

	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(5.0f));

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.ArmASR.CreateReactiveMask"), false));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FTakeActiveEditorScreenshotCommand(TEXT("ArmASR_ReactiveMaskTest_off.png")));

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.ArmASR.CreateReactiveMask"), true));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FTakeActiveEditorScreenshotCommand(TEXT("ArmASR_ReactiveMaskTest_on.png")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FArmASRFilmGrainTest,
	"ArmASR.PluginTests.FilmGrainTest",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext
	| EAutomationTestFlags::ServerContext | EAutomationTestFlags::CommandletContext
	| EAutomationTestFlags::EngineFilter | EAutomationTestFlags::NonNullRHI)

bool FArmASRFilmGrainTest::RunTest(const FString &Parameters)
{

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("ShowFlag.VisualizeTemporalUpscaler"), true));

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.AntiAliasingMethod"), 2));
	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.ArmASR.Enable"), true));

	const FString MapName = "/Game/_Game/ThirdPerson/ThirdPerson";
	if (!AutomationOpenMap(MapName))
	{
		AddError(FString::Printf(TEXT("Failed to open map %s"), *MapName));
		return false;
	}

	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(5.0f));

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.FilmGrain"), true));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FTakeActiveEditorScreenshotCommand(TEXT("ArmASR_FilmGrain_on.png")));

	ADD_LATENT_AUTOMATION_COMMAND(FSetConsoleVariableLatentCommand(TEXT("r.FilmGrain"), false));

	return true;
}

#endif
