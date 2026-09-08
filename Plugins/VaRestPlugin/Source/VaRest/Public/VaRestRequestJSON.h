

#pragma once

#include "Engine/LatentActionManager.h"
#include "Http.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "LatentActions.h"

#include "VaRestTypes.h"

#include "VaRestRequestJSON.generated.h"

class UVaRestJsonValue;
class UVaRestJsonObject;
class UVaRestSettings;

template <class T>
class FVaRestLatentAction : public FPendingLatentAction
{
public:
	virtual void Call(const T& Value)
	{
		Result = Value;
		Called = true;
	}

	void operator()(const T& Value)
	{
		Call(Value);
	}

	void Cancel();

	FVaRestLatentAction(FWeakObjectPtr RequestObj, T& ResultParam, const FLatentActionInfo& LatentInfo)
		: Called(false)
		, Request(RequestObj)
		, ExecutionFunction(LatentInfo.ExecutionFunction)
		, OutputLink(LatentInfo.Linkage)
		, CallbackTarget(LatentInfo.CallbackTarget)
		, Result(ResultParam)
	{
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		Response.FinishAndTriggerIf(Called, ExecutionFunction, OutputLink, CallbackTarget);
	}

	virtual void NotifyObjectDestroyed()
	{
		Cancel();
	}

	virtual void NotifyActionAborted()
	{
		Cancel();
	}

private:
	bool Called;
	FWeakObjectPtr Request;

public:
	const FName ExecutionFunction;
	const int32 OutputLink;
	const FWeakObjectPtr CallbackTarget;
	T& Result;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRequestComplete, class UVaRestRequestJSON*, Request);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRequestFail, class UVaRestRequestJSON*, Request);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnStaticRequestComplete, class UVaRestRequestJSON*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnStaticRequestFail, class UVaRestRequestJSON*);

UCLASS(BlueprintType, Blueprintable)
class VAREST_API UVaRestRequestJSON : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "VaRest|Request")
	void SetVerb(EVaRestRequestVerb Verb);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Request")
	void SetCustomVerb(FString Verb);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Request")
	void SetContentType(EVaRestRequestContentType ContentType);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Request")
	void SetBinaryContentType(const FString& ContentType);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Request")
	void SetBinaryRequestContent(const TArray<uint8>& Content);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Request")
	void SetStringRequestContent(const FString& Content);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Request")
	void SetHeader(const FString& HeaderName, const FString& HeaderValue);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Utility")
	void ResetData();

	UFUNCTION(BlueprintCallable, Category = "VaRest|Request")
	void ResetRequestData();

	UFUNCTION(BlueprintCallable, Category = "VaRest|Response")
	void ResetResponseData();

	UFUNCTION(BlueprintCallable, Category = "VaRest|Response")
	void Cancel();

	UFUNCTION(BlueprintCallable, Category = "VaRest|Request")
	UVaRestJsonObject* GetRequestObject() const;

	UFUNCTION(BlueprintCallable, Category = "VaRest|Request")
	void SetRequestObject(UVaRestJsonObject* JsonObject);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Response")
	UVaRestJsonObject* GetResponseObject() const;

	UFUNCTION(BlueprintCallable, Category = "VaRest|Response")
	void SetResponseObject(UVaRestJsonObject* JsonObject);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Response")
	UVaRestJsonValue* GetResponseValue() const;

	UFUNCTION(BlueprintPure, Category = "VaRest|Request")
	FString GetURL() const;

	UFUNCTION(BlueprintPure, Category = "VaRest|Request")
	EVaRestRequestVerb GetVerb() const;

	UFUNCTION(BlueprintPure, Category = "VaRest|Request")
	EVaRestRequestStatus GetStatus() const;

	UFUNCTION(BlueprintPure, Category = "VaRest|Response")
	int32 GetResponseCode() const;

	UFUNCTION(BlueprintPure, Category = "VaRest|Response")
	FString GetResponseHeader(const FString& HeaderName);

	UFUNCTION(BlueprintPure, Category = "VaRest|Response")
	TArray<FString> GetAllResponseHeaders() const;

	UFUNCTION(BlueprintPure, Category = "VaRest|Response")
	int32 GetResponseContentLength() const;

	UFUNCTION(BlueprintPure, Category = "VaRest|Response")
	const TArray<uint8>& GetResponseContent() const;

public:

	UFUNCTION(BlueprintCallable, Category = "VaRest|Request")
	void SetURL(const FString& Url = TEXT("http://alyamkin.com"));

	UFUNCTION(BlueprintCallable, Category = "VaRest|Request")
	virtual void ProcessURL(const FString& Url = TEXT("http://alyamkin.com"));

	UFUNCTION(BlueprintCallable, Category = "VaRest|Request", meta = (Latent, LatentInfo = "LatentInfo", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	virtual void ApplyURL(const FString& Url, UVaRestJsonObject*& Result, UObject* WorldContextObject, struct FLatentActionInfo LatentInfo);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Request")
	virtual void ExecuteProcessRequest();

protected:

	void ProcessRequest();

private:

	void OnProcessRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

public:

	UPROPERTY(BlueprintAssignable, Category = "VaRest|Event")
	FOnRequestComplete OnRequestComplete;

	UPROPERTY(BlueprintAssignable, Category = "VaRest|Event")
	FOnRequestFail OnRequestFail;

	FOnStaticRequestComplete OnStaticRequestComplete;

	FOnStaticRequestFail OnStaticRequestFail;

public:

	UFUNCTION(BlueprintCallable, Category = "VaRest|Utility")
	void AddTag(FName Tag);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Utility")
	int32 RemoveTag(FName Tag);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Utility")
	bool HasTag(FName Tag) const;

protected:

	TArray<FName> Tags;

public:

	UFUNCTION(BlueprintCallable, Category = "VaRest|Response")
	FString GetResponseContentAsString(bool bCacheResponseContent = true);

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VaRest|Response")
	int32 ResponseSize;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VaRest|Response")
	FString ResponseContent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VaRest|Response")
	bool bIsValidJsonResponse;

protected:

	static FString DeprecatedResponseString;

protected:

	FVaRestLatentAction<UVaRestJsonObject*>* ContinueAction;

	UPROPERTY()
	UVaRestJsonObject* RequestJsonObj;

	TArray<uint8> RequestBytes;
	FString BinaryContentType;

	TArray<uint8> ResponseBytes;
	int32 ResponseContentLength;

	FString StringRequestContent;

	UPROPERTY()
	UVaRestJsonObject* ResponseJsonObj;

	UPROPERTY()
	UVaRestJsonValue* ResponseJsonValue;

	EVaRestRequestVerb RequestVerb;

	EVaRestRequestContentType RequestContentType;

	TMap<FString, FString> RequestHeaders;

	TMap<FString, FString> ResponseHeaders;

	int32 ResponseCode;

	FString CustomVerb;

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

public:

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> GetHttpRequest() const { return HttpRequest; };
};
