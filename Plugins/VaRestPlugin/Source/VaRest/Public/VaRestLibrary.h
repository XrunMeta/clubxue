

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "VaRestTypes.h"

#include "VaRestLibrary.generated.h"

class UVaRestSettings;

UCLASS()
class VAREST_API UVaRestLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "VaRest|Common")
	static UVaRestSettings* GetVaRestSettings();

public:

	UFUNCTION(BlueprintPure, Category = "VaRest|Utility")
	static FString PercentEncode(const FString& Source);

	UFUNCTION(BlueprintPure, Category = "VaRest|Utility", meta = (DisplayName = "Base64 Encode"))
	static FString Base64Encode(const FString& Source);

	UFUNCTION(BlueprintPure, Category = "VaRest|Utility", meta = (DisplayName = "Base64 Decode"))
	static bool Base64Decode(const FString& Source, FString& Dest);

	UFUNCTION(BlueprintPure, Category = "VaRest|Utility", meta = (DisplayName = "Base64 Encode Data"))
	static bool Base64EncodeData(const TArray<uint8>& Data, FString& Dest);

	UFUNCTION(BlueprintPure, Category = "VaRest|Utility", meta = (DisplayName = "Base64 Decode Data"))
	static bool Base64DecodeData(const FString& Source, TArray<uint8>& Dest);

	UFUNCTION(BlueprintPure, Category = "VaRest|Utility", meta = (DisplayName = "String to MD5"))
	static FString StringToMd5(const FString& StringToHash);

	UFUNCTION(BlueprintPure, Category = "VaRest|Utility", meta = (DisplayName = "String to SHA1"))
	static FString StringToSha1(const FString& StringToHash);

	UFUNCTION(BlueprintPure, Category = "VaRest|Utility", meta = (DisplayName = "HTTP Status Int To Enum"))
	static FORCEINLINE EVaRestHttpStatusCode::Type HTTPStatusIntToEnum(int32 StatusCode) { return (EVaRestHttpStatusCode::Type)StatusCode; }

	UFUNCTION(BlueprintPure, Category = "VaRest|Utility", meta = (DisplayName = "Get VaRest Version"))
	static FString GetVaRestVersion();

public:

	UFUNCTION(BlueprintPure, Category = "VaRest|Utility", meta = (WorldContext = "WorldContextObject"))
	static FVaRestURL GetWorldURL(UObject* WorldContextObject);
};
