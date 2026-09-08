#include "Serializers/FBPAdvGripPhysicsSettingsNetSerializer.h"
#include "Serializers/SerializerHelpers.h"
#include "Iris/Serialization/NetSerializerDelegates.h"
#include "Iris/Serialization/NetSerializers.h"
#include "Iris/ReplicationState/PropertyNetSerializerInfoRegistry.h"
#include "Iris/ReplicationState/ReplicationStateDescriptorBuilder.h"

namespace UE::Net
{

    struct FBPAdvGripPhysicsSettingsNetSerializer
    {

        class FNetSerializerRegistryDelegates final : private UE::Net::FNetSerializerRegistryDelegates
        {
        public:
            virtual ~FNetSerializerRegistryDelegates();

        private:
            virtual void OnPreFreezeNetSerializerRegistry() override;

        };

        inline static FBPAdvGripPhysicsSettingsNetSerializer::FNetSerializerRegistryDelegates NetSerializerRegistryDelegates;

        static constexpr uint32 Version = 0;

        struct FQuantizedData
        {
            uint32 bUsePhysicsSettings : 1;
            uint32 PhysicsConstraintType : 1; 
            uint32 PhysicsGripLocationSettings : 3; 
            uint32 bTurnOffGravityDuringGrip : 1;
            uint32 bSkipSettingSimulating : 1;
            uint32 bUseCustomAngularValues : 1;
            uint32 Reserved : 25; 

            uint32 LinearMaxForceCoefficient;
            uint32 AngularMaxForceCoefficient;

            float AngularStiffness;
            float AngularDamping;
        };

        typedef FBPAdvGripPhysicsSettings SourceType;
        typedef FQuantizedData QuantizedType;
        typedef FBPAdvGripPhysicsSettingsNetSerializerConfig ConfigType;
        inline static const ConfigType DefaultConfig;

        static constexpr bool bUseDefaultDelta = true;

        static void Quantize(FNetSerializationContext& Context, const FNetQuantizeArgs& Args)
        {

            const SourceType& Source = *reinterpret_cast<const SourceType*>(Args.Source);
            QuantizedType& Target = *reinterpret_cast<QuantizedType*>(Args.Target);

            Target.bUsePhysicsSettings = Source.bUsePhysicsSettings;

            if (Target.bUsePhysicsSettings)
            {
                Target.PhysicsConstraintType = (uint32)Source.PhysicsConstraintType;
                Target.PhysicsGripLocationSettings = (uint32)Source.PhysicsGripLocationSettings;

                Target.bTurnOffGravityDuringGrip = Source.bTurnOffGravityDuringGrip;
                Target.bSkipSettingSimulating = Source.bSkipSettingSimulating;

                Target.LinearMaxForceCoefficient = GetCompressedFloat<512, 17>(Source.LinearMaxForceCoefficient);
                Target.AngularMaxForceCoefficient = GetCompressedFloat<512, 17>(Source.AngularMaxForceCoefficient);

                Target.bUseCustomAngularValues = Source.bUseCustomAngularValues;

                if (Target.bUseCustomAngularValues)
                {

                    Target.AngularStiffness = Source.AngularStiffness;
                    Target.AngularDamping = Source.AngularDamping;
                }
            }
        }

        static void Dequantize(FNetSerializationContext& Context, const FNetDequantizeArgs& Args)
        {
            const QuantizedType& Source = *reinterpret_cast<const QuantizedType*>(Args.Source);
            SourceType& Target = *reinterpret_cast<SourceType*>(Args.Target);

            Target.bUsePhysicsSettings = Source.bUsePhysicsSettings != 0;

            if (Source.bUsePhysicsSettings)
            {
                Target.AngularDamping = Source.AngularDamping;
                Target.AngularStiffness = Source.AngularStiffness;
                Target.bSkipSettingSimulating = Source.bSkipSettingSimulating != 0;
                Target.bTurnOffGravityDuringGrip = Source.bTurnOffGravityDuringGrip != 0;

                Target.PhysicsConstraintType = (EPhysicsGripConstraintType)Source.PhysicsConstraintType;
                Target.PhysicsGripLocationSettings = (EPhysicsGripCOMType)Source.PhysicsGripLocationSettings;

                Target.bUseCustomAngularValues = Source.bUseCustomAngularValues != 0;

                if (Target.bUseCustomAngularValues)
                {
                    Target.AngularMaxForceCoefficient = GetDecompressedFloat<512, 17>(Source.AngularMaxForceCoefficient);
                    Target.LinearMaxForceCoefficient = GetDecompressedFloat<512, 17>(Source.LinearMaxForceCoefficient);
                }
            }
        }

        static void Serialize(FNetSerializationContext& Context, const FNetSerializeArgs& Args)
        {
            const QuantizedType& Source = *reinterpret_cast<const QuantizedType*>(Args.Source);
            FNetBitStreamWriter* Writer = Context.GetBitStreamWriter();

            Writer->WriteBool(Source.bUsePhysicsSettings);

            if (Source.bUsePhysicsSettings)
            {
                Writer->WriteBits(static_cast<uint32>(Source.PhysicsGripLocationSettings), 3);
                Writer->WriteBits(static_cast<uint32>(Source.PhysicsConstraintType), 1);
                Writer->WriteBool(Source.bTurnOffGravityDuringGrip);
                Writer->WriteBool(Source.bSkipSettingSimulating);

                Writer->WriteBits(Source.LinearMaxForceCoefficient, 17);
                Writer->WriteBits(Source.AngularMaxForceCoefficient, 17);

                Writer->WriteBool(Source.bUseCustomAngularValues);
                if (Source.bUseCustomAngularValues)
                {
                    Writer->WriteBits(Source.AngularStiffness, 32);
                    Writer->WriteBits(Source.AngularDamping, 32);
                }
            }
        }

        static void Deserialize(FNetSerializationContext& Context, const FNetDeserializeArgs& Args)
        {
            QuantizedType& Target = *reinterpret_cast<QuantizedType*>(Args.Target);
            FNetBitStreamReader* Reader = Context.GetBitStreamReader();

            Target.bUsePhysicsSettings = Reader->ReadBool();

            if (Target.bUsePhysicsSettings)
            {
                Target.PhysicsGripLocationSettings = Reader->ReadBits(3);
                Target.PhysicsConstraintType = Reader->ReadBits(1);
                Target.bTurnOffGravityDuringGrip = Reader->ReadBool();
                Target.bSkipSettingSimulating = Reader->ReadBool();

                Target.LinearMaxForceCoefficient = Reader->ReadBits(17);
                Target.AngularMaxForceCoefficient = Reader->ReadBits(17);

                Target.bUseCustomAngularValues = Reader->ReadBool();
                if (Target.bUseCustomAngularValues)
                {                  
                    Target.AngularStiffness = Reader->ReadBits(32);
                    Target.AngularDamping = Reader->ReadBits(32);
                }
            }
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

                if (L.bUsePhysicsSettings != R.bUsePhysicsSettings) return false;
                if (!L.bUsePhysicsSettings) return true;

                if (L.PhysicsGripLocationSettings != R.PhysicsGripLocationSettings) return false;
                if (L.PhysicsConstraintType != R.PhysicsConstraintType) return false;
                if (L.bTurnOffGravityDuringGrip != R.bTurnOffGravityDuringGrip) return false;
                if (L.bSkipSettingSimulating != R.bSkipSettingSimulating) return false;

                if (!FMath::IsNearlyEqual(L.LinearMaxForceCoefficient, R.LinearMaxForceCoefficient)) return false;
                if (!FMath::IsNearlyEqual(L.AngularMaxForceCoefficient, R.AngularMaxForceCoefficient)) return false;

                if (L.bUseCustomAngularValues != R.bUseCustomAngularValues) return false;

                if (L.bUseCustomAngularValues)
                {
                    if (!FMath::IsNearlyEqual(L.AngularStiffness, R.AngularStiffness)) return false;
                    if (!FMath::IsNearlyEqual(L.AngularDamping, R.AngularDamping)) return false;
                }

                return true;
            }
        }
    };

	static const FName PropertyNetSerializerRegistry_NAME_BPAdvGripPhysicsSettings("BPAdvGripPhysicsSettings");
	UE_NET_IMPLEMENT_NAMED_STRUCT_NETSERIALIZER_INFO(PropertyNetSerializerRegistry_NAME_BPAdvGripPhysicsSettings, FBPAdvGripPhysicsSettingsNetSerializer);

	FBPAdvGripPhysicsSettingsNetSerializer::FNetSerializerRegistryDelegates::~FNetSerializerRegistryDelegates()
	{
		UE_NET_UNREGISTER_NETSERIALIZER_INFO(PropertyNetSerializerRegistry_NAME_BPAdvGripPhysicsSettings);
	}

	void FBPAdvGripPhysicsSettingsNetSerializer::FNetSerializerRegistryDelegates::OnPreFreezeNetSerializerRegistry()
	{
		UE_NET_REGISTER_NETSERIALIZER_INFO(PropertyNetSerializerRegistry_NAME_BPAdvGripPhysicsSettings);
	}

    UE_NET_IMPLEMENT_SERIALIZER(FBPAdvGripPhysicsSettingsNetSerializer);
}
