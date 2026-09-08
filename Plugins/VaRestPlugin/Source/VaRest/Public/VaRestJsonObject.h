

#pragma once

#include "VaRestDefines.h"

#include "Dom/JsonObject.h"
#include "Templates/UnrealTypeTraits.h"

#include "VaRestJsonObject.generated.h"

class UVaRestJsonValue;

UCLASS(BlueprintType, Blueprintable)
class VAREST_API UVaRestJsonObject : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	void Reset();

	TSharedRef<FJsonObject>& GetRootObject();

	void SetRootObject(const TSharedPtr<FJsonObject>& JsonObject);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	FString EncodeJson() const;

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	FString EncodeJsonToSingleString() const;

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	bool DecodeJson(const FString& JsonString, bool bUseIncrementalParser = true);

	UFUNCTION(BlueprintPure, Category = "VaRest|Json")
	FString GetFieldTypeString(const FString& FieldName) const;

	UFUNCTION(BlueprintPure, Category = "VaRest|Json")
	TArray<FString> GetFieldNames() const;

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	bool HasField(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	void RemoveField(const FString& FieldName);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	UVaRestJsonValue* GetField(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	void SetField(const FString& FieldName, UVaRestJsonValue* JsonValue);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	TArray<UVaRestJsonValue*> GetArrayField(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	void SetArrayField(const FString& FieldName, const TArray<UVaRestJsonValue*>& InArray);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	void MergeJsonObject(UVaRestJsonObject* InJsonObject, bool Overwrite);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	float GetNumberField(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	void SetNumberField(const FString& FieldName, float Number);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	void SetNumberFieldDouble(const FString& FieldName, double Number);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	int32 GetIntegerField(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	void SetIntegerField(const FString& FieldName, int32 Number);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	int64 GetInt64Field(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	void SetInt64Field(const FString& FieldName, int64 Number);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	FString GetStringField(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	void SetStringField(const FString& FieldName, const FString& StringValue);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	bool GetBoolField(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	void SetBoolField(const FString& FieldName, bool InValue);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	UVaRestJsonObject* GetObjectField(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	void SetObjectField(const FString& FieldName, UVaRestJsonObject* JsonObject);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	void SetMapFields_string(const TMap<FString, FString>& Fields);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	void SetMapFields_uint8(const TMap<FString, uint8>& Fields);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	void SetMapFields_int32(const TMap<FString, int32>& Fields);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	void SetMapFields_int64(const TMap<FString, int64>& Fields);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	void SetMapFields_bool(const TMap<FString, bool>& Fields);

private:

	template <typename T>
	void SetMapFields_Impl(const TMap<FString, T>& Fields)
	{
		for (auto& field : Fields)
		{

			if (std::is_same_v<T, uint8> || std::is_same_v<T, int32> || std::is_same_v<T, int64> || std::is_same_v<T, float>)
			{
				SetNumberField(field.Key, field.Value);
			}
			else if (std::is_same_v<T, bool>)
			{
				SetBoolField(field.Key, (bool)field.Value);
			}
		}
	}

	template <typename T>
	TArray<T> GetTypeArrayField(const FString& FieldName) const
	{
		TArray<T> NumberArray;
		if (!JsonObj->HasTypedField<EJson::Array>(FieldName) || FieldName.IsEmpty())
		{
			UE_LOG(LogVaRest, Warning, TEXT("%s: No field with name %s of type Array"), *VA_FUNC_LINE, *FieldName);
			return NumberArray;
		}

		const TArray<TSharedPtr<FJsonValue>> JsonArrayValues = JsonObj->GetArrayField(FieldName);
		for (TArray<TSharedPtr<FJsonValue>>::TConstIterator It(JsonArrayValues); It; ++It)
		{
			const auto Value = (*It).Get();
			if (Value->Type != EJson::Number)
			{
				UE_LOG(LogVaRest, Error, TEXT("Not Number element in array with field name %s"), *FieldName);
			}

			NumberArray.Add((*It)->AsNumber());
		}

		return NumberArray;
	}

public:

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	TArray<float> GetNumberArrayField(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	TArray<int32> GetIntegerArrayField(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	void SetNumberArrayField(const FString& FieldName, const TArray<float>& NumberArray);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	void SetNumberArrayFieldDouble(const FString& FieldName, const TArray<double>& NumberArray);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	TArray<FString> GetStringArrayField(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	void SetStringArrayField(const FString& FieldName, const TArray<FString>& StringArray);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	TArray<bool> GetBoolArrayField(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	void SetBoolArrayField(const FString& FieldName, const TArray<bool>& BoolArray);

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	TArray<UVaRestJsonObject*> GetObjectArrayField(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	void SetObjectArrayField(const FString& FieldName, const TArray<UVaRestJsonObject*>& ObjectArray);

public:

	int32 DeserializeFromUTF8Bytes(const ANSICHAR* Bytes, int32 Size);

	int32 DeserializeFromTCHARBytes(const TCHAR* Bytes, int32 Size);

	void DecodeFromArchive(TUniquePtr<FArchive>& Reader);

public:

	bool WriteToFile(const FString& Path) const;

	UFUNCTION(BlueprintCallable, Category = "VaRest|Json")
	bool WriteToFilePath(const FString& Path, const bool bIsRelativeToProjectDir = true);

	static bool WriteStringToArchive(FArchive& Ar, const TCHAR* StrPtr, int64 Len);

private:

	TSharedRef<FJsonObject> JsonObj;
};
