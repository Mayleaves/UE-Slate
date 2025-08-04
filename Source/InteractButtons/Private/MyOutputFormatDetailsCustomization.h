#pragma once

#include "IDetailCustomization.h"

class UMyDirectoryData;
class IDetailLayoutBuilder;
class IDetailCategoryBuilder;
class IPropertyHandle;

#define LOCTEXT_NAMESPACE "SMoviePipelineEditor"

class FMyOutputFormatDetailsCustomization final : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	static void AddDirectoryPropertyRow(
		IDetailCategoryBuilder& Category,
		const IDetailLayoutBuilder& DetailBuilder,
		const FName& DirectoryPropertyName,
		const FName& PathPropertyName,
		const FText& PickButtonToolTip,
		const FText& OpenButtonToolTip,
		UMyDirectoryData* DirData,
		TFunction<FString(UMyDirectoryData*)> GetPathFunc,
		const FString& DialogTitle
	);
};

#undef LOCTEXT_NAMESPACE