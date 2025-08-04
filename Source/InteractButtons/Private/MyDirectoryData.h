#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "MyDirectoryData.generated.h"

//  “使用 UE 属性编辑框架” 的基础必要条件
// 核心是：UObject 派生类 + UPROPERTY 标记的属性
UCLASS()
class INTERACTBUTTONS_API UMyDirectoryData : public UObject  // UMySettings
{
	GENERATED_BODY()
public:
	UMyDirectoryData(); // 构造函数声明
	
	UPROPERTY(EditAnywhere, Category = "File Directory")
	FDirectoryPath SourceDirectory;  // 默认显示名为 "Source Directory"
	
	UPROPERTY(EditAnywhere, Category = "File Directory", meta = (DisplayName = "Save Directory"))  // （可选）显示名为 "Save Directory"
	FDirectoryPath SaveDirectory;
};