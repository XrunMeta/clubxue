

#include "Misc/VRRenderTargetManager.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(VRRenderTargetManager)

#include "Engine/World.h"
#include "GlobalRenderResources.h"
#include "Components/ActorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#include "PixelFormat.h"
#include "CanvasTypes.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "Engine/Canvas.h"
#include "GeomTools.h"
#include "Serialization/ArchiveSaveCompressedProxy.h"
#include "Serialization/ArchiveLoadCompressedProxy.h"
#include "Materials/Material.h"
#include "Net/UnrealNetwork.h"

#include "Serializers/SerializerHelpers.h"
#include "Iris/Serialization/ObjectNetSerializer.h"
#include "Iris/Core/NetObjectReference.h"
#include "Iris/Serialization/NetSerializerDelegates.h"
#include "Iris/Serialization/NetSerializers.h"
#include "Iris/ReplicationState/PropertyNetSerializerInfoRegistry.h"
#include "Iris/ReplicationState/ReplicationStateDescriptorBuilder.h"
#include "Templates/IsPODType.h"

#include "Iris/Serialization/SoftObjectNetSerializers.h"
#include "Iris/Serialization/NetSerializerArrayStorage.h"

namespace RLE_Funcs
{
	enum RLE_Flags
	{
		RLE_CompressedByte = 1,
		RLE_CompressedShort = 2,
		RLE_Compressed24 = 3,
		RLE_NotCompressedByte = 4,
		RLE_NotCompressedShort = 5,
		RLE_NotCompressed24 = 6,

		RLE_ContinueRunByte = 7,
		RLE_ContinueRunShort = 8,
		RLE_ContinueRun24 = 9
	};

	template <typename DataType>
	static bool RLEEncodeLine(TArray<DataType>* LineToEncode, TArray<uint8>* EncodedLine);

	template <typename DataType>
	static bool RLEEncodeBuffer(DataType* BufferToEncode, uint32 EncodeLength, TArray<uint8>* EncodedLine);

	template <typename DataType>
	static void RLEDecodeLine(TArray<uint8>* LineToDecode, TArray<DataType>* DecodedLine, bool bCompressed);

	template <typename DataType>
	static void RLEDecodeLine(const uint8* LineToDecode, uint32 Num, TArray<DataType>* DecodedLine, bool bCompressed);

	static inline void RLEWriteContinueFlag(uint32 Count, uint8** loc);

	template <typename DataType>
	static inline void RLEWriteRunFlag(uint32 Count, uint8** loc, TArray<DataType>& Data, bool bCompressed);
}

UVRRenderTargetManager::UVRRenderTargetManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	PollRelevancyTime = 0.1f;
	DrawRate = 0.0333f;

	bIsStoringImage = false;
	RenderTarget = nullptr;
	RenderTargetWidth = 100;
	RenderTargetHeight = 100;
	ClearColor = FColor::White;

	TextureBlobSize = 512;
	MaxBytesPerSecondRate = 5000;

	bInitiallyReplicateTexture = false;
	bIsLoadingTextureBuffer = false;

	OwnerIDCounter = 0;
}

bool UVRRenderTargetManager::SendDrawOperations_Validate(const TArray<FRenderManagerOperation>& RenderOperationStoreList)
{
	return true;
}

void UVRRenderTargetManager::SendDrawOperations_Implementation(const TArray<FRenderManagerOperation>& RenderOperationStoreList)
{
	if (GetNetMode() == ENetMode::NM_Client)
	{
		RenderOperationStore.Append(RenderOperationStoreList);
	}

	DrawOperations();
}

void UVRRenderTargetManager::AddLineDrawOperation(FVector2D Point1, FVector2D Point2, FColor Color, int32 Thickness)
{
	FRenderManagerOperation NewOperation;
	NewOperation.OperationType = ERenderManagerOperationType::Op_LineDraw;
	NewOperation.Color = Color;
	NewOperation.P1 = Point1;
	NewOperation.P2 = Point2;
	NewOperation.Thickness = (uint32)Thickness;

	if (GetNetMode() < ENetMode::NM_Client)
		RenderOperationStore.Add(NewOperation);
	else
		LocalRenderOperationStore.Add(NewOperation);

	if (!DrawHandle.IsValid())
		GetWorld()->GetTimerManager().SetTimer(DrawHandle, this, &UVRRenderTargetManager::DrawPoll, DrawRate, true);

}

void UVRRenderTargetManager::AddTextureDrawOperation(FVector2D Position, UTexture2D* TextureToDisplay)
{

	if (!TextureToDisplay)
		return;

	FRenderManagerOperation NewOperation;
	NewOperation.OperationType = ERenderManagerOperationType::Op_TexDraw;
	NewOperation.P1 = Position;
	NewOperation.Texture = TextureToDisplay;

	if (GetNetMode() < ENetMode::NM_Client)
		RenderOperationStore.Add(NewOperation);
	else
		LocalRenderOperationStore.Add(NewOperation);

	if (!DrawHandle.IsValid())
		GetWorld()->GetTimerManager().SetTimer(DrawHandle, this, &UVRRenderTargetManager::DrawPoll, DrawRate, true);

}

void UVRRenderTargetManager::AddMaterialTrianglesDrawOperation(TArray<FCanvasUVTri> Tris, UMaterial* Material)
{

	if (!Tris.Num())
		return;

	FRenderManagerOperation NewOperation;
	NewOperation.OperationType = ERenderManagerOperationType::Op_TriDraw;
	NewOperation.Color = Tris[0].V0_Color.ToFColor(true);

	NewOperation.Tris.AddUninitialized(Tris.Num());
	int Counter = 0;
	FRenderManagerTri RenderTri;
	for (FCanvasUVTri Tri : Tris)
	{
		RenderTri.P1 = Tri.V0_Pos;
		RenderTri.P2 = Tri.V1_Pos;
		RenderTri.P3 = Tri.V2_Pos;
		NewOperation.Tris[Counter++] = RenderTri;
	}

	NewOperation.Material = Material;

	if (GetNetMode() < ENetMode::NM_Client)
		RenderOperationStore.Add(NewOperation);
	else
		LocalRenderOperationStore.Add(NewOperation);

	if (!DrawHandle.IsValid())
		GetWorld()->GetTimerManager().SetTimer(DrawHandle, this, &UVRRenderTargetManager::DrawPoll, DrawRate, true);

}

void UVRRenderTargetManager::DrawOperation(UCanvas* Canvas, const FRenderManagerOperation& Operation)
{
	if (IsValid(LocalProxy) && LocalProxy->OwnersID == Operation.OwnerID)
	{
		return;
	}

	switch (Operation.OperationType)
	{
	case ERenderManagerOperationType::Op_LineDraw:
	{
		FCanvasLineItem LineItem;
		LineItem.Origin = FVector(Operation.P1.X, Operation.P1.Y, 0.f);
		LineItem.EndPos = FVector(Operation.P2.X, Operation.P2.Y, 0.f);
		LineItem.LineThickness = (float)Operation.Thickness;
		LineItem.SetColor(Operation.Color.ReinterpretAsLinear());
		Canvas->DrawItem(LineItem);
	}break;
	case ERenderManagerOperationType::Op_TexDraw:
	{
		if (Operation.Texture && Operation.Texture->GetResource())
		{

			FCanvasTileItem TileItem(Operation.P1, Operation.Texture->GetResource(), FVector2D(Operation.Texture->GetSizeX(), Operation.Texture->GetSizeY()), FVector2D(0, 0), FVector2D(1.f, 1.f), ClearColor);
			TileItem.BlendMode = FCanvas::BlendToSimpleElementBlend(EBlendMode::BLEND_Translucent);
			Canvas->DrawItem(TileItem);
		}
	}break;
	case ERenderManagerOperationType::Op_TriDraw:
	{
		if (Operation.Tris.Num() && Operation.Material)
		{
			FCanvasTriangleItem TriangleItem(FVector2D::ZeroVector, FVector2D::ZeroVector, FVector2D::ZeroVector, NULL);
			TriangleItem.MaterialRenderProxy = Operation.Material->GetRenderProxy();

			FCanvasUVTri triStore;
			triStore.V0_Color = Operation.Color;
			triStore.V1_Color = Operation.Color;
			triStore.V2_Color = Operation.Color;

			TriangleItem.TriangleList.Reset(Operation.Tris.Num());
			TriangleItem.TriangleList.AddUninitialized(Operation.Tris.Num());
			uint32 Counter = 0;
			for (FRenderManagerTri Tri : Operation.Tris)
			{
				triStore.V0_Pos = Tri.P1;
				triStore.V1_Pos = Tri.P2;
				triStore.V2_Pos = Tri.P3;
				TriangleItem.TriangleList[Counter++] = triStore;
			}

			Canvas->DrawItem(TriangleItem);
		}
	}break;
	}

}

void UVRRenderTargetManager::DrawPoll()
{
	if (!RenderOperationStore.Num() && !LocalRenderOperationStore.Num())
	{
		GetWorld()->GetTimerManager().ClearTimer(DrawHandle);
		return;
	}

	if (GetNetMode() < ENetMode::NM_Client)
	{
		SendDrawOperations(RenderOperationStore);
	}
	else
	{
		if (LocalRenderOperationStore.Num())
		{

			if (IsValid(LocalProxy))
			{
				LocalProxy->SendLocalDrawOperations(LocalRenderOperationStore);
			}

			RenderOperationStore.Append(LocalRenderOperationStore);
			LocalRenderOperationStore.Empty();
		}

		DrawOperations();
	}
}

void UVRRenderTargetManager::DrawOperations()
{

	if (bIsLoadingTextureBuffer)
	{
		if (!DrawHandle.IsValid())
			GetWorld()->GetTimerManager().SetTimer(DrawHandle, this, &UVRRenderTargetManager::DrawPoll, DrawRate, true);

		return;
	}

	if (GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		RenderOperationStore.Empty();
		return;
	}

	UWorld* World = GetWorld();

	if (!World || !World->GetBegunPlay())
		return;

	FTextureRenderTargetResource* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();

	if (!RenderTargetResource)
	{
		RenderOperationStore.Empty();
		return;
	}

	UCanvas* CanvasToUse = World->GetCanvasForDrawMaterialToRenderTarget();

	FCanvas RenderCanvas(
		RenderTargetResource,
		nullptr,
		World,
		World->GetFeatureLevel());

	CanvasToUse->Init(RenderTarget->SizeX, RenderTarget->SizeY, nullptr, &RenderCanvas);
	CanvasToUse->Update();

	if (CanvasToUse)
	{
		for (const FRenderManagerOperation& opt : RenderOperationStore)
		{
			DrawOperation(CanvasToUse, opt);
		}

		RenderOperationStore.Empty();

		RenderCanvas.Flush_GameThread();

		CanvasToUse->Canvas = NULL;
	}
}

ARenderTargetReplicationProxy::ARenderTargetReplicationProxy(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bOnlyRelevantToOwner = true;
	bNetUseOwnerRelevancy = true;
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;
	SetReplicateMovement(false);
	bWaitingForManager = false;
}

void ARenderTargetReplicationProxy::OnRep_Manager()
{

	if (IsValid(OwningManager))
	{
		OwningManager->LocalProxy = this;

		if (bWaitingForManager)
		{
			OwningManager->bIsLoadingTextureBuffer = false;
			OwningManager->RenderTargetStore = TextureStore;
			TextureStore.Reset();
			TextureStore.PackedData.Empty();
			TextureStore.UnpackedData.Empty();

			OwningManager->DeCompressRenderTarget2D();
			bWaitingForManager = false;
		}
	}
}

bool ARenderTargetReplicationProxy::SendLocalDrawOperations_Validate(const TArray<FRenderManagerOperation>& LocalRenderOperationStoreList)
{
	return true;
}

void ARenderTargetReplicationProxy::SendLocalDrawOperations_Implementation(const TArray<FRenderManagerOperation>& LocalRenderOperationStoreList)
{
	if (IsValid(OwningManager))
	{
		OwningManager->RenderOperationStore.Append(LocalRenderOperationStoreList);

		if (APlayerController* OwningPlayer = Cast<APlayerController>(GetOwner()))
		{
			for (int i = (OwningManager->RenderOperationStore.Num() - LocalRenderOperationStoreList.Num()); i < OwningManager->RenderOperationStore.Num(); i++)
			{
				OwningManager->RenderOperationStore[i].OwnerID = OwnersID;
			}
		}

		if (!OwningManager->DrawHandle.IsValid())
			GetWorld()->GetTimerManager().SetTimer(OwningManager->DrawHandle, OwningManager.Get(), &UVRRenderTargetManager::DrawPoll, OwningManager->DrawRate, true);
	}
}

void ARenderTargetReplicationProxy::ReceiveTexture_Implementation(const FBPVRReplicatedTextureStore& TextureData)
{
	if (IsValid(OwningManager))
	{
		OwningManager->RenderTargetStore = TextureData;

		OwningManager->DeCompressRenderTarget2D();
	}
}

void ARenderTargetReplicationProxy::InitTextureSend_Implementation(int32 Width, int32 Height, int32 TotalDataCount, int32 BlobCount, EPixelFormat PixelFormat, bool bIsZipped)
{
	TextureStore.Reset();
	TextureStore.PixelFormat = PixelFormat;
	TextureStore.bIsZipped = bIsZipped;

	TextureStore.Width = Width;
	TextureStore.Height = Height;

	TextureStore.PackedData.Reset(TotalDataCount);
	TextureStore.PackedData.AddUninitialized(TotalDataCount);

	BlobNum = BlobCount;

	if (IsValid(OwningManager))
	{
		OwningManager->bIsLoadingTextureBuffer = true;
	}

	Ack_InitTextureSend(TotalDataCount);
}

bool ARenderTargetReplicationProxy::Ack_InitTextureSend_Validate(int32 TotalDataCount)
{
	return true;
}

void ARenderTargetReplicationProxy::Ack_InitTextureSend_Implementation(int32 TotalDataCount)
{
	if (TotalDataCount == TextureStore.PackedData.Num())
	{
		BlobNum = 0;

		float SendRate = 1.f / (MaxBytesPerSecondRate / (float)TextureBlobSize);

		GetWorld()->GetTimerManager().SetTimer(SendTimer_Handle, this, &ARenderTargetReplicationProxy::SendNextDataBlob, SendRate, true);

	}
}

void ARenderTargetReplicationProxy::SendInitMessage()
{
	int32 TotalBlobs = TextureStore.PackedData.Num() / TextureBlobSize + (TextureStore.PackedData.Num() % TextureBlobSize > 0 ? 1 : 0);

	InitTextureSend(TextureStore.Width, TextureStore.Height, TextureStore.PackedData.Num(), TotalBlobs, TextureStore.PixelFormat, TextureStore.bIsZipped);

}

void ARenderTargetReplicationProxy::SendNextDataBlob()
{
	if (!IsValidChecked(this) || !this->GetOwner() || !IsValid(this->GetOwner()))
	{	
		TextureStore.Reset();
		TextureStore.PackedData.Empty();
		TextureStore.UnpackedData.Empty();
		BlobNum = 0;
		if (SendTimer_Handle.IsValid())
			GetWorld()->GetTimerManager().ClearTimer(SendTimer_Handle);

		return;
	}

	BlobNum++;
	int32 TotalBlobs = TextureStore.PackedData.Num() / TextureBlobSize + (TextureStore.PackedData.Num() % TextureBlobSize > 0 ? 1 : 0);

	if (BlobNum <= TotalBlobs)
	{
		TArray<uint8> BlobStore;
		int32 BlobLen = (BlobNum == TotalBlobs ? TextureStore.PackedData.Num() % TextureBlobSize : TextureBlobSize);

		BlobStore.AddUninitialized(BlobLen);
		uint8* MemLoc = TextureStore.PackedData.GetData();
		int32 MemCount = (BlobNum - 1) * TextureBlobSize;
		MemLoc += MemCount;

		FMemory::Memcpy(BlobStore.GetData(), MemLoc, BlobLen);

		ReceiveTextureBlob(BlobStore, MemCount, BlobNum);
	}
	else
	{
		TextureStore.Reset();
		TextureStore.PackedData.Empty();
		TextureStore.UnpackedData.Empty();
		if (SendTimer_Handle.IsValid())
			GetWorld()->GetTimerManager().ClearTimer(SendTimer_Handle);
		BlobNum = 0;
	}
}

void ARenderTargetReplicationProxy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (SendTimer_Handle.IsValid())
		GetWorld()->GetTimerManager().ClearTimer(SendTimer_Handle);

	Super::EndPlay(EndPlayReason);
}

void ARenderTargetReplicationProxy::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams PushModelParamsWithCondition{ COND_InitialOnly, REPNOTIFY_OnChanged, true };

	DOREPLIFETIME_WITH_PARAMS_FAST(ARenderTargetReplicationProxy, OwningManager, PushModelParamsWithCondition);
	DOREPLIFETIME_WITH_PARAMS_FAST(ARenderTargetReplicationProxy, OwnersID, PushModelParamsWithCondition);
}

void ARenderTargetReplicationProxy::ReceiveTextureBlob_Implementation(const TArray<uint8>& TextureBlob, int32 LocationInData, int32 BlobNumber)
{
	if (LocationInData + TextureBlob.Num() <= TextureStore.PackedData.Num())
	{
		uint8* MemLoc = TextureStore.PackedData.GetData();
		MemLoc += LocationInData;
		FMemory::Memcpy(MemLoc, TextureBlob.GetData(), TextureBlob.Num());

	}

	if (BlobNumber == BlobNum)
	{
		Ack_ReceiveTextureBlob(BlobNum);

		if (IsValid(OwningManager))
		{
			OwningManager->bIsLoadingTextureBuffer = false;
			OwningManager->RenderTargetStore = TextureStore;
			TextureStore.Reset();
			TextureStore.PackedData.Empty();
			TextureStore.UnpackedData.Empty();

			OwningManager->DeCompressRenderTarget2D();
		}
		else
		{
			bWaitingForManager = true;
		}
	}

}

bool ARenderTargetReplicationProxy::Ack_ReceiveTextureBlob_Validate(int32 BlobCount)
{
	return true;
}

void ARenderTargetReplicationProxy::Ack_ReceiveTextureBlob_Implementation(int32 BlobCount)
{

}

void UVRRenderTargetManager::UpdateRelevancyMap()
{
	AActor* myOwner = GetOwner();

	for (int i = NetRelevancyLog.Num() - 1; i >= 0; i--)
	{
		if (!IsValid(NetRelevancyLog[i].PC) || NetRelevancyLog[i].PC->IsLocalController() || !NetRelevancyLog[i].PC->GetPawn())
		{
			NetRelevancyLog[i].ReplicationProxy->Destroy();
			NetRelevancyLog.RemoveAt(i);
		}
		else
		{
			if (APawn* pawn = NetRelevancyLog[i].PC->GetPawn())
			{
				if (!myOwner->IsNetRelevantFor(NetRelevancyLog[i].PC.Get(), pawn, pawn->GetActorLocation()))
				{
					NetRelevancyLog[i].bIsRelevant = false;
					NetRelevancyLog[i].bIsDirty = false;

				}
			}
		}
	}

	bool bHadDirtyActors = false;

	for (FConstPlayerControllerIterator PCIt = GetWorld()->GetPlayerControllerIterator(); PCIt; ++PCIt)
	{
		if (APlayerController* PC = PCIt->Get())
		{
			if (PC->IsLocalController())
				continue;

			if (!PC->HasClientLoadedCurrentWorld())
				continue;

			if (APawn* pawn = PC->GetPawn())
			{

				if (myOwner->IsNetRelevantFor(PC, pawn, pawn->GetActorLocation()))
				{
					FClientRepData* RepData = NetRelevancyLog.FindByPredicate([PC](const FClientRepData& Other)
						{
							return Other.PC == PC;
						});

					if (!RepData)
					{
						FClientRepData ClientRepData;

						FTransform NewTransform = this->GetOwner()->GetActorTransform();
						ARenderTargetReplicationProxy* RenderProxy = GetWorld()->SpawnActorDeferred<ARenderTargetReplicationProxy>(ARenderTargetReplicationProxy::StaticClass(), NewTransform, PC);
						if (RenderProxy)
						{
							RenderProxy->OwnersID = ++OwnerIDCounter;
							RenderProxy->OwningManager = this;
							RenderProxy->MaxBytesPerSecondRate = MaxBytesPerSecondRate;
							RenderProxy->TextureBlobSize = TextureBlobSize;
							UGameplayStatics::FinishSpawningActor(RenderProxy, NewTransform);
						}

						if (RenderProxy)
						{
							RenderProxy->AttachToActor(this->GetOwner(), FAttachmentTransformRules::SnapToTargetIncludingScale);

							ClientRepData.PC = PC;
							ClientRepData.ReplicationProxy = RenderProxy;
							ClientRepData.bIsRelevant = true;
							ClientRepData.bIsDirty = true;
							bHadDirtyActors = true;
							NetRelevancyLog.Add(ClientRepData);
						}

					}
					else
					{
						if (!RepData->bIsRelevant)
						{
							RepData->bIsRelevant = true;
							RepData->bIsDirty = true;
							bHadDirtyActors = true;
						}
					}
				}
			}

		}
	}

	if (bHadDirtyActors && bInitiallyReplicateTexture && GetNetMode() != ENetMode::NM_DedicatedServer)
	{
		QueueImageStore();
	}
}

bool UVRRenderTargetManager::DeCompressRenderTarget2D()
{
	if (!RenderTarget)
		return false;

	RenderTargetStore.UnPackData();

	int32 Width = RenderTargetStore.Width;
	int32 Height = RenderTargetStore.Height;
	EPixelFormat PixelFormat = RenderTargetStore.PixelFormat;
	uint8 PixelFormat8 = 0;

	TArray<FColor> FinalColorData;
	FinalColorData.AddUninitialized(RenderTargetStore.UnpackedData.Num());

	uint32 Counter = 0;
	FColor ColorVal;
	ColorVal.A = 0xFF;
	for (uint16 CompColor : RenderTargetStore.UnpackedData)
	{

		ColorVal.R = CompColor << 3;
		ColorVal.G = CompColor >> 5 << 2;
		ColorVal.B = CompColor >> 11 << 3;
		ColorVal.A = 0xFF;
		FinalColorData[Counter++] = ColorVal;
	}

	UTexture2D* RenderBase = UTexture2D::CreateTransient(Width, Height, PF_R8G8B8A8);

	uint8* MipData = (uint8*)RenderBase->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(MipData, (void*)FinalColorData.GetData(), FinalColorData.Num() * sizeof(FColor));
	RenderBase->GetPlatformData()->Mips[0].BulkData.Unlock();

	RenderBase->GetPlatformData()->SetNumSlices(1);
	RenderBase->NeverStream = true;
	RenderBase->SRGB = true;

	RenderBase->UpdateResource();

	UWorld* World = GetWorld();

	FTextureRenderTargetResource* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();

	UCanvas* CanvasToUse = World->GetCanvasForDrawMaterialToRenderTarget();

	FCanvas RenderCanvas(
		RenderTargetResource,
		nullptr,
		World,
		World->GetFeatureLevel());

	CanvasToUse->Init(RenderTarget->SizeX, RenderTarget->SizeY, nullptr, &RenderCanvas);
	CanvasToUse->Update();

	if (CanvasToUse)
	{
		FTexture* RenderTextureResource = (RenderBase) ? RenderBase->GetResource() : GWhiteTexture;
		FCanvasTileItem TileItem(FVector2D(0, 0), RenderTextureResource, FVector2D(RenderTarget->SizeX, RenderTarget->SizeY), FVector2D(0, 0), FVector2D(1.f, 1.f), FLinearColor::White);
		TileItem.BlendMode = FCanvas::BlendToSimpleElementBlend(EBlendMode::BLEND_Opaque);
		CanvasToUse->DrawItem(TileItem);

		RenderCanvas.Flush_GameThread();

		CanvasToUse->Canvas = NULL;
	}

	RenderBase->ReleaseResource();
	RenderBase->MarkAsGarbage();

	return true;
}

void UVRRenderTargetManager::QueueImageStore()
{

	if (!bInitiallyReplicateTexture || !RenderTarget || bIsStoringImage || GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		return;
	}

	bIsStoringImage = true;

	FRenderDataStore* renderData = new FRenderDataStore();

	FTextureRenderTargetResource* renderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();

	if (!renderTargetResource)
		return;

	renderData->Size2D = renderTargetResource->GetSizeXY();
	renderData->PixelFormat = RenderTarget->GetFormat();

	struct FReadSurfaceContext {
		FRenderTarget* SrcRenderTarget;
		TArray<FColor>* OutData;
		FIntRect Rect;
		FReadSurfaceDataFlags Flags;
	};

	FReadSurfaceContext readSurfaceContext =
	{
		renderTargetResource,
		&(renderData->ColorData),
		FIntRect(0,0,renderTargetResource->GetSizeXY().X, renderTargetResource->GetSizeXY().Y),
		FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX)
	};

	ENQUEUE_RENDER_COMMAND(SceneDrawCompletion)(
		[readSurfaceContext](FRHICommandListImmediate& RHICmdList) {
			RHICmdList.ReadSurfaceData(
				readSurfaceContext.SrcRenderTarget->GetRenderTargetTexture(),
				readSurfaceContext.Rect,
				*readSurfaceContext.OutData,
				readSurfaceContext.Flags
			);
		});

	RenderDataQueue.Enqueue(renderData);

	renderData->RenderFence.BeginFence();

	this->SetComponentTickEnabled(true);
}

void UVRRenderTargetManager::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bInitiallyReplicateTexture || RenderDataQueue.IsEmpty() || GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		SetComponentTickEnabled(false);
	}
	else
	{

		FRenderDataStore* nextRenderData;
		RenderDataQueue.Peek(nextRenderData);

		if (nextRenderData)
		{
			if (nextRenderData->RenderFence.IsFenceComplete())
			{
				bIsStoringImage = false;
				RenderTargetStore.Reset();
				uint32 SizeOfData = nextRenderData->ColorData.Num();

				RenderTargetStore.UnpackedData.Reset(SizeOfData);
				RenderTargetStore.UnpackedData.AddUninitialized(SizeOfData);

				uint16 ColorVal = 0;
				uint32 Counter = 0;

				for (FColor col : nextRenderData->ColorData)
				{
					ColorVal = (col.R >> 3) << 11 | (col.G >> 2) << 5 | (col.B >> 3);
					RenderTargetStore.UnpackedData[Counter++] = ColorVal;
				}

				FIntPoint Size2D = nextRenderData->Size2D;
				RenderTargetStore.Width = Size2D.X;
				RenderTargetStore.Height = Size2D.Y;
				RenderTargetStore.PixelFormat = nextRenderData->PixelFormat;
				RenderTargetStore.PackData();

				RenderDataQueue.Pop();
				delete nextRenderData;

				for (int i = NetRelevancyLog.Num() - 1; i >= 0; i--)
				{
					if (NetRelevancyLog[i].bIsDirty && IsValid(NetRelevancyLog[i].PC) && !NetRelevancyLog[i].PC->IsLocalController())
					{
						if (IsValid(NetRelevancyLog[i].ReplicationProxy))
						{
							NetRelevancyLog[i].ReplicationProxy->TextureStore = RenderTargetStore;
							NetRelevancyLog[i].ReplicationProxy->SendInitMessage();
							NetRelevancyLog[i].bIsDirty = false;
						}
					}
				}

			}
		}
	}

}

void UVRRenderTargetManager::BeginPlay()
{
	Super::BeginPlay();

	InitRenderTarget();

	if (GetNetMode() < ENetMode::NM_Client)
		GetWorld()->GetTimerManager().SetTimer(NetRelevancyTimer_Handle, this, &UVRRenderTargetManager::UpdateRelevancyMap, PollRelevancyTime, true);
}

void UVRRenderTargetManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	FRenderDataStore* Store = nullptr;
	while (!RenderDataQueue.IsEmpty())
	{
		RenderDataQueue.Dequeue(Store);

		if (Store)
		{
			delete Store;
		}
	}

	if (GetNetMode() < ENetMode::NM_Client)
		GetWorld()->GetTimerManager().ClearTimer(NetRelevancyTimer_Handle);

	if(DrawHandle.IsValid())
		GetWorld()->GetTimerManager().ClearTimer(DrawHandle);

	if (RenderTarget)
	{
		RenderTarget->ReleaseResource();
		RenderTarget = nullptr;
	}

	for (FClientRepData& RepData : NetRelevancyLog)
	{
		RepData.PC = nullptr;
		if (IsValid(RepData.ReplicationProxy.Get()))
		{
			RepData.ReplicationProxy->Destroy();
		}

		RepData.ReplicationProxy = nullptr;
	}

}

void UVRRenderTargetManager::InitRenderTarget()
{
	if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		return; 
	}

	UWorld* World = GetWorld();

	if (RenderTargetWidth > 0 && RenderTargetHeight > 0 && World)
	{
		RenderTarget = NewObject<UCanvasRenderTarget2D>(this);
		if (RenderTarget)
		{

			RenderTarget->InitAutoFormat(RenderTargetWidth, RenderTargetHeight);
			RenderTarget->ClearColor = ClearColor;
			RenderTarget->bAutoGenerateMips = false;
			RenderTarget->UpdateResourceImmediate(true);
		}
		else
		{
			RenderTarget = nullptr;
		}
	}
	else
	{
		RenderTarget = nullptr;
	}
}

bool UVRRenderTargetManager::GenerateTrisFromBoxPlaneIntersection(UPrimitiveComponent* PrimToBoxCheck, FTransform WorldTransformOfPlane, const FPlane& LocalProjectionPlane, FVector2D PlaneSize, FColor UVColor, TArray<FCanvasUVTri>& OutTris)
{

	if (!PrimToBoxCheck)
		return false;

	OutTris.Reset();

	FBoxSphereBounds LocalBounds = PrimToBoxCheck->CalcLocalBounds();
	FVector Center = LocalBounds.Origin;
	FVector Extent = LocalBounds.BoxExtent;

	FTransform LocalTrans = PrimToBoxCheck->GetComponentTransform() * WorldTransformOfPlane.Inverse();

	FVector BoxMin = Center - Extent;
	FVector BoxMax = Center + Extent;

	TArray<FVector> PointList;
	PointList.AddUninitialized(8); 

	PointList[0] = LocalTrans.TransformPosition(BoxMin);
	PointList[1] = LocalTrans.TransformPosition(BoxMax);
	PointList[2] = LocalTrans.TransformPosition(FVector(BoxMin.X, BoxMin.Y, BoxMax.Z));
	PointList[3] = LocalTrans.TransformPosition(FVector(BoxMin.X, BoxMax.Y, BoxMin.Z));
	PointList[4] = LocalTrans.TransformPosition(FVector(BoxMax.X, BoxMin.Y, BoxMin.Z));
	PointList[5] = LocalTrans.TransformPosition(FVector(BoxMin.X, BoxMax.Y, BoxMax.Z));
	PointList[6] = LocalTrans.TransformPosition(FVector(BoxMax.X, BoxMin.Y, BoxMax.Z));
	PointList[7] = LocalTrans.TransformPosition(FVector(BoxMax.X, BoxMax.Y, BoxMin.Z));

	int EdgeList[24] =
	{
		0, 3,
		0, 4,
		0, 2,
		2, 5,
		2, 6,
		4, 7,
		4, 6,
		6, 1,
		1, 7,
		1, 5,
		5, 3,
		3, 7
	};

	TArray<FVector2D> IntersectionPoints;

	FVector Intersection;
	float Time;

	FVector2D HalfPlane = PlaneSize / 2.f;
	FVector2D PtCenter;
	FVector2D NewPt;
	FVector PlanePoint;
	int CenterCount = 0;
	for (int i = 0; i < 24; i += 2)
	{

		if (UKismetMathLibrary::LinePlaneIntersection(PointList[EdgeList[i]], PointList[EdgeList[i + 1]], LocalProjectionPlane, Time, Intersection))
		{

			PlanePoint = Intersection;

			if (IsValid(RenderTarget))
			{
				NewPt.X = ((PlanePoint.X + HalfPlane.X) / PlaneSize.X) * RenderTarget->SizeX;
				NewPt.Y = ((PlanePoint.Y + HalfPlane.Y) / PlaneSize.Y) * RenderTarget->SizeY;
			}
			else
			{
				NewPt.X = ((PlanePoint.X + HalfPlane.X) / PlaneSize.X) * RenderTargetWidth;
				NewPt.Y = ((PlanePoint.Y + HalfPlane.Y) / PlaneSize.Y) * RenderTargetHeight;
			}

			IntersectionPoints.Add(NewPt);
			PtCenter += NewPt;
			CenterCount++;
		}
	}

	if (IntersectionPoints.Num() <= 2)
	{
		return false;
	}

	PtCenter /= CenterCount;

	struct FPointSortCompare
	{
	public:
		FVector2D CenterPoint;
		FPointSortCompare(const FVector2D& InCenterPoint)
			: CenterPoint(InCenterPoint)
		{

		}

		FORCEINLINE bool operator()(const FVector2D& A, const FVector2D& B) const
		{
			if (A.Y - CenterPoint.X >= 0 && B.X - CenterPoint.X < 0)
				return true;
			if (A.X - CenterPoint.X < 0 && B.X - CenterPoint.X >= 0)
				return false;
			if (A.X - CenterPoint.X == 0 && B.X - CenterPoint.X == 0) {
				if (A.Y - CenterPoint.Y >= 0 || B.Y - CenterPoint.Y >= 0)
					return A.Y > B.Y;
				return B.Y > A.Y;
			}

			int det = (A.X - CenterPoint.X) * (B.Y - CenterPoint.Y) - (B.X - CenterPoint.X) * (A.Y - CenterPoint.Y);
			if (det < 0)
				return true;
			if (det > 0)
				return false;

			int d1 = (A.X - CenterPoint.X) * (A.X - CenterPoint.X) + (A.Y - CenterPoint.Y) * (A.Y - CenterPoint.Y);
			int d2 = (B.X - CenterPoint.X) * (B.X - CenterPoint.X) + (B.Y - CenterPoint.Y) * (B.Y - CenterPoint.Y);
			return d1 > d2;
		}
	};

	IntersectionPoints.Sort(FPointSortCompare(PtCenter));

	FCanvasUVTri Tri;
	Tri.V0_Color = UVColor;
	Tri.V1_Color = UVColor;
	Tri.V2_Color = UVColor;

	OutTris.Reserve(IntersectionPoints.Num() - 2);

	for (int i = 1; i < IntersectionPoints.Num() - 1; i++)
	{
		Tri.V0_Pos = IntersectionPoints[0];
		Tri.V1_Pos = IntersectionPoints[i];
		Tri.V2_Pos = IntersectionPoints[i + 1];

		OutTris.Add(Tri);
	}

	return true;
}

void FBPVRReplicatedTextureStore::PackData()
{
	if (UnpackedData.Num() > 0)
	{
		TArray<uint8> TmpPacked;
		RLE_Funcs::RLEEncodeBuffer<uint16>(UnpackedData.GetData(), UnpackedData.Num(), &TmpPacked);
		UnpackedData.Reset();

if (TmpPacked.Num() > 512)
		{
			FArchiveSaveCompressedProxy Compressor(PackedData, NAME_Zlib, COMPRESS_BiasSpeed);
			Compressor << TmpPacked;
			Compressor.Flush();
			bIsZipped = true;

		}
		else
		{
			PackedData = TmpPacked;
			bIsZipped = false;

		}
	}
}

void FBPVRReplicatedTextureStore::UnPackData()
{
	if (PackedData.Num() > 0)
	{

if (bIsZipped)
		{
			TArray<uint8> RLEEncodedData;
			FArchiveLoadCompressedProxy DataArchive(PackedData, NAME_Zlib);
			DataArchive << RLEEncodedData;
			RLE_Funcs::RLEDecodeLine<uint16>(&RLEEncodedData, &UnpackedData, true);
		}
		else
		{
			RLE_Funcs::RLEDecodeLine<uint16>(&PackedData, &UnpackedData, true);
		}

		PackedData.Reset();
	}
}

bool FBPVRReplicatedTextureStore::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;

	Ar.SerializeBits(&bIsZipped, 1);
	Ar.SerializeIntPacked(Width);
	Ar.SerializeIntPacked(Height);
	Ar.SerializeBits(&PixelFormat, 8);

	Ar << PackedData;

	return bOutSuccess;
}

template <typename DataType>
void RLE_Funcs::RLEDecodeLine(TArray<uint8>* LineToDecode, TArray<DataType>* DecodedLine, bool bCompressed)
{
	if (!LineToDecode || !DecodedLine)
		return;

	RLEDecodeLine(LineToDecode->GetData(), LineToDecode->Num(), DecodedLine, bCompressed);
}

template <typename DataType>
void RLE_Funcs::RLEDecodeLine(const uint8* LineToDecode, uint32 Num, TArray<DataType>* DecodedLine, bool bCompressed)
{
	if (!bCompressed)
	{
		DecodedLine->Empty(Num / sizeof(DataType));
		DecodedLine->AddUninitialized(Num / sizeof(DataType));
		FMemory::Memcpy(DecodedLine->GetData(), LineToDecode, Num);
		return;
	}

	const uint8* StartLoc = LineToDecode;
	const uint8* EndLoc = StartLoc + Num;
	uint8 incr = sizeof(DataType);

	DataType ValToWrite = *((DataType*)LineToDecode); 

	DecodedLine->Empty();

	uint8 RLE_FLAG;
	uint32 Length32;
	uint32 Length8;
	uint32 Length16;
	int origLoc;

	for (const uint8* loc = StartLoc; loc < EndLoc;)
	{
		RLE_FLAG = *loc >> 4; 

		switch (RLE_FLAG)
		{
		case RLE_Flags::RLE_CompressedByte:
		{
			Length8 = (*loc & ~0xF0) + 1;
			loc++;
			ValToWrite = *((DataType*)loc);
			loc += incr;

			origLoc = DecodedLine->AddUninitialized(Length8);

			for (uint32 i = origLoc; i < origLoc + Length8; i++)
			{
				(*DecodedLine)[i] = ValToWrite;
			}

		}break;
		case RLE_Flags::RLE_CompressedShort:
		{
			Length16 = (((uint16)(*loc & ~0xF0)) << 8 | (*(loc + 1))) + 1;
			loc += 2;
			ValToWrite = *((DataType*)loc);
			loc += incr;

			origLoc = DecodedLine->AddUninitialized(Length16);

			for (uint32 i = origLoc; i < origLoc + Length16; i++)
			{
				(*DecodedLine)[i] = ValToWrite;
			}

		}break;
		case RLE_Flags::RLE_Compressed24:
		{
			Length32 = (((uint32)(*loc & ~0xF0)) << 16 | ((uint32)(*(loc + 1))) << 8 | (uint32)(*(loc + 2))) + 1;
			loc += 3;
			ValToWrite = *((DataType*)loc);
			loc += incr;

			origLoc = DecodedLine->AddUninitialized(Length32);

			for (uint32 i = origLoc; i < origLoc + Length32; i++)
			{
				(*DecodedLine)[i] = ValToWrite;
			}

		}break;

		case RLE_Flags::RLE_NotCompressedByte:
		{
			Length8 = (*loc & ~0xF0) + 1;
			loc++;

			origLoc = DecodedLine->AddUninitialized(Length8);

			for (uint32 i = origLoc; i < origLoc + Length8; i++)
			{
				(*DecodedLine)[i] = *((DataType*)loc);
				loc += incr;
			}

		}break;
		case RLE_Flags::RLE_NotCompressedShort:
		{
			Length16 = (((uint16)(*loc & ~0xF0)) << 8 | (*(loc + 1))) + 1;
			loc += 2;

			origLoc = DecodedLine->AddUninitialized(Length16);

			for (uint32 i = origLoc; i < origLoc + Length16; i++)
			{
				(*DecodedLine)[i] = *((DataType*)loc);
				loc += incr;
			}

		}break;
		case RLE_Flags::RLE_NotCompressed24:
		{
			Length32 = (((uint32)(*loc & ~0xF0)) << 16 | ((uint32)(*(loc + 1))) << 8 | ((uint32)(*(loc + 2)))) + 1;
			loc += 3;

			origLoc = DecodedLine->AddUninitialized(Length32);

			for (uint32 i = origLoc; i < origLoc + Length32; i++)
			{
				(*DecodedLine)[i] = *((DataType*)loc);
				loc += incr;
			}

		}break;

		case RLE_Flags::RLE_ContinueRunByte:
		{
			Length8 = (*loc & ~0xF0) + 1;
			loc++;

			origLoc = DecodedLine->AddUninitialized(Length8);

			for (uint32 i = origLoc; i < origLoc + Length8; i++)
			{
				(*DecodedLine)[i] = ValToWrite;
			}

		}break;
		case RLE_Flags::RLE_ContinueRunShort:
		{
			Length16 = (((uint16)(*loc & ~0xF0)) << 8 | (*(loc + 1))) + 1;
			loc += 2;

			origLoc = DecodedLine->AddUninitialized(Length16);

			for (uint32 i = origLoc; i < origLoc + Length16; i++)
			{
				(*DecodedLine)[i] = ValToWrite;
			}

		}break;
		case RLE_Flags::RLE_ContinueRun24:
		{
			Length32 = (((uint32)(*loc & ~0xF0)) << 16 | ((uint32)(*(loc + 1))) << 8 | (*(loc + 2))) + 1;
			loc += 3;

			origLoc = DecodedLine->AddUninitialized(Length32);

			for (uint32 i = origLoc; i < origLoc + Length32; i++)
			{
				(*DecodedLine)[i] = ValToWrite;
			}

		}break;

		}
	}
}

template <typename DataType>
bool RLE_Funcs::RLEEncodeLine(TArray<DataType>* LineToEncode, TArray<uint8>* EncodedLine)
{
	return RLEEncodeBuffer<DataType>(LineToEncode->GetData(), LineToEncode->Num(), EncodedLine);
}

void RLE_Funcs::RLEWriteContinueFlag(uint32 count, uint8** loc)
{
	if (count <= 16)
	{
		**loc = (((uint8)RLE_Flags::RLE_ContinueRunByte << 4) | ((uint8)count - 1));
		(*loc)++;
	}
	else if (count <= 4096)
	{
		uint16 val = ((((uint16)RLE_Flags::RLE_ContinueRunShort) << 12) | ((uint16)count - 1));
		**loc = val >> 8;
		(*loc)++;
		**loc = (uint8)val;
		(*loc)++;
	}
	else
	{
		uint32 val = ((((uint32)RLE_Flags::RLE_ContinueRun24) << 20) | ((uint32)count - 1));
		**loc = (uint8)(val >> 16);
		(*loc)++;
		**loc = (uint8)(val >> 8);
		(*loc)++;
		**loc = (uint8)val;
		(*loc)++;
	}
}

template <typename DataType>
void RLE_Funcs::RLEWriteRunFlag(uint32 count, uint8** loc, TArray<DataType>& Data, bool bCompressed)
{

	if (count <= 16)
	{
		uint8 val;
		if (bCompressed)
			val = ((((uint8)RLE_Flags::RLE_CompressedByte) << 4) | ((uint8)count - 1));
		else
			val = ((((uint8)RLE_Flags::RLE_NotCompressedByte) << 4) | ((uint8)count - 1));

		**loc = val;
		(*loc)++;
	}
	else if (count <= 4096)
	{
		uint16 val;
		if (bCompressed)
			val = ((((uint16)RLE_Flags::RLE_CompressedShort) << 12) | ((uint16)count - 1));
		else
			val = ((((uint16)RLE_Flags::RLE_NotCompressedShort) << 12) | ((uint16)count - 1));

		**loc = (uint8)(val >> 8);
		(*loc)++;
		**loc = (uint8)val;
		(*loc)++;
	}
	else
	{
		uint32 val;
		if (bCompressed)
			val = ((((uint32)RLE_Flags::RLE_Compressed24) << 20) | ((uint32)count - 1));
		else
			val = ((((uint32)RLE_Flags::RLE_NotCompressed24) << 20) | ((uint32)count - 1));

		**loc = (uint8)(val >> 16);
		(*loc)++;
		**loc = (uint8)(val >> 8);
		(*loc)++;
		**loc = (uint8)(val);
		(*loc)++;
	}

	FMemory::Memcpy(*loc, Data.GetData(), Data.Num() * sizeof(DataType));
	*loc += Data.Num() * sizeof(DataType);
	Data.Empty(256);
}

template <typename DataType>
bool RLE_Funcs::RLEEncodeBuffer(DataType* BufferToEncode, uint32 EncodeLength, TArray<uint8>* EncodedLine)
{
	uint32 OrigNum = EncodeLength;
	uint8 incr = sizeof(DataType);
	uint32 MAX_COUNT = 1048576; 

	EncodedLine->Empty((OrigNum * sizeof(DataType)) + (OrigNum * (sizeof(short))));

	EncodedLine->AddUninitialized((OrigNum * sizeof(DataType)) + ((OrigNum / MAX_COUNT * 3)));

	DataType* First = BufferToEncode;
	DataType Last;
	uint32 RunCount = 0;

	uint8* loc = EncodedLine->GetData();

	bool bInRun = false;
	bool bWroteStart = false;
	bool bContinueRun = false;

	TArray<DataType> TempBuffer;
	TempBuffer.Reserve(256);
	uint32 TempCount = 0;

	Last = *First;
	First++;

	for (uint32 i = 0; i < OrigNum - 1; i++, First++)
	{
		if (Last == *First)
		{
			if (bWroteStart && !bInRun)
			{
				RLE_Funcs::RLEWriteRunFlag(TempCount, &loc, TempBuffer, false);
				bWroteStart = false;
			}

			if (bInRun && TempCount < MAX_COUNT)
			{
				TempCount++;

				if (TempCount == MAX_COUNT)
				{

					if (bContinueRun)
					{
						RLE_Funcs::RLEWriteContinueFlag(TempCount, &loc);
					}
					else
						RLE_Funcs::RLEWriteRunFlag(TempCount, &loc, TempBuffer, true);

					bContinueRun = true;
					TempCount = 0;
				}
			}
			else
			{
				bInRun = true;
				bWroteStart = false;
				bContinueRun = false;

				TempBuffer.Add(Last);
				TempCount = 1;
			}

		}
		else if (bInRun)
		{
			bInRun = false;

			if (bContinueRun)
			{
				TempCount++;
				RLE_Funcs::RLEWriteContinueFlag(TempCount, &loc);
			}
			else
			{
				TempCount++;
				RLE_Funcs::RLEWriteRunFlag(TempCount, &loc, TempBuffer, true);
			}

			bContinueRun = false;
		}
		else
		{
			if (bWroteStart && TempCount < MAX_COUNT)
			{
				TempCount++;
				TempBuffer.Add(Last);
			}
			else if (bWroteStart && TempCount == MAX_COUNT)
			{
				RLE_Funcs::RLEWriteRunFlag(TempCount, &loc, TempBuffer, false);

				bWroteStart = true;
				TempBuffer.Add(Last);
				TempCount = 1;
			}
			else
			{
				TempBuffer.Add(Last);
				TempCount = 1;

				bWroteStart = true;
			}
		}

		Last = *First;
	}

	if (bInRun)
	{
		if (TempCount <= MAX_COUNT)
		{
			if (TempCount == MAX_COUNT)
			{

				RLE_Funcs::RLEWriteRunFlag(TempCount, &loc, TempBuffer, true);
				bContinueRun = true;
			}

			if (bContinueRun)
			{
				TempCount++;
				RLE_Funcs::RLEWriteContinueFlag(TempCount, &loc);
			}
			else
			{
				TempCount++;
				RLE_Funcs::RLEWriteRunFlag(TempCount, &loc, TempBuffer, true);
			}
		}

	}
	else
	{
		if (bWroteStart && TempCount <= MAX_COUNT)
		{
			if (TempCount == MAX_COUNT)
			{

				RLE_Funcs::RLEWriteRunFlag(TempCount, &loc, TempBuffer, false);
				TempCount = 0;
			}

			TempCount++;
			TempBuffer.Add(Last);
			RLE_Funcs::RLEWriteRunFlag(TempCount, &loc, TempBuffer, false);
		}
	}

	uint32 Wrote = loc - EncodedLine->GetData();
	EncodedLine->RemoveAt(Wrote, EncodedLine->Num() - Wrote, EAllowShrinking::Yes);

	return true;

}

template<int32 ScaleFactor, int32 MaxBitsPerComponent>
bool WritePackedVector2D(FVector2D Value, FArchive& Ar)	
{
	check(Ar.IsSaving());

	Value *= ScaleFactor;

	if (Value.ContainsNaN())
	{
		logOrEnsureNanError(TEXT("WritePackedVector2D: Value contains NaN, clearing for safety."));
		FVector2D	Dummy(0, 0);
		WritePackedVector2D<ScaleFactor, MaxBitsPerComponent>(Dummy, Ar);
		return false;
	}

	float MinV = -1073741824.0f;
	float MaxV = 1073741760.0f;

	const FVector2D ClampedValue = FVector2D(FMath::Clamp(Value.X, MinV, MaxV), FMath::Clamp(Value.Y, MinV, MaxV));
	bool bClamp = ClampedValue != Value;

	int32 IntX = FMath::RoundToInt(ClampedValue.X);
	int32 IntY = FMath::RoundToInt(ClampedValue.Y);

	uint32 Bits = FMath::Clamp<uint32>(FMath::CeilLogTwo(1 + FMath::Max(FMath::Abs(IntX), FMath::Abs(IntY))), 1, MaxBitsPerComponent) - 1;

	Ar.SerializeInt(Bits, MaxBitsPerComponent);

	int32  Bias = 1 << (Bits + 1);
	uint32 Max = 1 << (Bits + 2);
	uint32 DX = IntX + Bias;
	uint32 DY = IntY + Bias;

	if (DX >= Max) { bClamp = true; DX = static_cast<int32>(DX) > 0 ? Max - 1 : 0; }
	if (DY >= Max) { bClamp = true; DY = static_cast<int32>(DY) > 0 ? Max - 1 : 0; }

	Ar.SerializeInt(DX, Max);
	Ar.SerializeInt(DY, Max);

	return !bClamp;
}

template<uint32 ScaleFactor, int32 MaxBitsPerComponent>
bool ReadPackedVector2D(FVector2D& Value, FArchive& Ar)
{
	uint32 Bits = 0;

	Ar.SerializeInt(Bits, MaxBitsPerComponent);

	int32  Bias = 1 << (Bits + 1);
	uint32 Max = 1 << (Bits + 2);
	uint32 DX = 0;
	uint32 DY = 0;

	Ar.SerializeInt(DX, Max);
	Ar.SerializeInt(DY, Max);

	float fact = (float)ScaleFactor;

	Value.X = (float)(static_cast<int32>(DX) - Bias) / fact;
	Value.Y = (float)(static_cast<int32>(DY) - Bias) / fact;

	return true;
}

bool FRenderManagerOperation::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;

	Ar.SerializeIntPacked(OwnerID);
	Ar.SerializeBits(&OperationType, 3);

	switch (OperationType)
	{
	case ERenderManagerOperationType::Op_LineDraw:
	{
		Ar << Color;
		Ar.SerializeIntPacked(Thickness);

		if (Ar.IsSaving())
		{
			bOutSuccess &= WritePackedVector2D<1, 20>(P1, Ar);
			bOutSuccess &= WritePackedVector2D<1, 20>(P2, Ar);
		}
		else
		{
			ReadPackedVector2D<1, 20>(P1, Ar);
			ReadPackedVector2D<1, 20>(P2, Ar);
		}
	}break;
	case ERenderManagerOperationType::Op_TexDraw:
	{
		Ar << Texture;

		if (Ar.IsSaving())
		{
			bOutSuccess &= WritePackedVector2D<1, 20>(P1, Ar);
		}
		else
		{
			ReadPackedVector2D<1, 20>(P1, Ar);
		}
	}break;
	case ERenderManagerOperationType::Op_TriDraw:
	{
		Ar << Color;
		Ar << Material;

		uint32 ArrayCt = Tris.Num();
		Ar.SerializeIntPacked(ArrayCt);

		if (Ar.IsLoading())
		{
			Tris.Reset(ArrayCt);
			Tris.AddUninitialized(ArrayCt);

			FRenderManagerTri TriTemp;
			for (uint32 i = 0; i < ArrayCt; ++i)
			{
				ReadPackedVector2D<1, 20>(TriTemp.P1, Ar);
				ReadPackedVector2D<1, 20>(TriTemp.P2, Ar);
				ReadPackedVector2D<1, 20>(TriTemp.P3, Ar);
				Tris[i] = TriTemp;
			}
		}
		else
		{
			for (uint32 i = 0; i < ArrayCt; ++i)
			{
				WritePackedVector2D<1, 20>(Tris[i].P1, Ar);
				WritePackedVector2D<1, 20>(Tris[i].P2, Ar);
				WritePackedVector2D<1, 20>(Tris[i].P3, Ar);
			}
		}

	}break;
	}

	return bOutSuccess;
}

namespace UE::Net
{
	struct alignas(8) FQuantizedRenderTargetTextureStoreData
	{
		uint32 Width;
		uint32 Height;
		uint32 bIsZipped;

		typedef FNetSerializerArrayStorage<uint8> DataStorage;
		DataStorage Data;
	};
}

template <> struct TIsPODType<UE::Net::FQuantizedRenderTargetTextureStoreData> { enum { Value = true }; };

namespace UE::Net
{

	struct FBPVRReplicatedTextureStoreNetSerializer
	{

		static constexpr uint32 Version = 0;

		static constexpr bool bUseDefaultDelta = true;

		static constexpr bool bHasDynamicState = true;

		class FNetSerializerRegistryDelegates final : private UE::Net::FNetSerializerRegistryDelegates
		{
		public:
			virtual ~FNetSerializerRegistryDelegates();

		private:
			virtual void OnPreFreezeNetSerializerRegistry() override;

		};

		inline static FBPVRReplicatedTextureStoreNetSerializer::FNetSerializerRegistryDelegates NetSerializerRegistryDelegates;

		typedef FBPVRReplicatedTextureStore SourceType;
		typedef FQuantizedRenderTargetTextureStoreData QuantizedType;
		typedef FBPVRReplicatedTextureStoreSerializerConfig ConfigType;
		inline static const ConfigType DefaultConfig;

		static void Quantize(FNetSerializationContext& Context, const FNetQuantizeArgs& Args)
		{

			const SourceType& Source = *reinterpret_cast<const SourceType*>(Args.Source);
			QuantizedType& Target = *reinterpret_cast<QuantizedType*>(Args.Target);

			Target.bIsZipped = Source.bIsZipped ? 1 : 0;
			Target.Width = Source.Width;
			Target.Height = Source.Height;

			Target.Data.AdjustSize(Context, Source.PackedData.Num());

			if (Target.Data.Num() > 0)
			{
				FMemory::Memcpy(Target.Data.GetData(), Source.PackedData.GetData(), Target.Data.Num());
			}
		}

		static void Dequantize(FNetSerializationContext& Context, const FNetDequantizeArgs& Args)
		{
			const QuantizedType& Source = *reinterpret_cast<const QuantizedType*>(Args.Source);
			SourceType& Target = *reinterpret_cast<SourceType*>(Args.Target);

			Target.bIsZipped = Source.bIsZipped != 0;
			Target.Width = Source.Width;
			Target.Height = Source.Height;

			Target.PackedData.Reset(Source.Data.Num());

			if (Source.Data.Num() > 0)
			{
				Target.PackedData.AddUninitialized(Source.Data.Num());
				FMemory::Memcpy(Target.PackedData.GetData(), Source.Data.GetData(), Source.Data.Num());
			}
		}

		static void Serialize(FNetSerializationContext& Context, const FNetSerializeArgs& Args)
		{
			const QuantizedType& Source = *reinterpret_cast<const QuantizedType*>(Args.Source);
			FNetBitStreamWriter* Writer = Context.GetBitStreamWriter();

			Writer->WriteBits(Source.bIsZipped, 1);
			Writer->WriteBits(Source.Width, 32);
			Writer->WriteBits(Source.Height, 32);

			uint32 Size = Source.Data.Num();
			Writer->WriteBits(Size, 32);

			if (Size > 0)
			{
				Writer->WriteBitStream((uint32*)Source.Data.GetData(), 0, Source.Data.Num() * 8);
			}
		}

		static void Deserialize(FNetSerializationContext& Context, const FNetDeserializeArgs& Args)
		{
			QuantizedType& Target = *reinterpret_cast<QuantizedType*>(Args.Target);
			FNetBitStreamReader* Reader = Context.GetBitStreamReader();

			Target.bIsZipped = Reader->ReadBits(1);
			Target.Width = Reader->ReadBits(32);
			Target.Height = Reader->ReadBits(32);

			uint32 Size = Reader->ReadBits(32);
			Target.Data.AdjustSize(Context, Size);

			if (Size > 0)
			{
				Reader->ReadBitStream((uint32*)Target.Data.GetData(), Target.Data.Num() * 8);
			}
		}

		static bool IsEqual(FNetSerializationContext& Context, const FNetIsEqualArgs& Args)
		{

			return false;
		}

		static void CloneDynamicState(FNetSerializationContext& Context, const FNetCloneDynamicStateArgs& Args)
		{
			const QuantizedType* Source = reinterpret_cast<const QuantizedType*>(Args.Source);
			QuantizedType* Target = reinterpret_cast<QuantizedType*>(Args.Target);

			Target->bIsZipped = Source->bIsZipped;
			Target->Width = Source->Width;
			Target->Height = Source->Height;

			Target->Data.Clone(Context, Source->Data);
		}

		static void FreeDynamicState(FNetSerializationContext& Context, const FNetFreeDynamicStateArgs& Args)
		{
			QuantizedType& Target = *reinterpret_cast<QuantizedType*>(Args.Source);
			Target.Data.Free(Context);
		}
	};
}

namespace UE::Net
{

	struct alignas(8)  FQuantizedTriData
	{
		uint16 P1[2];
		uint16 P2[2];
		uint16 P3[2];
	};

	struct FQuantizedSoftObjectRef
	{
		alignas(8) uint8 Storage[64]; 

	};

	struct alignas(8)  FQuantizedRenderManagerOperationData
	{
		uint32 OwnerID;
		uint8 OperationType;
		uint32 Color;
		uint16 P1[2];
		uint16 P2[2];
		uint32 Thickness;

		typedef FNetSerializerArrayStorage<FQuantizedTriData> TriStorage;
		TriStorage Tris;

		FQuantizedSoftObjectRef Texture;
		FQuantizedSoftObjectRef Material;
	};
}

template <> struct TIsPODType<UE::Net::FQuantizedRenderManagerOperationData> { enum { Value = true }; };

namespace UE::Net
{

	struct FRenderManagerOperationNetSerializer
	{
		inline static const FSoftObjectNetSerializerConfig ObjectPtrNetSerializerConfig;

		inline static const FNetSerializerConfig* FObjectPtrSerializerConfigPtr = &ObjectPtrNetSerializerConfig;
		inline static const FNetSerializer* FObjectPtrNetSerializerPtr;

		static constexpr uint32 Version = 0;

		static constexpr bool bUseDefaultDelta = true;

		static constexpr bool bHasDynamicState = true;

		class FNetSerializerRegistryDelegates final : private UE::Net::FNetSerializerRegistryDelegates
		{
		public:
			virtual ~FNetSerializerRegistryDelegates();

			void InitNetSerializer()
			{
				FRenderManagerOperationNetSerializer::FObjectPtrNetSerializerPtr = &UE_NET_GET_SERIALIZER(FSoftObjectNetSerializer);
			}

		private:
			virtual void OnPreFreezeNetSerializerRegistry() override;

		};

		inline static FRenderManagerOperationNetSerializer::FNetSerializerRegistryDelegates NetSerializerRegistryDelegates;

		typedef FRenderManagerOperation SourceType;
		typedef FQuantizedRenderManagerOperationData QuantizedType;
		typedef FRenderManagerOperationSerializerConfig ConfigType;
		inline static const ConfigType DefaultConfig;

		static void Quantize(FNetSerializationContext& Context, const FNetQuantizeArgs& Args)
		{

			const SourceType& Source = *reinterpret_cast<const SourceType*>(Args.Source);
			QuantizedType& Target = *reinterpret_cast<QuantizedType*>(Args.Target);

			Target.OwnerID = Source.OwnerID;
			Target.OperationType = (uint8)Source.OperationType;

			switch ((ERenderManagerOperationType)Source.OperationType)
			{
			case ERenderManagerOperationType::Op_LineDraw:
			{

				uint8* ColorPtr = ((uint8*)&Target.Color);
				ColorPtr[0] = Source.Color.R;
				ColorPtr[1] = Source.Color.G;
				ColorPtr[2] = Source.Color.B;
				ColorPtr[3] = Source.Color.A;

				Target.Thickness = Source.Thickness;

				Target.P1[0] = (uint16)GetCompressedFloat<10000, 16>(Source.P1.X);
				Target.P1[1] = (uint16)GetCompressedFloat<10000, 16>(Source.P1.Y);
				Target.P2[0] = (uint16)GetCompressedFloat<10000, 16>(Source.P2.X);
				Target.P2[1] = (uint16)GetCompressedFloat<10000, 16>(Source.P2.Y);

			}break;

			case ERenderManagerOperationType::Op_TexDraw:
			{

				const FNetSerializer* ObjSerializer = FObjectPtrNetSerializerPtr;
				const FNetSerializerConfig* ObjSerializerConfig = FObjectPtrSerializerConfigPtr;

				FNetQuantizeArgs MemberArgsObj = Args;
				MemberArgsObj.NetSerializerConfig = NetSerializerConfigParam(ObjSerializerConfig);
				MemberArgsObj.Source = NetSerializerValuePointer(&Source.Texture);
				MemberArgsObj.Target = NetSerializerValuePointer(&Target.Texture);
				ObjSerializer->Quantize(Context, MemberArgsObj);

				Target.P1[0] = (uint16)GetCompressedFloat<10000, 16>(Source.P1.X);
				Target.P1[1] = (uint16)GetCompressedFloat<10000, 16>(Source.P1.Y);

			}break;

			case ERenderManagerOperationType::Op_TriDraw:
			{

				uint8* ColorPtr = ((uint8*)&Target.Color);
				ColorPtr[0] = Source.Color.R;
				ColorPtr[1] = Source.Color.G;
				ColorPtr[2] = Source.Color.B;
				ColorPtr[3] = Source.Color.A;

				const FNetSerializer* ObjSerializer = FObjectPtrNetSerializerPtr;
				const FNetSerializerConfig* ObjSerializerConfig = FObjectPtrSerializerConfigPtr;

				FNetQuantizeArgs MemberArgsObj = Args;
				MemberArgsObj.NetSerializerConfig = NetSerializerConfigParam(ObjSerializerConfig);
				MemberArgsObj.Source = NetSerializerValuePointer(&Source.Material);
				MemberArgsObj.Target = NetSerializerValuePointer(&Target.Material);
				ObjSerializer->Quantize(Context, MemberArgsObj);

				Target.Tris.AdjustSize(Context, Source.Tris.Num());
				FQuantizedTriData* TargetTriData = Target.Tris.GetData();

				if (Target.Tris.Num() > 0)
				{
					for (uint32 i = 0; i < Target.Tris.Num(); ++i)
					{
						TargetTriData[i].P1[0] = (uint16)GetCompressedFloat<10000, 16>(Source.Tris[i].P1.X);
						TargetTriData[i].P1[1] = (uint16)GetCompressedFloat<10000, 16>(Source.Tris[i].P1.Y);
						TargetTriData[i].P2[0] = (uint16)GetCompressedFloat<10000, 16>(Source.Tris[i].P2.X);
						TargetTriData[i].P2[1] = (uint16)GetCompressedFloat<10000, 16>(Source.Tris[i].P2.Y);
						TargetTriData[i].P3[0] = (uint16)GetCompressedFloat<10000, 16>(Source.Tris[i].P3.X);
						TargetTriData[i].P3[1] = (uint16)GetCompressedFloat<10000, 16>(Source.Tris[i].P3.Y);
					}
				}

			}break;
			}
		}

		static void Dequantize(FNetSerializationContext& Context, const FNetDequantizeArgs& Args)
		{
			const QuantizedType& Source = *reinterpret_cast<const QuantizedType*>(Args.Source);
			SourceType& Target = *reinterpret_cast<SourceType*>(Args.Target);

			Target.OwnerID = Source.OwnerID;
			Target.OperationType = (ERenderManagerOperationType)Source.OperationType;

			switch ((ERenderManagerOperationType)Source.OperationType)
			{
			case ERenderManagerOperationType::Op_LineDraw:
			{

				uint8* ColorPtr = ((uint8*)&Source.Color);
				Target.Color.R = ColorPtr[0];
				Target.Color.G = ColorPtr[1];
				Target.Color.B = ColorPtr[2];
				Target.Color.A = ColorPtr[3];

				Target.Thickness = Source.Thickness;

				Target.P1.X = GetDecompressedFloat<10000, 16>(Source.P1[0]);
				Target.P1.Y = GetDecompressedFloat<10000, 16>(Source.P1[1]);

				Target.P2.X = GetDecompressedFloat<10000, 16>(Source.P2[0]);
				Target.P2.Y = GetDecompressedFloat<10000, 16>(Source.P2[1]);

			}break;

			case ERenderManagerOperationType::Op_TexDraw:
			{

				const FNetSerializer* ObjSerializer = FObjectPtrNetSerializerPtr;
				const FNetSerializerConfig* ObjSerializerConfig = FObjectPtrSerializerConfigPtr;

				FNetDequantizeArgs MemberArgsObj = Args;
				MemberArgsObj.NetSerializerConfig = NetSerializerConfigParam(ObjSerializerConfig);
				MemberArgsObj.Source = NetSerializerValuePointer(&Source.Texture);
				MemberArgsObj.Target = NetSerializerValuePointer(&Target.Texture);
				ObjSerializer->Dequantize(Context, MemberArgsObj);

				Target.P1.X = GetDecompressedFloat<10000, 16>(Source.P1[0]);
				Target.P1.Y = GetDecompressedFloat<10000, 16>(Source.P1[1]);

			}break;

			case ERenderManagerOperationType::Op_TriDraw:
			{

				uint8* ColorPtr = ((uint8*)&Source.Color);
				Target.Color.R = ColorPtr[0];
				Target.Color.G = ColorPtr[1];
				Target.Color.B = ColorPtr[2];
				Target.Color.A = ColorPtr[3];

				const FNetSerializer* ObjSerializer = FObjectPtrNetSerializerPtr;
				const FNetSerializerConfig* ObjSerializerConfig = FObjectPtrSerializerConfigPtr;

				FNetDequantizeArgs MemberArgsObj = Args;
				MemberArgsObj.NetSerializerConfig = NetSerializerConfigParam(ObjSerializerConfig);
				MemberArgsObj.Source = NetSerializerValuePointer(&Source.Material);
				MemberArgsObj.Target = NetSerializerValuePointer(&Target.Material);
				ObjSerializer->Dequantize(Context, MemberArgsObj);

				Target.Tris.Reset(Source.Tris.Num());
				Target.Tris.AddUninitialized(Source.Tris.Num());

				const FQuantizedTriData* SourceTriData = Source.Tris.GetData();

				if (Target.Tris.Num() > 0)
				{
					for (int32 i = 0; i < Target.Tris.Num(); ++i)
					{
						Target.Tris[i].P1.X = GetDecompressedFloat<10000, 16>(SourceTriData[i].P1[0]);
						Target.Tris[i].P1.Y = GetDecompressedFloat<10000, 16>(SourceTriData[i].P1[1]);
						Target.Tris[i].P2.X = GetDecompressedFloat<10000, 16>(SourceTriData[i].P2[0]);
						Target.Tris[i].P2.Y = GetDecompressedFloat<10000, 16>(SourceTriData[i].P2[1]);
						Target.Tris[i].P3.X = GetDecompressedFloat<10000, 16>(SourceTriData[i].P3[0]);
						Target.Tris[i].P3.Y = GetDecompressedFloat<10000, 16>(SourceTriData[i].P3[1]);
					}
				}

			}break;
			}
		}

		static void Serialize(FNetSerializationContext& Context, const FNetSerializeArgs& Args)
		{
			const QuantizedType& Source = *reinterpret_cast<const QuantizedType*>(Args.Source);
			FNetBitStreamWriter* Writer = Context.GetBitStreamWriter();

			Writer->WriteBits(Source.OwnerID, 32);
			Writer->WriteBits(Source.OperationType, 8);

			switch ((ERenderManagerOperationType)Source.OperationType)
			{
			case ERenderManagerOperationType::Op_LineDraw:
			{

				Writer->WriteBits(Source.Color, 32);

				Writer->WriteBits(Source.Thickness, 32);

				if (Writer->WriteBool(Source.P1[0] != 0)) {Writer->WriteBits(Source.P1[0], 16);}
				if (Writer->WriteBool(Source.P1[1] != 0)) { Writer->WriteBits(Source.P1[1], 16);}
				if (Writer->WriteBool(Source.P2[0] != 0)) { Writer->WriteBits(Source.P2[0], 16);}
				if (Writer->WriteBool(Source.P2[1] != 0)) { Writer->WriteBits(Source.P2[1], 16);}

			}break;

			case ERenderManagerOperationType::Op_TexDraw:
			{

				const FNetSerializer* ObjSerializer = FObjectPtrNetSerializerPtr;
				const FNetSerializerConfig* ObjSerializerConfig = FObjectPtrSerializerConfigPtr;

				FNetSerializeArgs MemberArgsObj = Args;
				MemberArgsObj.NetSerializerConfig = NetSerializerConfigParam(ObjSerializerConfig);
				MemberArgsObj.Source = NetSerializerValuePointer(&Source.Texture);
				ObjSerializer->Serialize(Context, MemberArgsObj);

				if (Writer->WriteBool(Source.P1[0] != 0)) { Writer->WriteBits(Source.P1[0], 16); }
				if (Writer->WriteBool(Source.P1[1] != 0)) { Writer->WriteBits(Source.P1[1], 16); }

			}break;

			case ERenderManagerOperationType::Op_TriDraw:
			{

				Writer->WriteBits(Source.Color, 32);

				const FNetSerializer* ObjSerializer = FObjectPtrNetSerializerPtr;
				const FNetSerializerConfig* ObjSerializerConfig = FObjectPtrSerializerConfigPtr;

				FNetSerializeArgs MemberArgsObj = Args;
				MemberArgsObj.NetSerializerConfig = NetSerializerConfigParam(ObjSerializerConfig);
				MemberArgsObj.Source = NetSerializerValuePointer(&Source.Material);
				ObjSerializer->Serialize(Context, MemberArgsObj);

				const FQuantizedTriData* TargetTriData = Source.Tris.GetData();

				uint32 TriSize = Source.Tris.Num();
				Writer->WriteBits(TriSize, 32);

				if (Source.Tris.Num() > 0)
				{
					for (uint32 i = 0; i < Source.Tris.Num(); ++i)
					{
						Writer->WriteBits(TargetTriData[i].P1[0], 16);
						Writer->WriteBits(TargetTriData[i].P1[1], 16);
						Writer->WriteBits(TargetTriData[i].P2[0], 16);
						Writer->WriteBits(TargetTriData[i].P2[1], 16);
						Writer->WriteBits(TargetTriData[i].P3[0], 16);
						Writer->WriteBits(TargetTriData[i].P3[1], 16);
					}

				}

			}break;
			}
		}

		static void Deserialize(FNetSerializationContext& Context, const FNetDeserializeArgs& Args)
		{
			QuantizedType& Target = *reinterpret_cast<QuantizedType*>(Args.Target);
			FNetBitStreamReader* Reader = Context.GetBitStreamReader();

			Target.OwnerID = Reader->ReadBits(32);
			Target.OperationType = Reader->ReadBits(8);

			switch ((ERenderManagerOperationType)Target.OperationType)
			{
			case ERenderManagerOperationType::Op_LineDraw:
			{

				Target.Color = Reader->ReadBits(32);

				Target.Thickness = Reader->ReadBits(32);

				Target.P1[0] = Reader->ReadBool() ? Reader->ReadBits(16) : 0;
				Target.P1[1] = Reader->ReadBool() ? Reader->ReadBits(16) : 0;
				Target.P2[0] = Reader->ReadBool() ? Reader->ReadBits(16) : 0;
				Target.P2[1] = Reader->ReadBool() ? Reader->ReadBits(16) : 0;

			}break;

			case ERenderManagerOperationType::Op_TexDraw:
			{

				const FNetSerializer* ObjSerializer = FObjectPtrNetSerializerPtr;
				const FNetSerializerConfig* ObjSerializerConfig = FObjectPtrSerializerConfigPtr;

				FNetDeserializeArgs MemberArgsObj = Args;
				MemberArgsObj.NetSerializerConfig = NetSerializerConfigParam(ObjSerializerConfig);
				MemberArgsObj.Target = NetSerializerValuePointer(&Target.Texture);
				ObjSerializer->Deserialize(Context, MemberArgsObj);

				Target.P1[0] = Reader->ReadBool() ? Reader->ReadBits(16) : 0;
				Target.P1[1] = Reader->ReadBool() ? Reader->ReadBits(16) : 0;

			}break;

			case ERenderManagerOperationType::Op_TriDraw:
			{

				Target.Color = Reader->ReadBits(32);

				const FNetSerializer* ObjSerializer = FObjectPtrNetSerializerPtr;
				const FNetSerializerConfig* ObjSerializerConfig = FObjectPtrSerializerConfigPtr;

				FNetDeserializeArgs MemberArgsObj = Args;
				MemberArgsObj.NetSerializerConfig = NetSerializerConfigParam(ObjSerializerConfig);
				MemberArgsObj.Target = NetSerializerValuePointer(&Target.Material);
				ObjSerializer->Deserialize(Context, MemberArgsObj);

				uint32 TriSize = Reader->ReadBits(32);
				Target.Tris.AdjustSize(Context, TriSize);

				FQuantizedTriData* TargetTriData = Target.Tris.GetData();

				if (Target.Tris.Num() > 0)
				{
					for (uint32 i = 0; i < TriSize; ++i)
					{
						TargetTriData[i].P1[0] = Reader->ReadBits(16);
						TargetTriData[i].P1[1] = Reader->ReadBits(16);
						TargetTriData[i].P2[0] = Reader->ReadBits(16);
						TargetTriData[i].P2[1] = Reader->ReadBits(16);
						TargetTriData[i].P3[0] = Reader->ReadBits(16);
						TargetTriData[i].P3[1] = Reader->ReadBits(16);
					}

				}
			}break;
			}
		}

		static bool IsEqual(FNetSerializationContext& Context, const FNetIsEqualArgs& Args)
		{

			return false;
		}

		static void CloneDynamicState(FNetSerializationContext& Context, const FNetCloneDynamicStateArgs& Args)
		{
			const QuantizedType* Source = reinterpret_cast<const QuantizedType*>(Args.Source);
			QuantizedType* Target = reinterpret_cast<QuantizedType*>(Args.Target);

			Target->OwnerID = Source->OwnerID;
			Target->OperationType = Source->OperationType;

			switch ((ERenderManagerOperationType)Target->OperationType)
			{
			case ERenderManagerOperationType::Op_LineDraw:
			{

				Target->Color = Source->Color;

				Target->Thickness = Source->Thickness;

				Target->P1[0] = Source->P1[0];
				Target->P1[1] = Source->P1[1];
				Target->P2[0] = Source->P2[0];
				Target->P2[1] = Source->P2[1];

			}break;

			case ERenderManagerOperationType::Op_TexDraw:
			{

				const FNetSerializer* ObjSerializer = FObjectPtrNetSerializerPtr;
				const FNetSerializerConfig* ObjSerializerConfig = FObjectPtrSerializerConfigPtr;

				FNetCloneDynamicStateArgs ObjMemberArgs = Args;
				ObjMemberArgs.NetSerializerConfig = NetSerializerConfigParam(ObjSerializerConfig);
				ObjMemberArgs.Target = NetSerializerValuePointer(&Target->Texture);
				ObjMemberArgs.Source = NetSerializerValuePointer(&Source->Texture);
				ObjSerializer->CloneDynamicState(Context, ObjMemberArgs);

				Target->P1[0] = Source->P1[0];
				Target->P1[1] = Source->P1[1];

			}break;

			case ERenderManagerOperationType::Op_TriDraw:
			{

				Target->Color = Source->Color;

				const FNetSerializer* ObjSerializer = FObjectPtrNetSerializerPtr;
				const FNetSerializerConfig* ObjSerializerConfig = FObjectPtrSerializerConfigPtr;

				FNetCloneDynamicStateArgs ObjMemberArgs = Args;
				ObjMemberArgs.NetSerializerConfig = NetSerializerConfigParam(ObjSerializerConfig);
				ObjMemberArgs.Target = NetSerializerValuePointer(&Target->Material);
				ObjMemberArgs.Source = NetSerializerValuePointer(&Source->Material);
				ObjSerializer->CloneDynamicState(Context, ObjMemberArgs);

				Target->Tris.Clone(Context, Source->Tris);

			}break;
			}
		}

		static void FreeDynamicState(FNetSerializationContext& Context, const FNetFreeDynamicStateArgs& Args)
		{
			QuantizedType& Target = *reinterpret_cast<QuantizedType*>(Args.Source);
			Target.Tris.Free(Context);
		}
	};

	static const FName PropertyNetSerializerRegistry_NAME_BPVRReplicatedTextureStore("BPVRReplicatedTextureStore");
	UE_NET_IMPLEMENT_NAMED_STRUCT_NETSERIALIZER_INFO(PropertyNetSerializerRegistry_NAME_BPVRReplicatedTextureStore, FBPVRReplicatedTextureStoreNetSerializer);

	FBPVRReplicatedTextureStoreNetSerializer::FNetSerializerRegistryDelegates::~FNetSerializerRegistryDelegates()
	{
		UE_NET_UNREGISTER_NETSERIALIZER_INFO(PropertyNetSerializerRegistry_NAME_BPVRReplicatedTextureStore);
	}

	void FBPVRReplicatedTextureStoreNetSerializer::FNetSerializerRegistryDelegates::OnPreFreezeNetSerializerRegistry()
	{
		UE_NET_REGISTER_NETSERIALIZER_INFO(PropertyNetSerializerRegistry_NAME_BPVRReplicatedTextureStore);
	}

	UE_NET_IMPLEMENT_SERIALIZER(FBPVRReplicatedTextureStoreNetSerializer);

	static const FName PropertyNetSerializerRegistry_NAME_RenderManagerOperation("RenderManagerOperation");
	UE_NET_IMPLEMENT_NAMED_STRUCT_NETSERIALIZER_INFO(PropertyNetSerializerRegistry_NAME_RenderManagerOperation, FRenderManagerOperationNetSerializer);

	FRenderManagerOperationNetSerializer::FNetSerializerRegistryDelegates::~FNetSerializerRegistryDelegates()
	{
		UE_NET_UNREGISTER_NETSERIALIZER_INFO(PropertyNetSerializerRegistry_NAME_RenderManagerOperation);
	}

	void FRenderManagerOperationNetSerializer::FNetSerializerRegistryDelegates::OnPreFreezeNetSerializerRegistry()
	{
		InitNetSerializer();

		UE_NET_REGISTER_NETSERIALIZER_INFO(PropertyNetSerializerRegistry_NAME_RenderManagerOperation);
	}

	UE_NET_IMPLEMENT_SERIALIZER(FRenderManagerOperationNetSerializer);
}
