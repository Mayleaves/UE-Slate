#include "MyOutputFormatDetailsCustomization.h"
#include "DesktopPlatformModule.h"
#include "PropertyHandle.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDesktopPlatform.h"
#include "MyDirectoryData.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/Paths.h"
#include "Misc/ScopedSlowTask.h"
#include "HAL/PlatformProcess.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "SMoviePipelineEditor"

TSharedRef<IDetailCustomization> FMyOutputFormatDetailsCustomization::MakeInstance()
{
	return MakeShared<FMyOutputFormatDetailsCustomization>();
}

void FMyOutputFormatDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	UMyDirectoryData* DirData = Cast<UMyDirectoryData>(Objects.Num() > 0 ? Objects[0].Get() : nullptr);

	// File Directory
	IDetailCategoryBuilder& Cat = DetailBuilder.EditCategory(TEXT("File Directory"));

	// Source Directory
	AddDirectoryPropertyRow(
		Cat,
		DetailBuilder,
		GET_MEMBER_NAME_CHECKED(UMyDirectoryData, SourceDirectory),
		GET_MEMBER_NAME_CHECKED(FDirectoryPath, Path),
		NSLOCTEXT("MyDirectoryPicker", "FolderButtonToolTipText", "Choose a directory from this computer"),
		NSLOCTEXT("MyDirectoryData", "OpenFolder", "Open this directory"),
		DirData,
		[](UMyDirectoryData* Data) -> FString { return Data ? Data->SourceDirectory.Path : FString(); },
		TEXT("Choose a directory")
	);

	// Save Directory
	AddDirectoryPropertyRow(
		Cat,
		DetailBuilder,
		GET_MEMBER_NAME_CHECKED(UMyDirectoryData, SaveDirectory),
		GET_MEMBER_NAME_CHECKED(FDirectoryPath, Path),
		NSLOCTEXT("MyDirectoryPicker", "SaveFolderButtonToolTipText", "Choose a directory from this computer"),
		NSLOCTEXT("MyDirectoryData", "OpenSaveFolder", "Open this save directory"),
		DirData,
		[](UMyDirectoryData* Data) -> FString { return Data ? Data->SaveDirectory.Path : FString(); },
		TEXT("Choose a save directory")
	);
}

void FMyOutputFormatDetailsCustomization::AddDirectoryPropertyRow(
	IDetailCategoryBuilder& Category,
	const IDetailLayoutBuilder& DetailBuilder,
	const FName& DirectoryPropertyName,
	const FName& PathPropertyName,
	const FText& PickButtonToolTip,
	const FText& OpenButtonToolTip,
	UMyDirectoryData* DirData,
	TFunction<FString(UMyDirectoryData*)> GetPathFunc,
	const FString& DialogTitle
)
{
	const TSharedPtr<IPropertyHandle> DirHandle = DetailBuilder.GetProperty(DirectoryPropertyName);
	TSharedPtr<IPropertyHandle> PathHandle = DirHandle->GetChildHandle(PathPropertyName);

	IDetailPropertyRow& Row = Category.AddProperty(DirHandle.ToSharedRef());
	
	Row.CustomWidget()
	.NameContent()
	[
		DirHandle->CreatePropertyNameWidget()  // 第一列：属性名
	]
	.ValueContent()
	.MinDesiredWidth(500)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()  // 第二列：属性值
		.AutoWidth()
		[
			PathHandle->CreatePropertyValueWidget()
		]
		+ SHorizontalBox::Slot()  // 第二列：按钮
		.AutoWidth()
		.Padding(4, 4, 2, 4)
		[
			SNew(SButton)
			.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
			.ToolTipText(PickButtonToolTip)
			.ContentPadding(2.0f)
			.ForegroundColor(FSlateColor::UseForeground())
			.IsFocusable(false)
			.OnClicked_Lambda([PathHandle, DialogTitle]()
			{
				const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
				if (IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get(); DesktopPlatform && ParentWindowHandle)
				{
					FString SelectedDir;
					const bool bOpened = DesktopPlatform->OpenDirectoryDialog(
						ParentWindowHandle,
						DialogTitle,
						TEXT(""),
						SelectedDir
					);
					if (bOpened)
					{
						PathHandle->SetValue(SelectedDir);
					}
				}
				return FReply::Handled();
			})
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("PropertyWindow.Button_Ellipsis"))
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
		]
		+ SHorizontalBox::Slot()  // 第二列：打开按钮
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.ToolTipText(OpenButtonToolTip)
			.OnClicked_Lambda([DirData, GetPathFunc]() {
				const FString Path = GetPathFunc(DirData);
				if (!Path.IsEmpty())
					FPlatformProcess::ExploreFolder(*Path);
				return FReply::Handled();
			})
			.ContentPadding(4)
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("Icons.FolderOpen"))
			]
		]
	];
}

#undef LOCTEXT_NAMESPACE