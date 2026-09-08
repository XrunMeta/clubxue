

#pragma once

#include "CoreMinimal.h"
#include "StreamlineLibrary.h"
DECLARE_LOG_CATEGORY_EXTERN(LogStreamlineBlueprint, Verbose, All);

#if WITH_STREAMLINE

#define TRY_INIT_STREAMLINE_LIBRARY_AND_RETURN(ReturnValueOrEmptyOrVoidPreFiveThree) \
if (!TryInitStreamlineLibrary()) \
{ \
	UE_LOG(LogStreamlineBlueprint, Error, TEXT("%s should not be called before PostEngineInit"), ANSI_TO_TCHAR(__FUNCTION__)); \
	return ReturnValueOrEmptyOrVoidPreFiveThree; \
}

#else

#define TRY_INIT_STREAMLINE_LIBRARY_AND_RETURN(ReturnValueWhichCanBeEmpty) 

#endif

