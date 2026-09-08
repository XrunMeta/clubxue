

#pragma once

#include "CoreGlobals.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Templates/SharedPointer.h"

#if WITH_EOS_SDK
#if defined(EOS_PLATFORM_BASE_FILE_NAME)
#include EOS_PLATFORM_BASE_FILE_NAME
#endif

#include "eos_common.h"
#endif

#define EOS_CONNECTION_URL_PREFIX TEXT("EOS")
#define EOS_URL_SEPARATOR TEXT(":")

class EIKSHARED_API FCallbackBase
{
public:
	virtual ~FCallbackBase() {}
};
#if WITH_EOS_SDK

#if ENGINE_MAJOR_VERSION == 5
template<typename CallbackFuncType, typename CallbackParamType, typename OwningType, typename CallbackReturnType = void, typename... CallbackExtraParams>
#else
template<typename CallbackFuncType, typename CallbackType>
#endif
class TEIKGlobalCallback :
	public FCallbackBase
{
public:
#if ENGINE_MAJOR_VERSION == 5
	TFunction<CallbackReturnType(const CallbackParamType*, CallbackExtraParams... ExtraParams)> CallbackLambda;
	TEIKGlobalCallback(TWeakPtr<OwningType> InOwner)
		: FCallbackBase()
		, Owner(InOwner)
	{
	}
#else
	TFunction<void(const CallbackType*)> CallbackLambda;
	TEIKGlobalCallback() = default;
#endif
	virtual ~TEIKGlobalCallback() = default;

	CallbackFuncType GetCallbackPtr()
	{
		return &CallbackImpl;
	}

	bool bIsGameThreadCallback = true;

private:

#if ENGINE_MAJOR_VERSION == 5

	TWeakPtr<OwningType> Owner;

	static CallbackReturnType EOS_CALL CallbackImpl(const CallbackParamType* Data, CallbackExtraParams... ExtraParams)
	{
		TEIKGlobalCallback* CallbackThis = (TEIKGlobalCallback*)Data->ClientData;
		check(CallbackThis);

		if (CallbackThis->bIsGameThreadCallback)
		{
			check(IsInGameThread());
		}

		if (CallbackThis->Owner.IsValid())
		{
			check(CallbackThis->CallbackLambda);

			if constexpr (std::is_void<CallbackReturnType>::value)
			{
				CallbackThis->CallbackLambda(Data, ExtraParams...);
			}
			else
			{
				return CallbackThis->CallbackLambda(Data, ExtraParams...);
			}
		}

		if constexpr (!std::is_void<CallbackReturnType>::value)
		{

			return CallbackReturnType{};
		}
	}

#else

	static void EOS_CALL CallbackImpl(const CallbackType* Data)
	{
		check(IsInGameThread());

		TEIKGlobalCallback* CallbackThis = (TEIKGlobalCallback*)Data->ClientData;
		check(CallbackThis);

		check(CallbackThis->CallbackLambda);
		CallbackThis->CallbackLambda(Data);
	}
#endif
};

#endif