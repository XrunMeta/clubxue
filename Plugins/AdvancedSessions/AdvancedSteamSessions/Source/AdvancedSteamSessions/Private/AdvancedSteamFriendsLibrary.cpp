
#include "AdvancedSteamFriendsLibrary.h"
#include "OnlineSubSystemHeader.h"
#include "OnlineSubsystemTypes.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#include "PixelFormat.h"

DEFINE_LOG_CATEGORY(AdvancedSteamFriendsLog);

void UAdvancedSteamFriendsLibrary::GetSteamGroups(TArray<FBPSteamGroupInfo> & SteamGroups)
{

#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED

	if (SteamAPI_Init())
	{
		int numClans = SteamFriends()->GetClanCount();

		for (int i = 0; i < numClans; i++)
		{
			CSteamID SteamGroupID = SteamFriends()->GetClanByIndex(i);

			if(!SteamGroupID.IsValid())
				continue;

			FBPSteamGroupInfo GroupInfo;

			TSharedPtr<const FUniqueNetId> ValueID(new const FUniqueNetIdSteam2(SteamGroupID));
			GroupInfo.GroupID.SetUniqueNetId(ValueID);
			SteamFriends()->GetClanActivityCounts(SteamGroupID, &GroupInfo.numOnline, &GroupInfo.numInGame, &GroupInfo.numChatting);
			GroupInfo.GroupName = FString(UTF8_TO_TCHAR(SteamFriends()->GetClanName(SteamGroupID)));
			GroupInfo.GroupTag = FString(UTF8_TO_TCHAR(SteamFriends()->GetClanTag(SteamGroupID)));

			SteamGroups.Add(GroupInfo);
		}
	}
#endif

}

void UAdvancedSteamFriendsLibrary::GetSteamFriendGamePlayed(const FBPUniqueNetId UniqueNetId, EBlueprintResultSwitch &Result, int32 & AppID)
{

#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED
	if (!UniqueNetId.IsValid() || !UniqueNetId.UniqueNetId->IsValid() || UniqueNetId.UniqueNetId->GetType() != STEAM_SUBSYSTEM)
	{
		UE_LOGF(AdvancedSteamFriendsLog, Warning, "GetSteamFriendGamePlayed Had a bad UniqueNetId!");
		Result = EBlueprintResultSwitch::OnFailure;
		return;
	}

	if (SteamAPI_Init())
	{
		uint64 id = *((uint64*)UniqueNetId.UniqueNetId->GetBytes());

		FriendGameInfo_t GameInfo;
		bool bIsInGame = SteamFriends()->GetFriendGamePlayed(id, &GameInfo);

		if (bIsInGame && GameInfo.m_gameID.IsValid())
		{
			AppID = GameInfo.m_gameID.AppID();

			Result = EBlueprintResultSwitch::OnSuccess;
			return;
		}

	}
#endif

	Result = EBlueprintResultSwitch::OnFailure;
}

int32 UAdvancedSteamFriendsLibrary::GetFriendSteamLevel(const FBPUniqueNetId UniqueNetId)
{

#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED
	if (!UniqueNetId.IsValid() || !UniqueNetId.UniqueNetId->IsValid() || UniqueNetId.UniqueNetId->GetType() != STEAM_SUBSYSTEM)
	{
		UE_LOGF(AdvancedSteamFriendsLog, Warning, "IsAFriend Had a bad UniqueNetId!");
		return 0;
	}

	if (SteamAPI_Init())
	{
		uint64 id = *((uint64*)UniqueNetId.UniqueNetId->GetBytes());

		return SteamFriends()->GetFriendSteamLevel(id);
	}
#endif

	return 0;
}

FString UAdvancedSteamFriendsLibrary::GetSteamPersonaName(const FBPUniqueNetId UniqueNetId)
{

#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED
	if (!UniqueNetId.IsValid() || !UniqueNetId.UniqueNetId->IsValid() || UniqueNetId.UniqueNetId->GetType() != STEAM_SUBSYSTEM)
	{
		UE_LOGF(AdvancedSteamFriendsLog, Warning, "GetSteamPersonaName Had a bad UniqueNetId!");
		return FString(TEXT(""));
	}

	if (SteamAPI_Init())
	{
		uint64 id = *((uint64*)UniqueNetId.UniqueNetId->GetBytes());
		const char* PersonaName = SteamFriends()->GetFriendPersonaName(id);
		return FString(UTF8_TO_TCHAR(PersonaName));
	}
#endif

	return FString(TEXT(""));
}

FBPUniqueNetId UAdvancedSteamFriendsLibrary::CreateSteamIDFromString(const FString SteamID64)
{
	FBPUniqueNetId netId;

#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED
	if (!(SteamID64.Len() > 0))
	{
		UE_LOGF(AdvancedSteamFriendsLog, Warning, "CreateSteamIDFromString Had a bad UniqueNetId!");
		return netId;
	}

	if (SteamAPI_Init())
	{

		TSharedPtr<const FUniqueNetId> ValueID(new const FUniqueNetIdSteam2(SteamID64));

		netId.SetUniqueNetId(ValueID);
		return netId;
	}
#endif

	return netId;
}

FBPUniqueNetId UAdvancedSteamFriendsLibrary::GetLocalSteamIDFromSteam()
{
	FBPUniqueNetId netId;

#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED
	if (SteamAPI_Init())
	{
		TSharedPtr<const FUniqueNetId> SteamID(new const FUniqueNetIdSteam2(SteamUser()->GetSteamID()));
		netId.SetUniqueNetId(SteamID);
	}
#endif

	return netId;
}

bool UAdvancedSteamFriendsLibrary::RequestSteamFriendInfo(const FBPUniqueNetId UniqueNetId, bool bRequireNameOnly)
{
#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED
	if (!UniqueNetId.IsValid() || !UniqueNetId.UniqueNetId->IsValid() || UniqueNetId.UniqueNetId->GetType() != STEAM_SUBSYSTEM)
	{
		UE_LOGF(AdvancedSteamFriendsLog, Warning, "RequestSteamFriendInfo Had a bad UniqueNetId!");
		return false;
	}

	if (SteamAPI_Init())
	{
		uint64 id = *((uint64*)UniqueNetId.UniqueNetId->GetBytes());

		return !SteamFriends()->RequestUserInformation(id, bRequireNameOnly);
	}
#endif

	UE_LOGF(AdvancedSteamFriendsLog, Warning, "RequestSteamFriendInfo Couldn't init steamAPI!");
	return false;
}

bool UAdvancedSteamFriendsLibrary::OpenSteamUserOverlay(UObject* WorldContextObject,const FBPUniqueNetId UniqueNetId, ESteamUserOverlayType DialogType)
{
#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED
	if (!UniqueNetId.IsValid() || !UniqueNetId.UniqueNetId->IsValid() || UniqueNetId.UniqueNetId->GetType() != STEAM_SUBSYSTEM)
	{
		UE_LOGF(AdvancedSteamFriendsLog, Warning, "OpenSteamUserOverlay Had a bad UniqueNetId!");
		return false;
	}

	if (SteamAPI_Init())
	{
		if (DialogType == ESteamUserOverlayType::invitetolobby)
		{
			UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
			IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(World);
			if (SessionInterface.IsValid())
			{
				FNamedOnlineSession* CurrentSession = SessionInterface->GetNamedSession(NAME_GameSession);

				if (CurrentSession && CurrentSession->SessionInfo->GetSessionId().IsValid())
				{			
					uint64 id = *((uint64*)CurrentSession->SessionInfo->GetSessionId().GetBytes());
					SteamFriends()->ActivateGameOverlayInviteDialog(id);
				}
			}
		}
		else
		{
			uint64 id = *((uint64*)UniqueNetId.UniqueNetId->GetBytes());
			FString DialogName = EnumToString("ESteamUserOverlayType", (uint8)DialogType);
			SteamFriends()->ActivateGameOverlayToUser(TCHAR_TO_ANSI(*DialogName), id);
		}
		return true;
	}
#endif

	UE_LOGF(AdvancedSteamFriendsLog, Warning, "OpenSteamUserOverlay Couldn't init steamAPI!");
	return false;
}

bool UAdvancedSteamFriendsLibrary::IsOverlayEnabled()
{
#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED
	if (SteamAPI_Init())
	{
		return SteamUtils()->IsOverlayEnabled();
	}
#endif

	UE_LOGF(AdvancedSteamFriendsLog, Warning, "OpenSteamUserOverlay Couldn't init steamAPI!");
	return false;
}

UTexture2D * UAdvancedSteamFriendsLibrary::GetSteamFriendAvatar(const FBPUniqueNetId UniqueNetId, EBlueprintAsyncResultSwitch &Result, SteamAvatarSize AvatarSize)
{
#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED
	if (!UniqueNetId.IsValid() || !UniqueNetId.UniqueNetId->IsValid() || UniqueNetId.UniqueNetId->GetType() != STEAM_SUBSYSTEM)
	{
		UE_LOGF(AdvancedSteamFriendsLog, Warning, "GetSteamFriendAvatar Had a bad UniqueNetId!");
		Result = EBlueprintAsyncResultSwitch::OnFailure;
		return nullptr;
	}

	uint32 Width = 0;
	uint32 Height = 0;

	if (SteamAPI_Init())
	{

		uint64 id = *((uint64*)UniqueNetId.UniqueNetId->GetBytes());
		int Picture = 0;

		switch(AvatarSize)
		{
		case SteamAvatarSize::SteamAvatar_Small: Picture = SteamFriends()->GetSmallFriendAvatar(id); break;
		case SteamAvatarSize::SteamAvatar_Medium: Picture = SteamFriends()->GetMediumFriendAvatar(id); break;
		case SteamAvatarSize::SteamAvatar_Large: Picture = SteamFriends()->GetLargeFriendAvatar(id); break;
		default: break;
		}

		if (Picture == -1)
		{
			Result = EBlueprintAsyncResultSwitch::AsyncLoading;
			return NULL;
		}

		SteamUtils()->GetImageSize(Picture, &Width, &Height);

		if (Width > 0 && Height > 0)
		{

			uint8 *oAvatarRGBA = new uint8[Width * Height * 4];

			SteamUtils()->GetImageRGBA(Picture, (uint8*)oAvatarRGBA, 4 * Height * Width * sizeof(char));

			UTexture2D* Avatar = UTexture2D::CreateTransient(Width, Height, PF_R8G8B8A8);

			if (FTexturePlatformData* PlatformData = Avatar->GetPlatformData())
			{
				uint8* MipData = (uint8*)PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
				FMemory::Memcpy(MipData, (void*)oAvatarRGBA, Height * Width * 4);
				PlatformData->Mips[0].BulkData.Unlock();

				PlatformData->SetNumSlices(1);
				Avatar->NeverStream = true;

			}

			delete[] oAvatarRGBA;

			Avatar->UpdateResource();

			Result = EBlueprintAsyncResultSwitch::OnSuccess;
			return Avatar;
		}
		else
		{
			UE_LOGF(AdvancedSteamFriendsLog, Warning, "Bad Height / Width with steam avatar!");
		}

		Result = EBlueprintAsyncResultSwitch::OnFailure;
		return nullptr;
	}
#endif

	UE_LOGF(AdvancedSteamFriendsLog, Warning, "STEAM Couldn't be verified as initialized");
	Result = EBlueprintAsyncResultSwitch::OnFailure;
	return nullptr;
}

bool UAdvancedSteamFriendsLibrary::InitTextFiltering()
{
#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED

	if (SteamAPI_Init())
	{
		return SteamUtils()->InitFilterText();
	}

#endif

	return false;
}

bool UAdvancedSteamFriendsLibrary::FilterText(FString TextToFilter, EBPTextFilteringContext Context, const FBPUniqueNetId TextSourceID, FString& FilteredText)
{
#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED

	if (SteamAPI_Init())
	{
		uint32 BufferLen = TextToFilter.Len() + 10; 
		char* OutText = new char[BufferLen];

		uint64 id = 0;

		if (TextSourceID.IsValid())
		{
			id = *((uint64*)TextSourceID.UniqueNetId->GetBytes());
		}

		int FilterCount = SteamUtils()->FilterText((ETextFilteringContext)Context, id, TCHAR_TO_ANSI(*TextToFilter), OutText, BufferLen);

		if (FilterCount > 0)
		{
			FilteredText = FString(UTF8_TO_TCHAR(OutText));
			delete[] OutText;
			return true;
		}

		delete[] OutText;
	}

#endif

	FilteredText = TextToFilter;
	return false;
}

bool UAdvancedSteamFriendsLibrary::IsSteamInBigPictureMode()
{
#if (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX) && STEAM_SDK_INSTALLED

	if (SteamAPI_Init())
	{
		return SteamUtils()->IsSteamInBigPictureMode();
	}

#endif

	return false;
}