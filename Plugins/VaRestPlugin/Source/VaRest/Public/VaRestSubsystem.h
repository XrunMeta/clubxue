

#pragma once

#include "VaRestJsonObject.h"
#include "VaRestJsonValue.h"
#include "VaRestRequestJSON.h"

#include "Subsystems/EngineSubsystem.h"

#include "VaRestSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FVaRestCallDelegate, UVaRestRequestJSON*, Request);

USTRUCT()
struct FVaRestCallResponse
{
	GENERATED_BODY()

	UPROPERTY()
	UVaRestRequestJSON* Request;

	UPROPERTY()
	FVaRestCallDelegate Callback;

	FDelegateHandle CompleteDelegateHandle;
	FDelegateHandle FailDelegateHandle;

	FVaRestCallResponse()
		: Request(nullptr)
	{
	}
};

UCLASS()
class VAREST_API UVaRestSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	UVaRestSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:

	UFUNCTION(BlueprintCallable, Category = "VaRest|Utility")
	void CallURL(const FString& URL, EVaRestRequestVerb Verb, EVaRestRequestContentType ContentType, UVaRestJsonObject* VaRestJson, const FVaRestCallDelegate& Callback);

	void OnCallComplete(UVaRestRequestJSON* Request);

protected:
	UPROPERTY()
	TMap<UVaRestRequestJSON*, FVaRestCallResponse> RequestMap;

public:

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Construct Json Request (Empty)"), Category = "VaRest|Subsystem")
	UVaRestRequestJSON* ConstructVaRestRequest();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Construct Json Request"), Category = "VaRest|Subsystem")
	UVaRestRequestJSON* ConstructVaRestRequestExt(EVaRestRequestVerb Verb, EVaRestRequestContentType ContentType);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Construct Json Object"), Category = "VaRest|Subsystem")
	UVaRestJsonObject* ConstructVaRestJsonObject();

	UFUNCTION()
	static UVaRestJsonObject* StaticConstructVaRestJsonObject();

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Construct Json Number Value"), Category = "VaRest|Subsystem")
	UVaRestJsonValue* ConstructJsonValueNumber(float Number);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Construct Json String Value"), Category = "VaRest|Subsystem")
	UVaRestJsonValue* ConstructJsonValueString(const FString& StringValue);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Construct Json Bool Value"), Category = "VaRest|Subsystem")
	UVaRestJsonValue* ConstructJsonValueBool(bool InValue);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Construct Json Array Value"), Category = "VaRest|Subsystem")
	UVaRestJsonValue* ConstructJsonValueArray(const TArray<UVaRestJsonValue*>& InArray);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Construct Json Object Value"), Category = "VaRest|Subsystem")
	UVaRestJsonValue* ConstructJsonValueObject(UVaRestJsonObject* JsonObject);

	UVaRestJsonValue* ConstructJsonValue(const TSharedPtr<FJsonValue>& InValue);

public:

	UFUNCTION(BlueprintCallable, Category = "VaRest|Subsystem")
	UVaRestJsonValue* DecodeJsonValue(const FString& JsonString);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Subsystem")
	UVaRestJsonObject* DecodeJsonObject(const FString& JsonString);

public:

	UFUNCTION(BlueprintCallable, Category = "VaRest|Utility")
	UVaRestJsonObject* LoadJsonFromFile(const FString& Path, const bool bIsRelativeToContentDir = true);
};
