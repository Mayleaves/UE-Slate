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