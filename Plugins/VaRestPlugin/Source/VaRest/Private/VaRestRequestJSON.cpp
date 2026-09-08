

#include "VaRestRequestJSON.h"

#include "VaRestDefines.h"
#include "VaRestJsonObject.h"
#include "VaRestJsonValue.h"
#include "VaRestLibrary.h"
#include "VaRestSettings.h"

#include "Engine/Engine.h"
#include "Engine/LatentActionManager.h"
#include "Engine/World.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

FString UVaRestRequestJSON::DeprecatedResponseString(TEXT("DEPRECATED: Please use GetResponseContentAsString() instead"));

template <class T>
void FVaRestLatentAction<T>::Cancel()
{
	UObject* Obj = Request.Get();
	if (Obj != nullptr)
	{
		((UVaRestRequestJSON*)Obj)->Cancel();
	}
}

UVaRestRequestJSON::UVaRestRequestJSON(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, BinaryContentType(TEXT("application/octet-stream"))
{
	ContinueAction = nullptr;

	RequestVerb = EVaRestRequestVerb::GET;
	RequestContentType = EVaRestRequestContentType::x_www_form_urlencoded_url;

	ResetData();
}

void UVaRestRequestJSON::SetVerb(EVaRestRequestVerb Verb)
{
	RequestVerb = Verb;
}

void UVaRestRequestJSON::SetCustomVerb(FString Verb)
{
	CustomVerb = Verb;
}

void UVaRestRequestJSON::SetContentType(EVaRestRequestContentType ContentType)
{
	RequestContentType = ContentType;
}

void UVaRestRequestJSON::SetBinaryContentType(const FString& ContentType)
{
	BinaryContentType = ContentType;
}

void UVaRestRequestJSON::SetBinaryRequestContent(const TArray<uint8>& Bytes)
{
	RequestBytes = Bytes;
}

void UVaRestRequestJSON::SetStringRequestContent(const FString& Content)
{
	StringRequestContent = Content;
}

void UVaRestRequestJSON::SetHeader(const FString& HeaderName, const FString& HeaderValue)
{
	RequestHeaders.Add(HeaderName, HeaderValue);
}

void UVaRestRequestJSON::ResetData()
{
	ResetRequestData();
	ResetResponseData();
}

void UVaRestRequestJSON::ResetRequestData()
{
	if (RequestJsonObj != nullptr)
	{
		RequestJsonObj->Reset();
	}
	else
	{
		RequestJsonObj = NewObject<UVaRestJsonObject>();
	}

	RequestBytes.Empty();
	StringRequestContent.Empty();
}

void UVaRestRequestJSON::ResetResponseData()
{
	if (ResponseJsonObj != nullptr)
	{
		ResponseJsonObj->Reset();
	}
	else
	{
		ResponseJsonObj = NewObject<UVaRestJsonObject>();
	}

	if (ResponseJsonValue != nullptr)
	{
		ResponseJsonValue->Reset();
	}
	else
	{
		ResponseJsonValue = NewObject<UVaRestJsonValue>();
	}

	ResponseHeaders.Empty();
	ResponseCode = -1;
	ResponseSize = 0;

	bIsValidJsonResponse = false;

	ResponseContent = DeprecatedResponseString;

	ResponseBytes.Empty();
	ResponseContentLength = 0;
}

void UVaRestRequestJSON::Cancel()
{
	ContinueAction = nullptr;

	ResetResponseData();
}

UVaRestJsonObject* UVaRestRequestJSON::GetRequestObject() const
{
	check(RequestJsonObj);
	return RequestJsonObj;
}

void UVaRestRequestJSON::SetRequestObject(UVaRestJsonObject* JsonObject)
{
	if (JsonObject == nullptr)
	{
		UE_LOG(LogVaRest, Error, TEXT("%s: Provided JsonObject is nullptr"), *VA_FUNC_LINE);
		return;
	}

	RequestJsonObj = JsonObject;
}

UVaRestJsonObject* UVaRestRequestJSON::GetResponseObject() const
{
	check(ResponseJsonObj);
	return ResponseJsonObj;
}

void UVaRestRequestJSON::SetResponseObject(UVaRestJsonObject* JsonObject)
{
	if (JsonObject == nullptr)
	{
		UE_LOG(LogVaRest, Error, TEXT("%s: Provided JsonObject is nullptr"), *VA_FUNC_LINE);
		return;
	}

	ResponseJsonObj = JsonObject;
}

UVaRestJsonValue* UVaRestRequestJSON::GetResponseValue() const
{
	check(ResponseJsonValue);
	return ResponseJsonValue;
}

FString UVaRestRequestJSON::GetURL() const
{
	return HttpRequest->GetURL();
}

EVaRestRequestVerb UVaRestRequestJSON::GetVerb() const
{
	return RequestVerb;
}

EVaRestRequestStatus UVaRestRequestJSON::GetStatus() const
{
	return EVaRestRequestStatus((uint8)HttpRequest->GetStatus());
}

int32 UVaRestRequestJSON::GetResponseCode() const
{
	return ResponseCode;
}

FString UVaRestRequestJSON::GetResponseHeader(const FString& HeaderName)
{
	FString Result;

	FString* Header = ResponseHeaders.Find(HeaderName);
	if (Header != nullptr)
	{
		Result = *Header;
	}

	return Result;
}

TArray<FString> UVaRestRequestJSON::GetAllResponseHeaders() const
{
	TArray<FString> Result;
	for (TMap<FString, FString>::TConstIterator It(ResponseHeaders); It; ++It)
	{
		Result.Add(It.Key() + TEXT(": ") + It.Value());
	}
	return Result;
}

int32 UVaRestRequestJSON::GetResponseContentLength() const
{
	return ResponseContentLength;
}

const TArray<uint8>& UVaRestRequestJSON::GetResponseContent() const
{
	return ResponseBytes;
}

void UVaRestRequestJSON::SetURL(const FString& Url)
{

	FString TrimmedUrl = Url;

	TrimmedUrl.TrimStartInline();
	TrimmedUrl.TrimEndInline();

	HttpRequest->SetURL(TrimmedUrl);
}

void UVaRestRequestJSON::ProcessURL(const FString& Url)
{
	SetURL(Url);
	ProcessRequest();
}

void UVaRestRequestJSON::ApplyURL(const FString& Url, UVaRestJsonObject*& Result, UObject* WorldContextObject, FLatentActionInfo LatentInfo)
{

	FString TrimmedUrl = Url;

	TrimmedUrl.TrimStartInline();
	TrimmedUrl.TrimEndInline();

	HttpRequest->SetURL(TrimmedUrl);

	if (UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FVaRestLatentAction<UVaRestJsonObject*>* Kont = LatentActionManager.FindExistingAction<FVaRestLatentAction<UVaRestJsonObject*>>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		if (Kont != nullptr)
		{
			Kont->Cancel();
			LatentActionManager.RemoveActionsForObject(LatentInfo.CallbackTarget);
		}

		LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, ContinueAction = new FVaRestLatentAction<UVaRestJsonObject*>(this, Result, LatentInfo));
	}

	ProcessRequest();
}

void UVaRestRequestJSON::ExecuteProcessRequest()
{
	if (HttpRequest->GetURL().Len() == 0)
	{
		UE_LOG(LogVaRest, Error, TEXT("Request execution attempt with empty URL"));
		return;
	}

	ProcessRequest();
}

void UVaRestRequestJSON::ProcessRequest()
{

	AddToRoot();

	switch (RequestVerb)
	{
	case EVaRestRequestVerb::GET:
		HttpRequest->SetVerb(TEXT("GET"));
		break;

	case EVaRestRequestVerb::POST:
		HttpRequest->SetVerb(TEXT("POST"));
		break;

	case EVaRestRequestVerb::PUT:
		HttpRequest->SetVerb(TEXT("PUT"));
		break;

	case EVaRestRequestVerb::DEL:
		HttpRequest->SetVerb(TEXT("DELETE"));
		break;

	case EVaRestRequestVerb::CUSTOM:
		HttpRequest->SetVerb(CustomVerb);
		break;

	default:
		break;
	}

	switch (RequestContentType)
	{
	case EVaRestRequestContentType::x_www_form_urlencoded_url:
	{
		HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));

		FString UrlParams = "";
		uint16 ParamIdx = 0;

		for (auto RequestIt = RequestJsonObj->GetRootObject()->Values.CreateIterator(); RequestIt; ++RequestIt)
		{
			FString Key = FString(RequestIt.Key());
			FString Value = RequestIt.Value().Get()->AsString();

			if (!Key.IsEmpty() && !Value.IsEmpty())
			{
				UrlParams += ParamIdx == 0 ? "?" : "&";
				UrlParams += UVaRestLibrary::PercentEncode(Key) + "=" + UVaRestLibrary::PercentEncode(Value);
			}

			ParamIdx++;
		}

		HttpRequest->SetURL(HttpRequest->GetURL() + UrlParams);

		if (!StringRequestContent.IsEmpty())
		{
			HttpRequest->SetContentAsString(StringRequestContent);
		}

		if (UVaRestLibrary::GetVaRestSettings()->bExtendedLog)
		{
			UE_LOG(LogVaRest, Log, TEXT("%s: Request (urlencoded): %s %s %s %s"), *VA_FUNC_LINE, *HttpRequest->GetVerb(), *HttpRequest->GetURL(), *UrlParams, *StringRequestContent);
		}
		else
		{
			UE_LOG(LogVaRest, Log, TEXT("%s: Request (urlencoded): %s %s (check bExtendedLog for additional data)"), *VA_FUNC_LINE, *HttpRequest->GetVerb(), *HttpRequest->GetURL());
		}

		break;
	}
	case EVaRestRequestContentType::x_www_form_urlencoded_body:
	{
		HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));

		FString UrlParams = "";
		uint16 ParamIdx = 0;

		if (!StringRequestContent.IsEmpty())
		{
			UrlParams = StringRequestContent;
		}
		else
		{

			for (auto RequestIt = RequestJsonObj->GetRootObject()->Values.CreateIterator(); RequestIt; ++RequestIt)
			{
				FString Key = FString(RequestIt.Key());
				FString Value = RequestIt.Value().Get()->AsString();

				if (!Key.IsEmpty() && !Value.IsEmpty())
				{
					UrlParams += ParamIdx == 0 ? "" : "&";
					UrlParams += UVaRestLibrary::PercentEncode(Key) + "=" + UVaRestLibrary::PercentEncode(Value);
				}

				ParamIdx++;
			}
		}

		HttpRequest->SetContentAsString(UrlParams);

		if (UVaRestLibrary::GetVaRestSettings()->bExtendedLog)
		{
			UE_LOG(LogVaRest, Log, TEXT("%s: Request (url body): %s %s %s"), *VA_FUNC_LINE, *HttpRequest->GetVerb(), *HttpRequest->GetURL(), *UrlParams);
		}
		else
		{
			UE_LOG(LogVaRest, Log, TEXT("%s: Request (url body): %s %s (check bExtendedLog for additional data)"), *VA_FUNC_LINE, *HttpRequest->GetVerb(), *HttpRequest->GetURL());
		}

		break;
	}
	case EVaRestRequestContentType::binary:
	{
		HttpRequest->SetHeader(TEXT("Content-Type"), BinaryContentType);
		HttpRequest->SetContent(RequestBytes);

		UE_LOG(LogVaRest, Log, TEXT("Request (binary): %s %s"), *HttpRequest->GetVerb(), *HttpRequest->GetURL());

		break;
	}
	case EVaRestRequestContentType::json:
	{
		HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

		if (RequestVerb == EVaRestRequestVerb::GET)
		{
			break;
		}

		FString OutputString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
		FJsonSerializer::Serialize(RequestJsonObj->GetRootObject(), Writer);

		HttpRequest->SetContentAsString(OutputString);

		if (UVaRestLibrary::GetVaRestSettings()->bExtendedLog)
		{
			UE_LOG(LogVaRest, Log, TEXT("Request (json): %s %s %sJSON(%s%s%s)JSON"), *HttpRequest->GetVerb(), *HttpRequest->GetURL(), LINE_TERMINATOR, LINE_TERMINATOR, *OutputString, LINE_TERMINATOR);
		}
		else
		{
			UE_LOG(LogVaRest, Log, TEXT("Request (json): %s %s (check bExtendedLog for additional data)"), *HttpRequest->GetVerb(), *HttpRequest->GetURL());
		}

		break;
	}

	default:
		break;
	}

	for (TMap<FString, FString>::TConstIterator It(RequestHeaders); It; ++It)
	{
		HttpRequest->SetHeader(It.Key(), It.Value());
	}

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UVaRestRequestJSON::OnProcessRequestComplete);

	HttpRequest->ProcessRequest();
}

void UVaRestRequestJSON::OnProcessRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{

	RemoveFromRoot();

	ResetResponseData();

	if (Response.IsValid())
	{
		ResponseCode = Response->GetResponseCode();
	}

	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogVaRest, Error, TEXT("Request failed (%d): %s"), ResponseCode, *Request->GetURL());

		OnRequestFail.Broadcast(this);
		OnStaticRequestFail.Broadcast(this);

		return;
	}

#if PLATFORM_DESKTOP

	UE_LOG(LogVaRest, Log, TEXT("Response (%d): %sJSON(%s%s%s)JSON"), ResponseCode, LINE_TERMINATOR, LINE_TERMINATOR, *Response->GetContentAsString(), LINE_TERMINATOR);
#endif

	TArray<FString> Headers = Response->GetAllHeaders();
	for (FString Header : Headers)
	{
		FString Key;
		FString Value;
		if (Header.Split(TEXT(": "), &Key, &Value))
		{
			ResponseHeaders.Add(Key, Value);
		}
	}

	if (UVaRestLibrary::GetVaRestSettings()->bUseChunkedParser)
	{

		const TArray<uint8>& Bytes = Response->GetContent();
		ResponseSize = ResponseJsonObj->DeserializeFromUTF8Bytes((const ANSICHAR*)Bytes.GetData(), Bytes.Num());

		if (ResponseSize == 0)
		{

			UE_LOG(LogVaRest, Warning, TEXT("JSON could not be decoded!"));
		}
	}
	else
	{

		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(*Response->GetContentAsString());
		TSharedPtr<FJsonValue> OutJsonValue;
		if (FJsonSerializer::Deserialize(Reader, OutJsonValue))
		{
			ResponseJsonValue->SetRootValue(OutJsonValue);

			if (ResponseJsonValue->GetType() == EVaJson::Object)
			{
				ResponseJsonObj->SetRootObject(ResponseJsonValue->GetRootValue()->AsObject());
				ResponseSize = Response->GetContentLength();
			}
		}
	}

	bIsValidJsonResponse = bWasSuccessful && (ResponseSize > 0);

	if (!bIsValidJsonResponse)
	{

		ResponseContent = Response->GetContentAsString();
		ResponseSize = ResponseContent.GetAllocatedSize();

		ResponseBytes = Response->GetContent();
		ResponseContentLength = Response->GetContentLength();
	}

	OnRequestComplete.Broadcast(this);
	OnStaticRequestComplete.Broadcast(this);

	if (ContinueAction)
	{
		FVaRestLatentAction<UVaRestJsonObject*>* K = ContinueAction;
		ContinueAction = nullptr;

		K->Call(ResponseJsonObj);
	}
}

void UVaRestRequestJSON::AddTag(FName Tag)
{
	if (Tag != NAME_None)
	{
		Tags.AddUnique(Tag);
	}
}

int32 UVaRestRequestJSON::RemoveTag(FName Tag)
{
	return Tags.Remove(Tag);
}

bool UVaRestRequestJSON::HasTag(FName Tag) const
{
	return (Tag != NAME_None) && Tags.Contains(Tag);
}

FString UVaRestRequestJSON::GetResponseContentAsString(bool bCacheResponseContent)
{

	if (!bIsValidJsonResponse)
	{

		return ResponseContent;
	}

	if (!ResponseJsonObj || !ResponseJsonObj->IsValidLowLevel())
	{

		ResponseContent = DeprecatedResponseString;

		return TEXT("Invalid response");
	}

	if (!bCacheResponseContent)
	{
		UE_LOG(LogVaRest, Warning, TEXT("%s: Use of uncashed getter could be slow"), *VA_FUNC_LINE);
		return ResponseJsonObj->EncodeJson();
	}

	if (ResponseContent == DeprecatedResponseString)
	{
		UE_LOG(LogVaRest, Warning, TEXT("%s: Response content string is cached"), *VA_FUNC_LINE);
		ResponseContent = ResponseJsonObj->EncodeJson();
	}

	return ResponseContent;
}

