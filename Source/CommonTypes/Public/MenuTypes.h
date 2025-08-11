#pragma once

#include "CoreMinimal.h"

// 菜单项 - 唯一标识和显示名称
struct FMenuItem
{
	FName Name;         // 唯一标识（方便反查、传递）
	FString Label;      // 显示名称：Camera / Folder
	int32 FolderAlias;  // Folder 别名（数字），用于 txt 存储
	// 可扩展其他字段...

	// 默认构造函数
	FMenuItem() = default;
	// 带参构造函数
	FMenuItem(const FName InName, const FString& InLabel)
		: Name(InName), Label(InLabel), FolderAlias(-1) {}
	FMenuItem(const FName InName, const FString& InLabel, const int32 InFolderAlias)
		: Name(InName), Label(InLabel), FolderAlias(InFolderAlias) {}
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
struct FMeshBaseData
{
	FName Name;       // 唯一标识
	FString Label;    // 显示标签

	FMeshBaseData() : Name(TEXT("")), Label(TEXT("")) {}
	FMeshBaseData(const FName InName, const FString& InLabel) : Name(InName), Label(InLabel) {}
};

// StaticMesh 原始数据
struct FStaticMeshData : FMeshBaseData
{
	FVector Location;                 // StaticMeshActor 世界坐标 cm
	FRotator Rotation;
	FVector Scale;
	FMatrix WorldMatrix; 
	TArray<FVector3f> LocalVertices;  // StaticMesh 局域顶点坐标 cm

	FStaticMeshData()
		: Location(FVector::ZeroVector)
		, Rotation(FRotator::ZeroRotator)
		, Scale(FVector::OneVector)
		, WorldMatrix(FMatrix::Identity)
		, LocalVertices(TArray<FVector3f>())
	{}
};

struct FMeshBoundsData : FMeshBaseData
{
	// 归一化坐标 cm
	float NormXMin;
	float NormXMax;
	float NormYMin;
	float NormYMax;

	// 构造函数
	FMeshBoundsData(const FName InName, const FString& InLabel,
				   const float InNormXMin, const float InNormXMax,
				   const float InNormYMin, const float InNormYMax)
		: FMeshBaseData(InName, InLabel)
		, NormXMin(InNormXMin), NormXMax(InNormXMax)
		, NormYMin(InNormYMin), NormYMax(InNormYMax)
	{}
};

// 像素边界结果
struct FPixelBoundResult : FMeshBaseData
{
	// 框边界坐标 px
	float XMinPixel;
	float XMaxPixel;
	float YMinPixel;
	float YMaxPixel;

	FString CategoryLabel;    // Folder 标签：用于框上显示
	int32 CategoryAlias;      // Folder 别名（数字）：存于 txt
	float CenterX;            // 框中心 X
	float CenterY;            // 框中心 Y
	float BoxWidth;           // 框宽
	float BoxHeight;          // 框高
	
	FPixelBoundResult() = default;
};
// 图片元数据
struct FImageMetadata
{
	FString Name;                                // 图片完整路径
	FString Label;                               // 图片显示标签，用于 UI 展示
	
	int32 Width;                                 // 图宽 px
	int32 Height;                                // 图高 px
	TArray<FPixelBoundResult> PixelBoundsArray;  // 多个像素坐标边界

	FImageMetadata(const FString& InName, const FString& InLabel, const int32 InWidth, const int32 InHeight)
		: Name(InName)
		, Label(InLabel)
		, Width(InWidth)
		, Height(InHeight)
	{}
};