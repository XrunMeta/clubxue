

#pragma once

#include "CoreMinimal.h"
#include "Json.h"
#include "GooglePlayGamesStructures.generated.h"

USTRUCT(BlueprintType)
struct FGPGS_Player
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Google Play Games")
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Google Play Games")
	FString PlayerID;

	UPROPERTY(BlueprintReadOnly, Category = "Google Play Games")
	int64 RetrievedTimeStamp;

	UPROPERTY(BlueprintReadOnly, Category = "Google Play Games")
	bool bHasHiResImage;

	UPROPERTY(BlueprintReadOnly, Category = "Google Play Games")
	bool bHasIconImage;

	UPROPERTY(BlueprintReadOnly, Category = "Google Play Games")
	FString HiResImageUrl;

	UPROPERTY(BlueprintReadOnly, Category = "Google Play Games")
	FString IconImageUrl;

	UPROPERTY(BlueprintReadOnly, Category = "Google Play Games")
	FString Title;

	UPROPERTY(BlueprintReadOnly, Category = "Google Play Games")
	FString BannerImageLandscapeUrl;

	UPROPERTY(BlueprintReadOnly, Category = "Google Play Games")
	FString BannerImagePortraitUrl;

	FGPGS_Player()
		: DisplayName(TEXT(""))
		, PlayerID(TEXT(""))
		, RetrievedTimeStamp(0)
		, bHasHiResImage(false)
		, bHasIconImage(false)
		, HiResImageUrl(TEXT(""))
		, IconImageUrl(TEXT(""))
		, Title(TEXT(""))
		, BannerImageLandscapeUrl(TEXT(""))
		, BannerImagePortraitUrl(TEXT(""))
	{
	}

	static FGPGS_Player ParseFromJson(const TSharedPtr<FJsonObject>& JsonObject)
	{
		FGPGS_Player Player;
		if (JsonObject.IsValid())
		{

			JsonObject->TryGetStringField(TEXT("displayName"), Player.DisplayName);
			JsonObject->TryGetStringField(TEXT("playerId"), Player.PlayerID);
			Player.RetrievedTimeStamp = static_cast<int64>(JsonObject->GetNumberField(TEXT("retrievedTimeStamp")));
			JsonObject->TryGetBoolField(TEXT("hasHiResImage"), Player.bHasHiResImage);
			JsonObject->TryGetBoolField(TEXT("hasIconImage"), Player.bHasIconImage);
			JsonObject->TryGetStringField(TEXT("hiResImageUrl"), Player.HiResImageUrl);
			JsonObject->TryGetStringField(TEXT("iconImageUrl"), Player.IconImageUrl);
			JsonObject->TryGetStringField(TEXT("title"), Player.Title);
			JsonObject->TryGetStringField(TEXT("bannerImageLandscapeUrl"), Player.BannerImageLandscapeUrl);
			JsonObject->TryGetStringField(TEXT("bannerImagePortraitUrl"), Player.BannerImagePortraitUrl);
		}
		return Player;
	}

	static FGPGS_Player ParseFromJson(const FString& JsonString)
	{
		FGPGS_Player Player;
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			if (JsonObject.IsValid())
			{

				JsonObject->TryGetStringField(TEXT("displayName"), Player.DisplayName);
				JsonObject->TryGetStringField(TEXT("playerId"), Player.PlayerID);
				Player.RetrievedTimeStamp = static_cast<int64>(JsonObject->GetNumberField(TEXT("retrievedTimeStamp")));
				JsonObject->TryGetBoolField(TEXT("hasHiResImage"), Player.bHasHiResImage);
				JsonObject->TryGetBoolField(TEXT("hasIconImage"), Player.bHasIconImage);
				JsonObject->TryGetStringField(TEXT("hiResImageUrl"), Player.HiResImageUrl);
				JsonObject->TryGetStringField(TEXT("iconImageUrl"), Player.IconImageUrl);
				JsonObject->TryGetStringField(TEXT("title"), Player.Title);
				JsonObject->TryGetStringField(TEXT("bannerImageLandscapeUrl"), Player.BannerImageLandscapeUrl);
				JsonObject->TryGetStringField(TEXT("bannerImagePortraitUrl"), Player.BannerImagePortraitUrl);
			}
		}
		return Player;
	}

	static TArray<FGPGS_Player> ParseFriendArrayFromJson(const FString& JsonString)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
		TArray<FGPGS_Player> Friends;

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{

			const TArray<TSharedPtr<FJsonValue>>* FriendsJsonArray;
			if (JsonObject->TryGetArrayField(TEXT("friends"), FriendsJsonArray))
			{

				for (const TSharedPtr<FJsonValue>& Value : *FriendsJsonArray)
				{
					TSharedPtr<FJsonObject> FriendObject = Value->AsObject();
					if (FriendObject.IsValid())
					{

						Friends.Add(FGPGS_Player::ParseFromJson(FriendObject));
					}
				}
			}
		}

		return Friends;
	}
};

USTRUCT(BlueprintType)
struct FGPGS_Event
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Google Play Games")
	FString Name;

	UPROPERTY(BlueprintReadOnly, Category = "Google Play Games")
	FString Description;

	UPROPERTY(BlueprintReadOnly, Category = "Google Play Games")
	FString EventID;

	UPROPERTY(BlueprintReadOnly, Category = "Google Play Games")
	int64 Value;

	UPROPERTY(BlueprintReadOnly, Category = "Google Play Games")
	bool bIsVisible;

	FGPGS_Event()
		: Name(TEXT(""))
		, Description(TEXT(""))
		, EventID(TEXT(""))
		, Value(0)
		, bIsVisible(false)
	{
	}

	static FGPGS_Event ParseFromJson(const FString& JsonString)
	{
		FGPGS_Event Event;

		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			if (JsonObject.IsValid())
			{
				JsonObject->TryGetStringField(TEXT("name"), Event.Name);
				JsonObject->TryGetStringField(TEXT("description"), Event.Description);
				JsonObject->TryGetStringField(TEXT("eventId"), Event.EventID);
				Event.Value = static_cast<int64>(JsonObject->GetNumberField(TEXT("value")));
				JsonObject->TryGetBoolField(TEXT("isVisible"), Event.bIsVisible);
			}
		}
		return Event;
	}
};

USTRUCT(BlueprintType)
struct FGPGS_PlayerStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Google Play Games")
	float AverageSessionLength;

	UPROPERTY(BlueprintReadOnly, Category = "Google Play Games")
	int32 DaysSinceLastPlayed;

	UPROPERTY(BlueprintReadOnly, Category = "Google Play Games")
	int32 NumberOfPurchases;

	UPROPERTY(BlueprintReadOnly, Category = "Google Play Games")
	int32 NumberOfSessions;

	UPROPERTY(BlueprintReadOnly, Category = "Google Play Games")
	float SessionPercentile;

	UPROPERTY(BlueprintReadOnly, Category = "Google Play Games")
	float SpendPercentile;

	FGPGS_PlayerStats()
		: AverageSessionLength(0)
		, DaysSinceLastPlayed(0)
		, NumberOfPurchases(0)
		, NumberOfSessions(0)
		, SessionPercentile(0)
		, SpendPercentile(0)
	{
	}

	static FGPGS_PlayerStats ParseFromJson(const FString& JsonString)
	{
		FGPGS_PlayerStats PlayerStats;
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			if (JsonObject.IsValid())
			{
				PlayerStats.AverageSessionLength = static_cast<float>(JsonObject->GetNumberField(TEXT("averageSessionLength")));
				PlayerStats.DaysSinceLastPlayed = static_cast<int32>(JsonObject->GetNumberField(TEXT("daysSinceLastPlayed")));
				PlayerStats.NumberOfPurchases = static_cast<int32>(JsonObject->GetNumberField(TEXT("numberOfPurchases")));
				PlayerStats.NumberOfSessions = static_cast<int32>(JsonObject->GetNumberField(TEXT("numberOfSessions")));
				PlayerStats.SessionPercentile = static_cast<float>(JsonObject->GetNumberField(TEXT("sessionPercentile")));
				PlayerStats.SpendPercentile = static_cast<float>(JsonObject->GetNumberField(TEXT("spendPercentile")));
			}
		}
		return PlayerStats;
	}

};

