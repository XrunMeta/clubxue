

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION == 5
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EIK_SetPlayerData_AsyncFunction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSetDataResult);

UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_SetPlayerData_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FSetDataResult OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FSetDataResult OnFail;

	bool bDelegateCalled = false;

	FString FileName;
	TArray<uint8> DataToSave;

	UFUNCTION(BlueprintCallable, DisplayName="Set EIK Player Storage", meta = (BlueprintInternalUseOnly = "true"), Category="EOS Integration Kit || Storage")
	static UEIK_SetPlayerData_AsyncFunction* SetPlayerData(FString FileName, const TArray<uint8>& DataToSave);

	virtual void Activate() override;

	void SetPlayerData();

	void OnWriteFileComplete(bool bSuccess, const FUniqueNetId& UserID, const FString& Var_FileName);

};
