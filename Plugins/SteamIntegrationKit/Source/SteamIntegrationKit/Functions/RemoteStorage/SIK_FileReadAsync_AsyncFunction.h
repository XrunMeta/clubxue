

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "SIK_SharedFile.h"
#include "SIK_FileReadAsync_AsyncFunction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnRemoteStorageFileReadAsyncComplete, const TEnumAsByte<ESIK_Result>&, Result, int32, nOffset, int32, nBytesRead, const TArray<uint8>&, Data);

UCLASS()
class STEAMINTEGRATIONKIT_API USIK_FileReadAsync_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, DisplayName="File Read Async",meta = (BlueprintInternalUseOnly = "true"), Category="Steam Integration Kit || SDK Functions || Remote Storage|| Async")
	static USIK_FileReadAsync_AsyncFunction* FileReadAsync(const FString& FileName, int32 nOffset, int32 nBytesToRead);

	UPROPERTY(BlueprintAssignable)
	FOnRemoteStorageFileReadAsyncComplete OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnRemoteStorageFileReadAsyncComplete OnFailure;
private:
	FString Var_FileName;
	int32 Var_nOffset;
	int32 Var_nBytesToRead;
	virtual void Activate() override;
#if (WITH_ENGINE_STEAM && ONLINESUBSYSTEMSTEAM_PACKAGE) || (WITH_STEAMKIT && !WITH_ENGINE_STEAM)	
	void OnFileReadAsync(RemoteStorageFileReadAsyncComplete_t* RemoteStorageFileReadAsyncComplete, bool bIOFailure);
	SteamAPICall_t CallbackHandle;
	CCallResult<USIK_FileReadAsync_AsyncFunction, RemoteStorageFileReadAsyncComplete_t> OnFileReadAsyncCallResult;
#endif

};
