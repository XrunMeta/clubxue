

#pragma once

#include "VaRestJsonValue.generated.h"

class UVaRestJsonObject;
class FJsonValue;

UENUM(BlueprintType)
enum class EVaJson : uint8
{
	None,
	Null,
	String,
	Number,
	Boolean,
	Array,
	Object,
};

UCLASS(BlueprintType, Blueprintable)
class VAREST_API UVaRestJsonValue : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	void Reset();

	TSharedPtr<FJsonValue>& GetRootValue();

	void SetRootValue(TSharedPtr<FJsonValue>& JsonValue);

	UFUNCTION(BlueprintPure, Category = "VaRest|Json")
	EVaJson GetType() const;

	UFUNCTION(BlueprintPure, Category = "VaRest|Json")
	FString GetTypeString() const;

	UFUNCTION(BlueprintPure, Category = "VaRest|Json")
	bool IsNull() const;

	UFUNCTION(BlueprintPure, Category = "VaRest|Json")
	float AsNumber() const;

	UFUNCTION(BlueprintPure, Category = "VaRest|Json")
	int32 AsInt32() const;

	UFUNCTION(BlueprintPure, Category = "VaRest|Json")
	int64 AsInt64() const;

	UFUNCTION(BlueprintPure, Category = "VaRest|Json")
	FString AsString() const;

	UFUNCTION(BlueprintPure, Category = "VaRest|Json")
	bool AsBool() const;

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	TArray<UVaRestJsonValue*> AsArray() const;

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	UVaRestJsonObject* AsObject();

private:

	TSharedPtr<FJsonValue> JsonVal;

protected:

	void ErrorMessage(const FString& InType) const;
};
