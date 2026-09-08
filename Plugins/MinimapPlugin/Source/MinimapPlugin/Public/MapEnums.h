

#pragma once

#include "MapEnums.generated.h"

UENUM(BlueprintType)
enum class EIconSizeUnit : uint8
{

	ScreenSpace,

	WorldSpace,
};

UENUM(BlueprintType)
enum class EMapFogRevealMode : uint8
{

	Off,

	Temporary,

	Permanent,
};

UENUM(BlueprintType)
enum class EIconFogInteraction : uint8
{

	OnlyRenderWhenRevealing,

	OnlyRenderWhenExplored,

	AlwaysRenderUnderFog,

	AlwaysRenderAboveFog,
};

UENUM(BlueprintType)
enum class EIconBackgroundInteraction : uint8
{

	AlwaysRender,

	OnlyRenderInSameVolume,

	OnlyRenderOnSameFloor,

	OnlyRenderInPriorityVolume,

	OnlyRenderOnPriorityFloor,
};

UENUM(BlueprintType)
enum class EMapViewSearchOption : uint8
{

	Any,

	OnPlayer,

	OnMapBackground,

	OnMapFog,

	Disabled,
};

UENUM(BlueprintType)
enum class EFogPostProcessVolumeOption : uint8
{

	AutoLocate,

	AutoLocateOrCreate,

	Manual,
};

UENUM(BlueprintType)
enum class EMapViewRotationMode : uint8
{

	UseFixedRotation,

	InheritYaw,
};