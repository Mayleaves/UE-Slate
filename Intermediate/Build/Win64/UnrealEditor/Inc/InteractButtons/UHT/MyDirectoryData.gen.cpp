// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "MyDirectoryData.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

void EmptyLinkFunctionForGeneratedCodeMyDirectoryData() {}

// ********** Begin Cross Module References ********************************************************
COREUOBJECT_API UClass* Z_Construct_UClass_UObject();
COREUOBJECT_API UScriptStruct* Z_Construct_UScriptStruct_FDirectoryPath();
INTERACTBUTTONS_API UClass* Z_Construct_UClass_UMyDirectoryData();
INTERACTBUTTONS_API UClass* Z_Construct_UClass_UMyDirectoryData_NoRegister();
UPackage* Z_Construct_UPackage__Script_InteractButtons();
// ********** End Cross Module References **********************************************************

// ********** Begin Class UMyDirectoryData *********************************************************
void UMyDirectoryData::StaticRegisterNativesUMyDirectoryData()
{
}
FClassRegistrationInfo Z_Registration_Info_UClass_UMyDirectoryData;
UClass* UMyDirectoryData::GetPrivateStaticClass()
{
	using TClass = UMyDirectoryData;
	if (!Z_Registration_Info_UClass_UMyDirectoryData.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			StaticPackage(),
			TEXT("MyDirectoryData"),
			Z_Registration_Info_UClass_UMyDirectoryData.InnerSingleton,
			StaticRegisterNativesUMyDirectoryData,
			sizeof(TClass),
			alignof(TClass),
			TClass::StaticClassFlags,
			TClass::StaticClassCastFlags(),
			TClass::StaticConfigName(),
			(UClass::ClassConstructorType)InternalConstructor<TClass>,
			(UClass::ClassVTableHelperCtorCallerType)InternalVTableHelperCtorCaller<TClass>,
			UOBJECT_CPPCLASS_STATICFUNCTIONS_FORCLASS(TClass),
			&TClass::Super::StaticClass,
			&TClass::WithinClass::StaticClass
		);
	}
	return Z_Registration_Info_UClass_UMyDirectoryData.InnerSingleton;
}
UClass* Z_Construct_UClass_UMyDirectoryData_NoRegister()
{
	return UMyDirectoryData::GetPrivateStaticClass();
}
struct Z_Construct_UClass_UMyDirectoryData_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "//  \xe2\x80\x9c\xe4\xbd\xbf\xe7\x94\xa8 UE \xe5\xb1\x9e\xe6\x80\xa7\xe7\xbc\x96\xe8\xbe\x91\xe6\xa1\x86\xe6\x9e\xb6\xe2\x80\x9d \xe7\x9a\x84\xe5\x9f\xba\xe7\xa1\x80\xe5\xbf\x85\xe8\xa6\x81\xe6\x9d\xa1\xe4\xbb\xb6\n// \xe6\xa0\xb8\xe5\xbf\x83\xe6\x98\xaf\xef\xbc\x9aUObject \xe6\xb4\xbe\xe7\x94\x9f\xe7\xb1\xbb + UPROPERTY \xe6\xa0\x87\xe8\xae\xb0\xe7\x9a\x84\xe5\xb1\x9e\xe6\x80\xa7\n" },
#endif
		{ "IncludePath", "MyDirectoryData.h" },
		{ "ModuleRelativePath", "Private/MyDirectoryData.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xe2\x80\x9c\xe4\xbd\xbf\xe7\x94\xa8 UE \xe5\xb1\x9e\xe6\x80\xa7\xe7\xbc\x96\xe8\xbe\x91\xe6\xa1\x86\xe6\x9e\xb6\xe2\x80\x9d \xe7\x9a\x84\xe5\x9f\xba\xe7\xa1\x80\xe5\xbf\x85\xe8\xa6\x81\xe6\x9d\xa1\xe4\xbb\xb6\n\xe6\xa0\xb8\xe5\xbf\x83\xe6\x98\xaf\xef\xbc\x9aUObject \xe6\xb4\xbe\xe7\x94\x9f\xe7\xb1\xbb + UPROPERTY \xe6\xa0\x87\xe8\xae\xb0\xe7\x9a\x84\xe5\xb1\x9e\xe6\x80\xa7" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_SourceDirectory_MetaData[] = {
		{ "Category", "File Directory" },
		{ "ModuleRelativePath", "Private/MyDirectoryData.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "The source directory used for input files." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_SaveDirectory_MetaData[] = {
		{ "Category", "File Directory" },
		{ "DisplayName", "Save Directory" },
		{ "ModuleRelativePath", "Private/MyDirectoryData.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "The directory where output files will be saved." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStructPropertyParams NewProp_SourceDirectory;
	static const UECodeGen_Private::FStructPropertyParams NewProp_SaveDirectory;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UMyDirectoryData>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FStructPropertyParams Z_Construct_UClass_UMyDirectoryData_Statics::NewProp_SourceDirectory = { "SourceDirectory", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UMyDirectoryData, SourceDirectory), Z_Construct_UScriptStruct_FDirectoryPath, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_SourceDirectory_MetaData), NewProp_SourceDirectory_MetaData) };
const UECodeGen_Private::FStructPropertyParams Z_Construct_UClass_UMyDirectoryData_Statics::NewProp_SaveDirectory = { "SaveDirectory", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UMyDirectoryData, SaveDirectory), Z_Construct_UScriptStruct_FDirectoryPath, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_SaveDirectory_MetaData), NewProp_SaveDirectory_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UMyDirectoryData_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UMyDirectoryData_Statics::NewProp_SourceDirectory,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UMyDirectoryData_Statics::NewProp_SaveDirectory,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UMyDirectoryData_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_UMyDirectoryData_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UObject,
	(UObject* (*)())Z_Construct_UPackage__Script_InteractButtons,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UMyDirectoryData_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UMyDirectoryData_Statics::ClassParams = {
	&UMyDirectoryData::StaticClass,
	nullptr,
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	Z_Construct_UClass_UMyDirectoryData_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	UE_ARRAY_COUNT(Z_Construct_UClass_UMyDirectoryData_Statics::PropPointers),
	0,
	0x001000A0u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UMyDirectoryData_Statics::Class_MetaDataParams), Z_Construct_UClass_UMyDirectoryData_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UMyDirectoryData()
{
	if (!Z_Registration_Info_UClass_UMyDirectoryData.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UMyDirectoryData.OuterSingleton, Z_Construct_UClass_UMyDirectoryData_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UMyDirectoryData.OuterSingleton;
}
DEFINE_VTABLE_PTR_HELPER_CTOR(UMyDirectoryData);
UMyDirectoryData::~UMyDirectoryData() {}
// ********** End Class UMyDirectoryData ***********************************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_DevelopmentTools_Unreal_Projects_CISAI_SD_Factory_Plugins_SyntheticDataFactory_Source_InteractButtons_Private_MyDirectoryData_h__Script_InteractButtons_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UMyDirectoryData, UMyDirectoryData::StaticClass, TEXT("UMyDirectoryData"), &Z_Registration_Info_UClass_UMyDirectoryData, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UMyDirectoryData), 723184230U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_DevelopmentTools_Unreal_Projects_CISAI_SD_Factory_Plugins_SyntheticDataFactory_Source_InteractButtons_Private_MyDirectoryData_h__Script_InteractButtons_1949283364(TEXT("/Script/InteractButtons"),
	Z_CompiledInDeferFile_FID_DevelopmentTools_Unreal_Projects_CISAI_SD_Factory_Plugins_SyntheticDataFactory_Source_InteractButtons_Private_MyDirectoryData_h__Script_InteractButtons_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_DevelopmentTools_Unreal_Projects_CISAI_SD_Factory_Plugins_SyntheticDataFactory_Source_InteractButtons_Private_MyDirectoryData_h__Script_InteractButtons_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
