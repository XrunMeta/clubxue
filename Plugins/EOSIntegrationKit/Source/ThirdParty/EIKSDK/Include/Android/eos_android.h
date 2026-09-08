
#pragma once

#include "eos_types.h"

#pragma pack(push, 8)

#define EOS_ANDROID_INITIALIZEOPTIONS_API_LATEST 2

EOS_STRUCT(EOS_Android_InitializeOptions, (

	int32_t ApiVersion;

	void* Reserved;

	const char* OptionalInternalDirectory;

	const char* OptionalExternalDirectory;
));

#pragma pack(pop)
