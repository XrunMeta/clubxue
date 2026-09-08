

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Canvas.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Console.h"
#include "Containers/UnrealString.h"
#include "Misc/OutputDeviceHelper.h"
#include "VRLogComponent.generated.h"

UENUM(BlueprintType)
enum class EBPVRConsoleDrawType : uint8
{
	VRConsole_Draw_ConsoleOnly,
	VRConsole_Draw_OutputLogOnly

};

struct FVRLogMessage
{
	TSharedRef<FString> Message;
	ELogVerbosity::Type Verbosity;
	FName Category;
	FName Style;

	FVRLogMessage(const TSharedRef<FString>& NewMessage, FName NewCategory, FName NewStyle = NAME_None)
		: Message(NewMessage)
		, Verbosity(ELogVerbosity::Log)
		, Category(NewCategory)
		, Style(NewStyle)
	{
	}

	FVRLogMessage(const TSharedRef<FString>& NewMessage, ELogVerbosity::Type NewVerbosity, FName NewCategory, FName NewStyle = NAME_None)
		: Message(NewMessage)
		, Verbosity(NewVerbosity)
		, Category(NewCategory)
		, Style(NewStyle)
	{
	}
};

class FVROutputLogHistory : public FOutputDevice
{
public:

	int32 MaxStoredMessages;
	bool bIsDirty;
	int32 MaxLineLength;

	FVROutputLogHistory()
	{
		MaxLineLength = 130;
		bIsDirty = false;
		MaxStoredMessages = 1000;
		GLog->AddOutputDevice(this);

	}

	~FVROutputLogHistory()
	{

		if (GLog != NULL)
		{
			GLog->RemoveOutputDevice(this);
		}
	}

	const TArray< TSharedPtr<FVRLogMessage> >& GetMessages() const
	{
		return Messages;
	}

protected:

	virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category) override
	{

		CreateLogMessages(V, Verbosity, Category, Messages);
	}

	bool CreateLogMessages(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category, TArray< TSharedPtr<FVRLogMessage> >& OutMessages)
	{
		if (Verbosity == ELogVerbosity::SetColor)
		{

			return false;
		}
		else
		{
			FName Style;
			if (Category == NAME_Cmd)
			{
				Style = FName(TEXT("Log.Command"));
			}
			else if (Verbosity == ELogVerbosity::Error)
			{
				Style = FName(TEXT("Log.Error"));
			}
			else if (Verbosity == ELogVerbosity::Warning)
			{
				Style = FName(TEXT("Log.Warning"));
			}
			else
			{
				Style = FName(TEXT("Log.Normal"));
			}

			static ELogTimes::Type LogTimestampMode = ELogTimes::None;

			const int32 OldNumMessages = OutMessages.Num();

			TArray<FTextRange> LineRanges;
			FString CurrentLogDump = V;
			FTextRange::CalculateLineRangesFromString(CurrentLogDump, LineRanges);

			bool bIsFirstLineInMessage = true;
			for (const FTextRange& LineRange : LineRanges)
			{
				if (!LineRange.IsEmpty())
				{
					FString Line = CurrentLogDump.Mid(LineRange.BeginIndex, LineRange.Len());
					Line = Line.ConvertTabsToSpaces(4);

					int32 HardWrapLen = MaxLineLength;
					for (int32 CurrentStartIndex = 0; CurrentStartIndex < Line.Len();)
					{
						int32 HardWrapLineLen = 0;
						if (bIsFirstLineInMessage)
						{
							FString MessagePrefix = FOutputDeviceHelper::FormatLogLine(Verbosity, Category, nullptr, LogTimestampMode);

							HardWrapLineLen = FMath::Min(HardWrapLen - MessagePrefix.Len(), Line.Len() - CurrentStartIndex);
							FString HardWrapLine = Line.Mid(CurrentStartIndex, HardWrapLineLen);

							OutMessages.Add(MakeShareable(new FVRLogMessage(MakeShareable(new FString(MessagePrefix + HardWrapLine)), Verbosity, Category, Style)));
						}
						else
						{
							HardWrapLineLen = FMath::Min(HardWrapLen, Line.Len() - CurrentStartIndex);
							FString HardWrapLine = Line.Mid(CurrentStartIndex, HardWrapLineLen);

							OutMessages.Add(MakeShareable(new FVRLogMessage(MakeShareable(new FString(MoveTemp(HardWrapLine))), Verbosity, Category, Style)));
						}

						bIsFirstLineInMessage = false;
						CurrentStartIndex += HardWrapLineLen;
					}
				}
			}

			int numMessages = OutMessages.Num();
			if (numMessages > MaxStoredMessages)
			{
				OutMessages.RemoveAt(0, numMessages - MaxStoredMessages, EAllowShrinking::Yes);
			}
			if (OldNumMessages != numMessages)
				bIsDirty = true;

			return OldNumMessages != numMessages;
		}
	}

private:

	TArray< TSharedPtr<FVRLogMessage> > Messages;
};

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = (VRExpansionPlugin))
class VREXPANSIONPLUGIN_API UVRLogComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVRLogComponent(const FObjectInitializer& ObjectInitializer);

	~UVRLogComponent();

	FVROutputLogHistory OutputLogHistory;

	virtual void PostInitProperties() override
	{
		Super::PostInitProperties();
		OutputLogHistory.MaxStoredMessages = FMath::Clamp(MaxStoredMessages, 100, 100000);
		OutputLogHistory.MaxLineLength = FMath::Clamp(MaxLineLength, 50, 1000);
	}

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "VRLogComponent|Console")
		int32 MaxLineLength;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRLogComponent|Console")
		int32 MaxStoredMessages;

	UFUNCTION(BlueprintCallable, Category = "VRLogComponent|Console", meta = (bIgnoreSelf = "true"))
		void SetConsoleText(FString Text);

	UFUNCTION(BlueprintCallable, Category = "VRLogComponent|Console", meta = (bIgnoreSelf = "true"))
		void SendKeyEventToConsole(FKey Key, EInputEvent KeyEvent);

	UFUNCTION(BlueprintCallable, Category = "VRLogComponent|Console", meta = (bIgnoreSelf = "true"))
		void AppendTextToConsole(FString Text, bool bReturnAtEnd = false);

	UFUNCTION(BlueprintCallable, Category = "VRLogComponent|Console", meta = (bIgnoreSelf = "true", DisplayName = "DrawConsoleToCanvasRenderTarget2D"))
		bool DrawConsoleToRenderTarget2D(EBPVRConsoleDrawType DrawType, UTextureRenderTarget2D * Texture, float ScrollOffset, bool bForceDraw);

	void DrawConsole(bool bLowerHalfOnly, UCanvas* Canvas);
	void DrawOutputLog(bool bUpperHalfOnly, UCanvas* Canvas, float ScrollOffset);

};