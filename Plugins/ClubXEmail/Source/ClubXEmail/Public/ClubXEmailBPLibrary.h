#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ClubXEmailBPLibrary.generated.h"

UCLASS()
class CLUBXEMAIL_API UClubXEmailBPLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category="ClubX|Android")
    static void OpenEmail(const FString& Email);
};