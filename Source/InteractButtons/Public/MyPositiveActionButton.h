#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SComboButton.h"

#define LOCTEXT_NAMESPACE "MySPositiveActionButton"

class SMyPositiveActionButton final : public SComboButton
{
public:
	SLATE_BEGIN_ARGS(SMyPositiveActionButton)
			: _Text(LOCTEXT("DefaultButtonText", "Button")) // 默认文本
			  , _Icon(FAppStyle::Get().GetBrush("Icons.Plus")) // 默认图标
			  , _OnGetMenuContent()
		{
		}

		/** 按钮显示文本 */
		SLATE_ATTRIBUTE(FText, Text)

		/** 按钮图标（Slate Brush） */
		SLATE_ATTRIBUTE(const FSlateBrush*, Icon)

		/** 下拉菜单内容回调 */
		SLATE_EVENT(FOnGetContent, OnGetMenuContent)

	SLATE_END_ARGS()

	/** 构造方法 */
	void Construct(const FArguments& InArgs);

private:
	FButtonStyle CustomButtonStyle; // 成员变量
};

#undef LOCTEXT_NAMESPACE
