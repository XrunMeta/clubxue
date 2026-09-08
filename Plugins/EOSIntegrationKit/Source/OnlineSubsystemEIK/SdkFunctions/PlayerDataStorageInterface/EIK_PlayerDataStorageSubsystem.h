

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EIK_PlayerDataStorageSubsystem.generated.h"

UCLASS(DisplayName="Player Data Storage Interface", meta=(DisplayName="Player Data Storage Interface"))
class ONLINESUBSYSTEMEIK_API UEIK_PlayerDataStorageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Player Data Storage Interface", DisplayName="EOS_PlayerDataStorage_CopyFileMetadataAtIndex")
	TEnumAsByte<EEIK_Result>  EIK_PlayerDataStorage_CopyFileMetadataAtIndex(FEIK_ProductUserId LocalUserId, int32 Index, FEIK_PlayerDataStorage_FileMetadata& OutMetadata);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Player Data Storage Interface", DisplayName="EOS_PlayerDataStorage_CopyFileMetadataByFilename")
	TEnumAsByte<EEIK_Result>  EIK_PlayerDataStorage_CopyFileMetadataByFilename(FEIK_ProductUserId LocalUserId, FString Filename, FEIK_PlayerDataStorage_FileMetadata& OutMetadata);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Player Data Storage Interface", DisplayName="EOS_PlayerDataStorage_FileMetadata_Release")
	void EIK_PlayerDataStorage_FileMetadata_Release(FEIK_PlayerDataStorage_FileMetadata& Metadata);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Player Data Storage Interface", DisplayName="EOS_PlayerDataStorage_GetFileMetadataCount")
	TEnumAsByte<EEIK_Result>  EIK_PlayerDataStorage_GetFileMetadataCount(FEIK_ProductUserId LocalUserId, int32& OutFileMetadataCount);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Player Data Storage Interface", DisplayName="EOS_PlayerDataStorageFileTransferRequest_GetFilename")
	TEnumAsByte<EEIK_Result>  EIK_PlayerDataStorageFileTransferRequest_GetFilename(FEIK_HPlayerDataStorageFileTransferRequest TransferRequestHandle, FString& OutFilename);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Player Data Storage Interface", DisplayName="EOS_PlayerDataStorageFileTransferRequest_GetFileRequestState")
	TEnumAsByte<EEIK_Result>  EIK_PlayerDataStorageFileTransferRequest_GetFileRequestState(FEIK_HPlayerDataStorageFileTransferRequest TransferRequestHandle);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Player Data Storage Interface", DisplayName="EOS_PlayerDataStorageFileTransferRequest_Release")
	void EIK_PlayerDataStorageFileTransferRequest_Release(FEIK_HPlayerDataStorageFileTransferRequest TransferRequestHandle);
};
