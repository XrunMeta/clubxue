

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
#include "EIK_Lobby_CreateLobby.generated.h"

USTRUCT(BlueprintType)
struct FEIK_Lobby_CreateLobbyOptions
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	FEIK_ProductUserId LocalUserId;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	int32 MaxLobbyMembers;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	TEnumAsByte<EEIK_ELobbyPermissionLevel> PermissionLevel;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	bool bPresenceEnabled;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	bool bAllowInvites;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	FString BucketId;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	bool bDisableHostMigration;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	bool bEnableRTCRoom;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	FEIK_Lobby_LocalRTCOptions LocalRTCOptions;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	FEIK_LobbyId LobbyId;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	bool bEnableJoinById;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	bool bRejoinAfterKickRequiresInvite;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	TArray<int32> AllowedPlatformIds;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	bool bCrossplayOptOut;

	FEIK_Lobby_CreateLobbyOptions()
	{
		MaxLobbyMembers = 0;
		PermissionLevel = EEIK_ELobbyPermissionLevel::EIK_LPL_InviteOnly;
		bPresenceEnabled = false;
		bAllowInvites = false;
		bDisableHostMigration = false;
		bEnableRTCRoom = false;
		bEnableJoinById = false;
		bRejoinAfterKickRequiresInvite = false;
		bCrossplayOptOut = false;
	}
	EOS_Lobby_CreateLobbyOptions GetCreateLobbyOptions()
	{
		EOS_Lobby_CreateLobbyOptions Options;
		Options.ApiVersion = EOS_LOBBY_CREATELOBBY_API_LATEST;
		Options.LocalUserId = LocalUserId.GetValueAsEosType();
		Options.MaxLobbyMembers = MaxLobbyMembers;
		Options.PermissionLevel = static_cast<EOS_ELobbyPermissionLevel>(PermissionLevel.GetValue());
		Options.bPresenceEnabled = bPresenceEnabled;
		Options.bAllowInvites = bAllowInvites;
		auto BucketIdAnsi = StringCast<ANSICHAR>(*BucketId);
		CachedBucketIdAnsi.SetNumUninitialized(BucketIdAnsi.Length() + 1);
		FMemory::Memcpy(CachedBucketIdAnsi.GetData(), BucketIdAnsi.Get(), BucketIdAnsi.Length() + 1);
		Options.BucketId = CachedBucketIdAnsi.GetData();
		Options.bDisableHostMigration = bDisableHostMigration;
		Options.bEnableRTCRoom = bEnableRTCRoom;
		EOS_Lobby_LocalRTCOptions TempValue = LocalRTCOptions.GetValueAsEosType();
		Options.LocalRTCOptions = &TempValue;
		Options.LobbyId = LobbyId.GetValueAsEosType();
		Options.bEnableJoinById = bEnableJoinById;
		Options.bRejoinAfterKickRequiresInvite = bRejoinAfterKickRequiresInvite;
		Options.AllowedPlatformIdsCount = AllowedPlatformIds.Num();
		uint32_t* TempVar = new uint32_t[AllowedPlatformIds.Num()];
		for (int32 i = 0; i < AllowedPlatformIds.Num(); i++)
		{
			TempVar[i] = AllowedPlatformIds[i];
		}
		Options.AllowedPlatformIds = TempVar;
		Options.bCrossplayOptOut = bCrossplayOptOut;
		return Options;
	}

	TArray<ANSICHAR> CachedBucketIdAnsi;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEIK_Lobby_CreateLobbyComplete, const TEnumAsByte<EEIK_Result>&, Result, const FEIK_LobbyId&, LobbyId);

UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_Lobby_CreateLobby : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface", DisplayName="EOS_Lobby_CreateLobby")
	static UEIK_Lobby_CreateLobby* EIK_Lobby_CreateLobby(FEIK_Lobby_CreateLobbyOptions CreateLobbyOptions);

	UPROPERTY(BlueprintAssignable, Category = "EOS Integration Kit | SDK Functions | Lobby Interface")
	FEIK_Lobby_CreateLobbyComplete OnCallback;

private:
	FEIK_Lobby_CreateLobbyOptions Var_CreateLobbyOptions;
	static void EOS_CALL OnCreateLobbyComplete(const EOS_Lobby_CreateLobbyCallbackInfo* Data);
	virtual void Activate() override;
};
