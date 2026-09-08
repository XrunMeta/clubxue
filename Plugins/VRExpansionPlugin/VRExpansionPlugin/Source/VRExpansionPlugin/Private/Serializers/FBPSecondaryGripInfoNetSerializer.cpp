#include "Serializers/FBPSecondaryGripInfoNetSerializer.h"
#include "Serializers/SerializerHelpers.h"
#include "Iris/Serialization/NetSerializerDelegates.h"
#include "Iris/Serialization/NetSerializers.h"
#include "Iris/ReplicationState/PropertyNetSerializerInfoRegistry.h"
#include "Iris/ReplicationState/ReplicationStateDescriptorBuilder.h"
#include "Serializers/FTransformNetQuantizeNetSerializer.h"

namespace UE::Net
{

    struct FBPSecondaryGripInfoNetSerializer
    {
		inline static const FVectorNetQuantize100NetSerializerConfig FTransformQuantizeSerializerConfig;
		inline static const FObjectPtrNetSerializerConfig ObjectPtrNetSerializerConfig;
		inline static const FNameNetSerializerConfig NameNetSerializerConfig;

		inline static const FNetSerializerConfig* FTransformQuantizeSerializerConfigPtr = &FTransformQuantizeSerializerConfig;
		inline static const FNetSerializer* FTransformQuantizeNetSerializerPtr;

		inline static const FNetSerializerConfig* FObjectPtrSerializerConfigPtr = &ObjectPtrNetSerializerConfig;
		inline static const FNetSerializer* FObjectPtrNetSerializerPtr;

		inline static const FNetSerializerConfig* FNameSerializerConfigPtr = &NameNetSerializerConfig;
		inline static const FNetSerializer* FNameNetSerializerPtr;

        class FNetSerializerRegistryDelegates final : private UE::Net::FNetSerializerRegistryDelegates
        {
        public:
            virtual ~FNetSerializerRegistryDelegates();

			void InitNetSerializer()
			{
				FBPSecondaryGripInfoNetSerializer::FTransformQuantizeNetSerializerPtr = &UE_NET_GET_SERIALIZER(FTransformNetQuantizeNetSerializer);
				FBPSecondaryGripInfoNetSerializer::FObjectPtrNetSerializerPtr = &UE_NET_GET_SERIALIZER(FObjectPtrNetSerializer);
				FBPSecondaryGripInfoNetSerializer::FNameNetSerializerPtr = &UE_NET_GET_SERIALIZER(FNameNetSerializer);
			}

        private:
            virtual void OnPreFreezeNetSerializerRegistry() override;

        };

        inline static FBPSecondaryGripInfoNetSerializer::FNetSerializerRegistryDelegates NetSerializerRegistryDelegates;

        static constexpr uint32 Version = 0;

        struct alignas(8) FQuantizedData
        {
			uint8 bHasSecondaryAttachment;
			FObjectNetSerializerQuantizedReferenceStorage SecondaryAttachment;
			FTransformNetQuantizeQuantizedData SecondaryRelativeTransform;

			uint8 bIsSlotGrip;

			alignas(8) uint8 SecondarySlotName[GetNameNetSerializerSafeQuantizedSize()];

			uint16 LerpToRate;

        };

        typedef FBPSecondaryGripInfo SourceType;
        typedef FQuantizedData QuantizedType;
        typedef FBPSecondaryGripInfoNetSerializerConfig ConfigType;
        inline static const ConfigType DefaultConfig;

        static constexpr bool bUseDefaultDelta = true;
		static constexpr bool bHasDynamicState = true;

		static void Quantize(FNetSerializationContext& Context, const FNetQuantizeArgs& Args)
		{

			const SourceType& Source = *reinterpret_cast<const SourceType*>(Args.Source);
			QuantizedType& Target = *reinterpret_cast<QuantizedType*>(Args.Target);

			Target.bHasSecondaryAttachment = Source.bHasSecondaryAttachment ? 1 : 0;

			if (Target.bHasSecondaryAttachment)
			{

				const FNetSerializer* ObjSerializer = FObjectPtrNetSerializerPtr;
				const FNetSerializerConfig* ObjSerializerConfig = FObjectPtrSerializerConfigPtr;

				FNetQuantizeArgs MemberArgsObj = Args;
				MemberArgsObj.NetSerializerConfig = NetSerializerConfigParam(ObjSerializerConfig);
				MemberArgsObj.Source = NetSerializerValuePointer(&Source.SecondaryAttachment);
				MemberArgsObj.Target = NetSerializerValuePointer(&Target.SecondaryAttachment);
				ObjSerializer->Quantize(Context, MemberArgsObj);

				const FNetSerializer* TransformSerializer = FTransformQuantizeNetSerializerPtr;
				const FNetSerializerConfig* TransformSerializerConfig = FTransformQuantizeSerializerConfigPtr;

				FNetQuantizeArgs MemberArgsTransform = Args;
				MemberArgsTransform.NetSerializerConfig = NetSerializerConfigParam(TransformSerializerConfig);
				MemberArgsTransform.Source = NetSerializerValuePointer(&Source.SecondaryRelativeTransform);
				MemberArgsTransform.Target = NetSerializerValuePointer(&Target.SecondaryRelativeTransform);
				TransformSerializer->Quantize(Context, MemberArgsTransform);

				Target.bIsSlotGrip = Source.bIsSlotGrip ? 1 : 0;

				const FNetSerializer* NameSerializer = FNameNetSerializerPtr;
				const FNetSerializerConfig* NameSerializerConfig = FNameSerializerConfigPtr;

				FNetQuantizeArgs MemberArgs = Args;
				MemberArgs.NetSerializerConfig = NetSerializerConfigParam(NameSerializerConfig);
				MemberArgs.Source = NetSerializerValuePointer(&Source.SecondarySlotName);
				MemberArgs.Target = NetSerializerValuePointer(&Target.SecondarySlotName);
				NameSerializer->Quantize(Context, MemberArgs);
			}

			Target.LerpToRate = GetCompressedFloat<16, 12>(Source.LerpToRate);
		}

		static void Dequantize(FNetSerializationContext& Context, const FNetDequantizeArgs& Args)
		{
			const QuantizedType& Source = *reinterpret_cast<const QuantizedType*>(Args.Source);
			SourceType& Target = *reinterpret_cast<SourceType*>(Args.Target);

			Target.bHasSecondaryAttachment = Source.bHasSecondaryAttachment != 0;

			if (Target.bHasSecondaryAttachment)
			{

				const FNetSerializer* ObjSerializer = FObjectPtrNetSerializerPtr;
				const FNetSerializerConfig* ObjSerializerConfig = FObjectPtrSerializerConfigPtr;

				FNetDequantizeArgs MemberArgsObj = Args;
				MemberArgsObj.NetSerializerConfig = NetSerializerConfigParam(ObjSerializerConfig);
				MemberArgsObj.Source = NetSerializerValuePointer(&Source.SecondaryAttachment);
				MemberArgsObj.Target = NetSerializerValuePointer(&Target.SecondaryAttachment);
				ObjSerializer->Dequantize(Context, MemberArgsObj);

				const FNetSerializer* TransformSerializer = FTransformQuantizeNetSerializerPtr;
				const FNetSerializerConfig* TransformSerializerConfig = FTransformQuantizeSerializerConfigPtr;

				FNetDequantizeArgs MemberArgsTransform = Args;
				MemberArgsTransform.NetSerializerConfig = NetSerializerConfigParam(TransformSerializerConfig);
				MemberArgsTransform.Source = NetSerializerValuePointer(&Source.SecondaryRelativeTransform);
				MemberArgsTransform.Target = NetSerializerValuePointer(&Target.SecondaryRelativeTransform);
				TransformSerializer->Dequantize(Context, MemberArgsTransform);

				Target.bIsSlotGrip = Source.bIsSlotGrip != 0;

				const FNetSerializer* NameSerializer = FNameNetSerializerPtr;
				const FNetSerializerConfig* NameSerializerConfig = FNameSerializerConfigPtr;

				FNetDequantizeArgs MemberArgs = Args;
				MemberArgs.NetSerializerConfig = NetSerializerConfigParam(NameSerializerConfig);
				MemberArgs.Source = NetSerializerValuePointer(&Source.SecondarySlotName);
				MemberArgs.Target = NetSerializerValuePointer(&Target.SecondarySlotName);
				NameSerializer->Dequantize(Context, MemberArgs);
			}

			Target.LerpToRate = GetDecompressedFloat<16, 12>(Source.LerpToRate);
		}

		static void Serialize(FNetSerializationContext& Context, const FNetSerializeArgs& Args)
		{
			const QuantizedType& Source = *reinterpret_cast<const QuantizedType*>(Args.Source);
			FNetBitStreamWriter* Writer = Context.GetBitStreamWriter();

			Writer->WriteBits(static_cast<uint32>(Source.bHasSecondaryAttachment), 1);

			if (Source.bHasSecondaryAttachment != 0)
			{

				const FNetSerializer* ObjSerializer = FObjectPtrNetSerializerPtr;
				const FNetSerializerConfig* ObjSerializerConfig = FObjectPtrSerializerConfigPtr;

				FNetSerializeArgs MemberArgsObj = Args;
				MemberArgsObj.NetSerializerConfig = NetSerializerConfigParam(ObjSerializerConfig);
				MemberArgsObj.Source = NetSerializerValuePointer(&Source.SecondaryAttachment);
				ObjSerializer->Serialize(Context, MemberArgsObj);

				const FNetSerializer* TransformSerializer = FTransformQuantizeNetSerializerPtr;
				const FNetSerializerConfig* TransformSerializerConfig = FTransformQuantizeSerializerConfigPtr;

				FNetSerializeArgs MemberArgsTransform = Args;
				MemberArgsTransform.NetSerializerConfig = NetSerializerConfigParam(TransformSerializerConfig);
				MemberArgsTransform.Source = NetSerializerValuePointer(&Source.SecondaryRelativeTransform);
				TransformSerializer->Serialize(Context, MemberArgsTransform);

				Writer->WriteBits(static_cast<uint32>(Source.bIsSlotGrip), 1);

				const FNetSerializer* NameSerializer = FNameNetSerializerPtr;
				const FNetSerializerConfig* NameSerializerConfig = FNameSerializerConfigPtr;

				FNetSerializeArgs MemberArgs = Args;
				MemberArgs.NetSerializerConfig = NetSerializerConfigParam(NameSerializerConfig);
				MemberArgs.Source = NetSerializerValuePointer(&Source.SecondarySlotName);
				NameSerializer->Serialize(Context, MemberArgs);
			}

			Writer->WriteBits(static_cast<uint32>(Source.LerpToRate), 12);
		}

		static void Deserialize(FNetSerializationContext& Context, const FNetDeserializeArgs& Args)
		{
			QuantizedType& Target = *reinterpret_cast<QuantizedType*>(Args.Target);
			FNetBitStreamReader* Reader = Context.GetBitStreamReader();

			Target.bHasSecondaryAttachment = Reader->ReadBits(1);

			if (Target.bHasSecondaryAttachment != 0)
			{

				const FNetSerializer* ObjSerializer = FObjectPtrNetSerializerPtr;
				const FNetSerializerConfig* ObjSerializerConfig = FObjectPtrSerializerConfigPtr;

				FNetDeserializeArgs MemberArgsObj = Args;
				MemberArgsObj.NetSerializerConfig = NetSerializerConfigParam(ObjSerializerConfig);
				MemberArgsObj.Target = NetSerializerValuePointer(&Target.SecondaryAttachment);
				ObjSerializer->Deserialize(Context, MemberArgsObj);

				const FNetSerializer* TransformSerializer = FTransformQuantizeNetSerializerPtr;
				const FNetSerializerConfig* TransformSerializerConfig = FTransformQuantizeSerializerConfigPtr;

				FNetDeserializeArgs MemberArgsTransform = Args;
				MemberArgsTransform.NetSerializerConfig = NetSerializerConfigParam(TransformSerializerConfig);
				MemberArgsTransform.Target = NetSerializerValuePointer(&Target.SecondaryRelativeTransform);
				TransformSerializer->Deserialize(Context, MemberArgsTransform);

				Target.bIsSlotGrip = Reader->ReadBits(1);

				const FNetSerializer* NameSerializer = FNameNetSerializerPtr;
				const FNetSerializerConfig* NameSerializerConfig = FNameSerializerConfigPtr;

				FNetDeserializeArgs MemberArgs = Args;
				MemberArgs.NetSerializerConfig = NetSerializerConfigParam(NameSerializerConfig);
				MemberArgs.Target = NetSerializerValuePointer(&Target.SecondarySlotName);
				NameSerializer->Deserialize(Context, MemberArgs);
			}

			Target.LerpToRate = Reader->ReadBits(12);
		}

		static bool IsEqual(FNetSerializationContext& Context, const FNetIsEqualArgs& Args)
		{
			if (Args.bStateIsQuantized)
			{
				const QuantizedType& QuantizedValue0 = *reinterpret_cast<const QuantizedType*>(Args.Source0);
				const QuantizedType& QuantizedValue1 = *reinterpret_cast<const QuantizedType*>(Args.Source1);
				return FPlatformMemory::Memcmp(&QuantizedValue0, &QuantizedValue1, sizeof(QuantizedType)) == 0;
			}
			else
			{
				const SourceType& L = *reinterpret_cast<const SourceType*>(Args.Source0);
				const SourceType& R = *reinterpret_cast<const SourceType*>(Args.Source1);

				if (L.bHasSecondaryAttachment != R.bHasSecondaryAttachment) return false;

				if (L.bHasSecondaryAttachment)
				{
					if (L.SecondaryAttachment != R.SecondaryAttachment) return false;
					if (!L.SecondaryRelativeTransform.Equals(R.SecondaryRelativeTransform)) return false;
					if (L.bIsSlotGrip != R.bIsSlotGrip) return false;
					if (L.SecondarySlotName != R.SecondarySlotName) return false;
				}

				if (L.LerpToRate != R.LerpToRate) return false;

				return true;
			}
		}

		static void CloneDynamicState(FNetSerializationContext& Context, const FNetCloneDynamicStateArgs& Args)
		{
			const QuantizedType* Source = reinterpret_cast<const QuantizedType*>(Args.Source);
			QuantizedType& Target = *reinterpret_cast<QuantizedType*>(Args.Target);

			const FNetSerializer* NameSerializer = FNameNetSerializerPtr;
			const FNetSerializerConfig* NameSerializerConfig = FNameSerializerConfigPtr;

			FNetCloneDynamicStateArgs MemberArgs = Args;
			MemberArgs.NetSerializerConfig = NetSerializerConfigParam(NameSerializerConfig);
			MemberArgs.Target = NetSerializerValuePointer(&Target.SecondarySlotName);
			MemberArgs.Source = NetSerializerValuePointer(&Source->SecondarySlotName);
			NameSerializer->CloneDynamicState(Context, MemberArgs);

			const FNetSerializer* ObjSerializer = FObjectPtrNetSerializerPtr;
			const FNetSerializerConfig* ObjSerializerConfig = FObjectPtrSerializerConfigPtr;

			FNetCloneDynamicStateArgs ObjMemberArgs = Args;
			ObjMemberArgs.NetSerializerConfig = NetSerializerConfigParam(ObjSerializerConfig);
			ObjMemberArgs.Target = NetSerializerValuePointer(&Target.SecondaryAttachment);
			ObjMemberArgs.Source = NetSerializerValuePointer(&Source->SecondaryAttachment);
			ObjSerializer->CloneDynamicState(Context, MemberArgs);
		}

		static void FreeDynamicState(FNetSerializationContext& Context, const FNetFreeDynamicStateArgs& Args)
		{
			QuantizedType& Source = *reinterpret_cast<QuantizedType*>(Args.Source);

			const FNetSerializer* NameSerializer = FNameNetSerializerPtr;
			const FNetSerializerConfig* NameSerializerConfig = FNameSerializerConfigPtr;

			FNetFreeDynamicStateArgs MemberArgs = Args;
			MemberArgs.NetSerializerConfig = NetSerializerConfigParam(NameSerializerConfig);
			MemberArgs.Source = NetSerializerValuePointer(&Source.SecondarySlotName);
			NameSerializer->FreeDynamicState(Context, MemberArgs);

			const FNetSerializer* ObjSerializer = FObjectPtrNetSerializerPtr;
			const FNetSerializerConfig* ObjSerializerConfig = FObjectPtrSerializerConfigPtr;

			FNetFreeDynamicStateArgs ObjMemberArgs = Args;
			ObjMemberArgs.NetSerializerConfig = NetSerializerConfigParam(ObjSerializerConfig);
			ObjMemberArgs.Source = NetSerializerValuePointer(&Source.SecondaryAttachment);
			ObjSerializer->FreeDynamicState(Context, MemberArgs);
		}

		static void Apply(FNetSerializationContext&, const FNetApplyArgs& Args)
		{
			const SourceType& Source = *reinterpret_cast<const SourceType*>(Args.Source);
			SourceType& Target = *reinterpret_cast<SourceType*>(Args.Target);

			Target.bHasSecondaryAttachment = Source.bHasSecondaryAttachment;

			if (Target.bHasSecondaryAttachment)
			{
				Target.SecondaryAttachment = Source.SecondaryAttachment;
				Target.SecondaryRelativeTransform = Source.SecondaryRelativeTransform;
				Target.bIsSlotGrip = Source.bIsSlotGrip;
				Target.SecondarySlotName = Source.SecondarySlotName;
			}
			else
			{

				Target.SecondaryAttachment = nullptr;
				Target.SecondaryRelativeTransform = FTransform::Identity;
				Target.bIsSlotGrip = false;
				Target.SecondarySlotName = NAME_None;
			}

			Target.LerpToRate = Source.LerpToRate;

		}

    };

	static const FName PropertyNetSerializerRegistry_NAME_BPSecondaryGripInfo("BPSecondaryGripInfo");
	UE_NET_IMPLEMENT_NAMED_STRUCT_NETSERIALIZER_INFO(PropertyNetSerializerRegistry_NAME_BPSecondaryGripInfo, FBPSecondaryGripInfoNetSerializer);

	FBPSecondaryGripInfoNetSerializer::FNetSerializerRegistryDelegates::~FNetSerializerRegistryDelegates()
	{
		UE_NET_UNREGISTER_NETSERIALIZER_INFO(PropertyNetSerializerRegistry_NAME_BPSecondaryGripInfo);
	}

	void FBPSecondaryGripInfoNetSerializer::FNetSerializerRegistryDelegates::OnPreFreezeNetSerializerRegistry()
	{
		InitNetSerializer();
		UE_NET_REGISTER_NETSERIALIZER_INFO(PropertyNetSerializerRegistry_NAME_BPSecondaryGripInfo);
	}

    UE_NET_IMPLEMENT_SERIALIZER(FBPSecondaryGripInfoNetSerializer);
}
