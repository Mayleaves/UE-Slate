#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "LabelingOptionsData.generated.h"

// 用于CineCamera采集最大距离范围内StaticMesh的设置数据
UCLASS()
class INTERACTBUTTONS_API ULabelingOptionsData : public UObject
{
	GENERATED_BODY()
public:
	ULabelingOptionsData();

	// 最大采集距离（单位：米）
	UPROPERTY(EditAnywhere, Category = "Labeling Options",
		meta = (DisplayName = "Maximum Distance (m)", ClampMin = "0", ToolTip = "The maximum distance at which the current cinecamera can detect static meshes."))
	float MaximumDistance;
};
