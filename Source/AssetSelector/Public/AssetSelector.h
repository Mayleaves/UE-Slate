// Slate 控件类（SAssetSelector）

#pragma once

#include "Widgets/SCompoundWidget.h"

class ASSETSELECTOR_API SAssetSelector : public SCompoundWidget  // 注意导出宏
{
public:
	SLATE_BEGIN_ARGS(SAssetSelector) {}
	SLATE_EVENT(FOnClicked, OnGenerateButtonClicked)  // 声明按钮点击事件
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FOnClicked OnGenerateButtonClicked;  // 存储外部传入的事件
};
