#include "LabelingOptionsDetailsCustomization.h"
#include "PropertyHandle.h"
#include "DetailLayoutBuilder.h"
#include "LabelingOptionsData.h"
#include "DetailCategoryBuilder.h"

#define LOCTEXT_NAMESPACE "SLabelingOptionsEditor"

TSharedRef<IDetailCustomization> FLabelingOptionsDetailsCustomization::MakeInstance()
{
	return MakeShared<FLabelingOptionsDetailsCustomization>();
}

void FLabelingOptionsDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	const TSharedRef<IPropertyHandle> MaxDistHandle = DetailBuilder.GetProperty(
		GET_MEMBER_NAME_CHECKED(ULabelingOptionsData, MaximumDistance));

	// Labeling Options
	IDetailCategoryBuilder& Cat = DetailBuilder.EditCategory(TEXT("Labeling Options"));
	Cat.AddProperty(MaxDistHandle);
}

#undef LOCTEXT_NAMESPACE