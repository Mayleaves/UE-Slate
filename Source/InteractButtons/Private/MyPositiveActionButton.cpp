
#include "MyPositiveActionButton.h"

#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Styling/AppStyle.h"

void SMyPositiveActionButton::Construct(const FArguments& InArgs)
{
	SComboButton::Construct(
		SComboButton::FArguments()
		.ContentPadding(FMargin(4.0f, 3.0f))  // 内边距
		.OnGetMenuContent(InArgs._OnGetMenuContent)  // 菜单回调
		.HasDownArrow(false)  // 隐藏默认箭头
		.ButtonStyle(FAppStyle::Get(), "Button")
		.ButtonContent()  // 自定义按钮内容
		[
			SNew(SHorizontalBox)  // 水平布局
			+ SHorizontalBox::Slot()  // 图标 Slot
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SImage)
						.Image(InArgs._Icon)  // 使用传入的图标
						// 设置图标为绿色
						.ColorAndOpacity(FStyleColors::AccentGreen)
				]
			+ SHorizontalBox::Slot()  // 文本 Slot
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(2.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
						.Text(InArgs._Text)  // 使用传入的文本
						.Font(FAppStyle::Get().GetFontStyle("ButtonText"))
						.ColorAndOpacity(FLinearColor::White)  // 白色文字
				]
		]
	);
}