

#pragma once

#include "Components/ActorComponent.h"
#include "MapTrackerComponent.generated.h"

class UMapIconComponent;
class UMapRevealerComponent;
class AMapBackground;
class AMapFog;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMapIconRegisteredSignature, UMapIconComponent*, MapIcon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMapIconUnregisteredSignature, UMapIconComponent*, MapIcon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMapBackgroundRegisteredSignature, AMapBackground*, MapBackground);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMapBackgroundUnregisteredSignature, AMapBackground*, MapBackground);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMapFogRegisteredSignature, AMapFog*, MapFog);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMapFogUnregisteredSignature, AMapFog*, MapFog);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMapRevealerRegisteredSignature, UMapRevealerComponent*, MapRevealer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMapRevealerUnregisteredSignature, UMapRevealerComponent*, MapRevealer);

UCLASS(ClassGroup=(MinimapPlugin), meta=(BlueprintSpawnableComponent))
class MINIMAPPLUGIN_API UMapTrackerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMapTrackerComponent();

	void RegisterMapIcon(UMapIconComponent* MapIcon);

	void UnregisterMapIcon(UMapIconComponent* MapIcon);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	const TArray<UMapIconComponent*>& GetMapIcons() const;

	void RegisterMapBackground(AMapBackground* MapBackground);

	void UnregisterMapBackground(AMapBackground* MapBackground);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	const TArray<AMapBackground*>& GetMapBackgrounds() const;

	void RegisterMapFog(AMapFog* MapFog);

	void UnregisterMapFog(AMapFog* MapFog);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	const TArray<AMapFog*>& GetMapFogs() const;

	UFUNCTION(BlueprintPure, Category = "Minimap")
	bool HasMapFog() const;

	UFUNCTION(BlueprintPure, Category = "Minimap")
	float GetFogRevealedFactor(const FVector& WorldLocation, const bool bRequireCurrentlyRevealing, bool& bIsInsideFogVolume) const;

	void RegisterMapRevealer(UMapRevealerComponent* MapRevealer);

	void UnregisterMapRevealer(UMapRevealerComponent* MapRevealer);

	UFUNCTION(BlueprintPure, Category = "Minimap")
	const TArray<UMapRevealerComponent*>& GetMapRevealers() const;

public:

	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FMapIconRegisteredSignature OnMapIconRegistered;

	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FMapIconUnregisteredSignature OnMapIconUnregistered;

	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FMapBackgroundRegisteredSignature OnMapBackgroundRegistered;

	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FMapBackgroundUnregisteredSignature OnMapBackgroundUnregistered;

	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FMapFogRegisteredSignature OnMapFogRegistered;

	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FMapFogRegisteredSignature OnMapFogUnregistered;

	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FMapRevealerRegisteredSignature OnMapRevealerRegistered;

	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FMapRevealerUnregisteredSignature OnMapRevealerUnregistered;

private:

	UPROPERTY(Transient)
	TArray<UMapIconComponent*> MapIcons;

	UPROPERTY(Transient)
	TArray<AMapBackground*> MapBackgrounds;

	UPROPERTY(Transient)
	TArray<AMapFog*> MapFogs;

	UPROPERTY(Transient)
	TArray<UMapRevealerComponent*> MapRevealers;

};
