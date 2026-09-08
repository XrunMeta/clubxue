#pragma once
#include "TimerManager.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Containers/Queue.h"
#include "Iris/Serialization/NetSerializer.h"
#include "VRRenderTargetManager.generated.h"

class UVRRenderTargetManager;
class UCanvasRenderTarget2D;
class UCanvas;
class UTexture2D;
class UMaterial;
class APlayerController;
class FRenderCommandFence;
enum EPixelFormat : uint8;

USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPVRReplicatedTextureStore
{
	GENERATED_BODY()
public:

	UPROPERTY(Transient)
		TArray<uint8> PackedData;

	UPROPERTY(Transient)
		TArray<uint16> UnpackedData;

	UPROPERTY(Transient)
		uint32 Width;

	UPROPERTY()
		uint32 Height;

	UPROPERTY(Transient)
		bool bIsZipped;

	EPixelFormat PixelFormat;

	FBPVRReplicatedTextureStore()
	{
		Width = 0;
		Height = 0;
		bIsZipped = false;
	}

	void Reset()
	{
		PackedData.Reset();
		UnpackedData.Reset();
		Width = 0;
		Height = 0;
		PixelFormat = (EPixelFormat)0;
		bIsZipped = false;

	}

	void PackData();
	void UnPackData();

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

};

template<>
struct TStructOpsTypeTraits< FBPVRReplicatedTextureStore > : public TStructOpsTypeTraitsBase2<FBPVRReplicatedTextureStore>
{
	enum
	{
		WithNetSerializer = true,
		WithNetSharedSerialization = true,
	};
};

USTRUCT()
struct FRenderDataStore {
	GENERATED_BODY()

	TArray<FColor> ColorData;
	FRenderCommandFence RenderFence;
	FIntPoint Size2D;
	EPixelFormat PixelFormat;

	FRenderDataStore() {
	}
};

UENUM(BlueprintType)
enum class ERenderManagerOperationType : uint8
{
	Op_LineDraw = 0x00,
	Op_TriDraw = 0x01,
	Op_TexDraw = 0x02
};

USTRUCT()
struct FRenderManagerTri {
	GENERATED_BODY()
public:
	FVector2D P1;
	FVector2D P2;
	FVector2D P3;
};

USTRUCT()
struct FRenderManagerOperation {
	GENERATED_BODY()
public:

	UPROPERTY()
		uint32 OwnerID;

	UPROPERTY()
	ERenderManagerOperationType OperationType;

	UPROPERTY()
	FColor Color;

	UPROPERTY()
	FVector2D P1;

	UPROPERTY()
	FVector2D P2;

	UPROPERTY()
		uint32 Thickness;

	UPROPERTY()
	TArray<FRenderManagerTri> Tris;

	UPROPERTY()
		TSoftObjectPtr<UTexture2D> Texture;

	UPROPERTY()
		TSoftObjectPtr<UMaterial> Material;

	FRenderManagerOperation()
	{
		OwnerID = 0;
		OperationType = ERenderManagerOperationType::Op_LineDraw;
		Color = FColor::White;
		P1 = FVector2D::ZeroVector;
		P2 = FVector2D::ZeroVector;
		Thickness = 0;
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};
template<>
struct TStructOpsTypeTraits< FRenderManagerOperation > : public TStructOpsTypeTraitsBase2<FRenderManagerOperation>
{
	enum
	{
		WithNetSerializer = true,
		WithNetSharedSerialization = true,
	};
};

UCLASS(ClassGroup = (VRExpansionPlugin))
class VREXPANSIONPLUGIN_API ARenderTargetReplicationProxy : public AActor
{
	GENERATED_BODY()

public:
	ARenderTargetReplicationProxy(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_Manager)
		TObjectPtr<UVRRenderTargetManager> OwningManager;

	UPROPERTY(Replicated)
		uint32 OwnersID;

	UFUNCTION()
		void OnRep_Manager();

	UPROPERTY(Transient)
	FBPVRReplicatedTextureStore TextureStore;

	UPROPERTY(Transient)
		int32 BlobNum;

	bool bWaitingForManager;

	void SendInitMessage();

	UFUNCTION()
	void SendNextDataBlob();

	FTimerHandle SendTimer_Handle;
	FTimerHandle CheckManager_Handle;

	UPROPERTY()
		int32 TextureBlobSize;

	UPROPERTY()
		int32 MaxBytesPerSecondRate;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(Reliable, Server, WithValidation)
		void SendLocalDrawOperations(const TArray<FRenderManagerOperation>& LocalRenderOperationStoreList);

	UFUNCTION(Reliable, Client)
		void InitTextureSend(int32 Width, int32 Height, int32 TotalDataCount, int32 BlobCount, EPixelFormat PixelFormat, bool bIsZipped);

	UFUNCTION(Reliable, Server, WithValidation)
		void Ack_InitTextureSend(int32 TotalDataCount);

	UFUNCTION(Reliable, Client)
		void ReceiveTextureBlob(const TArray<uint8>& TextureBlob, int32 LocationInData, int32 BlobCount);

	UFUNCTION(Reliable, Server, WithValidation)
		void Ack_ReceiveTextureBlob(int32 BlobCount);

	UFUNCTION(Reliable, Client)
		void ReceiveTexture(const FBPVRReplicatedTextureStore&TextureData);

};

USTRUCT()
struct FClientRepData {
	GENERATED_BODY()

	UPROPERTY()
		TObjectPtr<APlayerController> PC;

	UPROPERTY()
		TObjectPtr<ARenderTargetReplicationProxy> ReplicationProxy;

	UPROPERTY()
		bool bIsRelevant;

	UPROPERTY()
		bool bIsDirty;

	FClientRepData() 
	{
		PC = nullptr;
		ReplicationProxy = nullptr;
		bIsRelevant = false;
		bIsDirty = false;
	}
};

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = (VRExpansionPlugin))
class VREXPANSIONPLUGIN_API UVRRenderTargetManager : public UActorComponent
{
	GENERATED_BODY()

public:

    UVRRenderTargetManager(const FObjectInitializer& ObjectInitializer);

	uint32 OwnerIDCounter;

	UPROPERTY(Transient)
		TObjectPtr<ARenderTargetReplicationProxy> LocalProxy;

	TArray<FRenderManagerOperation> RenderOperationStore;
	TArray<FRenderManagerOperation> LocalRenderOperationStore;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RenderTargetManager")
		float DrawRate;

	UFUNCTION(Reliable, NetMultiCast, WithValidation)
		void SendDrawOperations(const TArray<FRenderManagerOperation>& RenderOperationStoreList);

	UFUNCTION(BlueprintCallable, Category = "VRRenderTargetManager|DrawingFunctions")
		void AddLineDrawOperation(FVector2D Point1, FVector2D Point2, FColor Color, int32 Thickness);

	UFUNCTION(BlueprintCallable, Category = "VRRenderTargetManager|DrawingFunctions")
		void AddTextureDrawOperation(FVector2D Position, UTexture2D* TextureToDisplay);

	UFUNCTION(BlueprintCallable, Category = "VRRenderTargetManager|DrawingFunctions")
		void AddMaterialTrianglesDrawOperation(TArray<FCanvasUVTri> Tris, UMaterial* Material);

	void DrawOperation(UCanvas* Canvas, const FRenderManagerOperation& Operation);

	UFUNCTION()
		void DrawPoll();

	void DrawOperations();

	UPROPERTY()
		FTimerHandle DrawHandle;

	UPROPERTY(Transient)
		bool bIsStoringImage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RenderTargetManager")
		bool bInitiallyReplicateTexture;

	UPROPERTY(Transient)
		bool bIsLoadingTextureBuffer;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RenderTargetManager")
		int32 TextureBlobSize;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RenderTargetManager")
		int32 MaxBytesPerSecondRate;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "RenderTargetManager")
		TObjectPtr<UCanvasRenderTarget2D> RenderTarget;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RenderTargetManager")
		int32 RenderTargetWidth;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RenderTargetManager")
		int32 RenderTargetHeight;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RenderTargetManager")
		FColor ClearColor;

	UPROPERTY(Transient)
		TArray<FClientRepData> NetRelevancyLog;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RenderTargetManager")
		float PollRelevancyTime;

	FTimerHandle NetRelevancyTimer_Handle;

	UPROPERTY(Transient)
		FBPVRReplicatedTextureStore RenderTargetStore;

	UFUNCTION(BlueprintCallable, Category = "VRRenderTargetManager|UtilityFunctions")
		bool GenerateTrisFromBoxPlaneIntersection(UPrimitiveComponent* PrimToBoxCheck, FTransform WorldTransformOfPlane, const FPlane& LocalProjectionPlane, FVector2D PlaneSize, FColor UVColor, TArray<FCanvasUVTri>& OutTris);

	void InitRenderTarget();

	void UpdateRelevancyMap();

	bool DeCompressRenderTarget2D();

	void QueueImageStore();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	TQueue<FRenderDataStore *> RenderDataQueue;

};

USTRUCT()
struct FBPVRReplicatedTextureStoreSerializerConfig : public FNetSerializerConfig
{
	GENERATED_BODY()
};

USTRUCT()
struct FRenderManagerOperationSerializerConfig : public FNetSerializerConfig
{
	GENERATED_BODY()
};

namespace UE::Net
{
	UE_NET_DECLARE_SERIALIZER(FBPVRReplicatedTextureStoreNetSerializer, VREXPANSIONPLUGIN_API);
	UE_NET_DECLARE_SERIALIZER(FRenderManagerOperationNetSerializer, VREXPANSIONPLUGIN_API);
}
