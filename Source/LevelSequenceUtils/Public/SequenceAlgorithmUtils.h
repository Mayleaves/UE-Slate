#pragma once

#include "CoreMinimal.h"
#include "CineCameraActor.h"
#include "IImageWrapper.h"
#include "MenuTypes.h"

class LEVELSEQUENCEUTILS_API FSequenceAlgorithmUtils
{
public:
	void CollectCheckedCineCameraData(const FName& CameraName, FCineCameraData& OutCameraData);
	static void CollectCheckedStaticMeshData(const TArray<FName>& FolderNames, TArray<FStaticMeshData>& OutMeshArray);
	void ProjectMeshVerticesToCameraPlane(const FCineCameraData& CameraData, const TArray<FStaticMeshData>& MeshArray);

	void CollectCameraSequenceFrameMetadata(const FString& SourceDirectoryPath, const FString& SaveDirectoryPath) const;
	

private:
	float MMToCm = 0.1f;
	float HalfW = 0, HalfH = 0, F = 0;  // 成像平面物理尺寸
	void ExtractCineCameraData(const ACineCameraActor* CameraActor, FCineCameraData& OutCameraData);

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
	// 存储 Mesh 的边界数据
	TMap<FName, FMeshBoundsData> MeshBoundsMap;

	// 根据文件路径确定图像格式
	static EImageFormat GetImageFormat(const FString& FilePath, const TMap<FString, EImageFormat>& FormatMap);

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
    // 收集图片元数据
	static void CollectImageMetadata(const FString& SourceDirectoryPath, TArray<FImageMetadata>& OutImageMetadataArray);
	// 计算像素边界
	static void CalculateMeshPixelBounds(FImageMetadata& ImageMeta, const TMap<FName, FMeshBoundsData>& MeshBoundsMap);
	// 保存单张图片坐标到txt
	static void SavePixelBoundsToTxt(const FString& SaveDirectoryPath, const FImageMetadata& ImageMeta);
};