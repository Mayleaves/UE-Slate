#pragma once

#include "CoreMinimal.h"

// 菜单项 - 唯一标识和显示名称
struct FMenuItem
{
	FName Name; // 唯一标识（方便反查、传递）
	FString Label; // 显示名称
	// 可扩展其他字段...

	// 默认构造函数
	FMenuItem() = default;
	// 带参构造函数
	FMenuItem(const FName& InName, const FString& InLabel)
		: Name(InName)
		, Label(InLabel)
	{
	}
};

// CineCamera
struct FCineCameraData
{
	FName Name;            // 唯一标识
	FString Label;         // 显示名称
	FVector Location;      // 世界坐标 cm
	FRotator Rotation;     // 旋转
	FVector Scale;         // 缩放
	FMatrix WorldMatrix;   // 4x4变换矩阵 cm

	float FocalLength;     // 焦距 mm
	float SensorWidth;     // 传感器宽度 mm
	float SensorHeight;    // 传感器高度 mm
	float Aperture;        // 光圈 f/stop
	float FocusDistance;   // 对焦距离 cm

	TArray<FVector> ImagingPlaneCorners; // “局部坐标”下的成像平面四顶点 cm
	
	FCineCameraData()
		: Name(TEXT(""))
		, Label(TEXT(""))
		, Location(FVector::ZeroVector)
		, Rotation(FRotator::ZeroRotator)
		, Scale(FVector::OneVector)
		, WorldMatrix(FMatrix::Identity)
		, FocalLength(0.f)
		, SensorWidth(0.f)
		, SensorHeight(0.f)
		, Aperture(0.f)
		, FocusDistance(0.f)
	{}

};

// StaticMesh
struct FStaticMeshData
{
	FName Name; 
	FString Label; 
	FVector Location;  // StaticMeshActor 世界坐标 cm
	FRotator Rotation;
	FVector Scale;
	FMatrix WorldMatrix; 
	TArray<FVector3f> LocalVertices;  // StaticMesh 局域顶点坐标 cm

	FStaticMeshData()
		: Name(TEXT(""))
		, Label(TEXT(""))
		, Location(FVector::ZeroVector)
		, Rotation(FRotator::ZeroRotator)
		, Scale(FVector::OneVector)
		, WorldMatrix(FMatrix::Identity)
		, LocalVertices(TArray<FVector3f>())
	{
	}
};

struct FMeshBoundsData
{
	FName Name;
	FString Label;

	// 标准化坐标范围
	float NormXMin;
	float NormXMax;
	float NormYMin;
	float NormYMax;

	// 构造函数
	FMeshBoundsData(const FName InName, const FString& InLabel,
			 const float InNormXMin, const float InNormXMax,
			 const float InNormYMin, const float InNormYMax)
	: Name(InName)
	, Label(InLabel)
	, NormXMin(InNormXMin)
	, NormXMax(InNormXMax)
	, NormYMin(InNormYMin)
	, NormYMax(InNormYMax)
	{}
};

// 像素边界结果
struct FPixelBoundResult
{
	FName MeshName;
	FString MeshLabel;

	float XMinPixel;
	float XMaxPixel;
	float YMinPixel;
	float YMaxPixel;
	
	FPixelBoundResult() = default;
};
// 图片元数据
struct FImageMetadata
{
	FString Path;  // 图片完整路径
		
	FString Label;  // 图片显示标签，用于UI展示
	int32 Width;  // 像素 x
	int32 Height;  // 像素 y
	TArray<FPixelBoundResult> PixelBoundsArray;  // 多个像素坐标边界

	FImageMetadata(const FString& InPath, const FString& InLabel, const int32 InWidth, const int32 InHeight)
		: Path(InPath)
		, Label(InLabel)
		, Width(InWidth)
		, Height(InHeight)
	{}
};