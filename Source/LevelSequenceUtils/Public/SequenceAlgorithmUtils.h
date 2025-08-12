#pragma once

#include "CoreMinimal.h"
#include "CineCameraActor.h"
#include "IImageWrapper.h"
#include "MenuTypes.h"
#include "opencv2/core/types.hpp"

class LEVELSEQUENCEUTILS_API FSequenceAlgorithmUtils
{
public:
	// 收集图片元数据
	static void CollectImageMetadata(const FString& SourceDirectoryPath, TArray<FImageMetadata>& OutImageMetadataArray);

	void DoLabelingForFrame(
		const FName& CameraMenuRadioButton,
		const TArray<FMenuItem>& CheckedFolders,
		FImageMetadata& ImageMeta,
		const FString& SavePath);

private:
	void CollectCheckedCineCameraData(const FName& CameraName, FCineCameraData& OutCameraData);
	static void CollectCheckedStaticMeshData(
		const TArray<FMenuItem>& FolderMenuItems,
		TArray<FStaticMeshData>& OutMeshArray,
		TMap<FName, FMenuItem>& OutMeshFolderMap);
	// 将 Mesh 顶点投影到成像平面
	void ProjectMeshVerticesToCameraPlane(const FCineCameraData& CameraData, const TArray<FStaticMeshData>& MeshArray,
	                                      TMap<FName, FMeshBoundsData>& MeshBoundsMap) const;

	float MMToCm = 0.1f;
	float HalfW = 0, HalfH = 0, F = 0; // 成像平面物理尺寸
	void ExtractCineCameraData(const ACineCameraActor* CameraActor, FCineCameraData& OutCameraData);

	// 根据文件路径确定图像格式
	static EImageFormat GetImageFormat(const FString& FilePath, const TMap<FString, EImageFormat>& FormatMap);

	// 计算像素边界
	static void CalculateMeshPixelBounds(
		FImageMetadata& ImageMeta,
		const TMap<FName, FMeshBoundsData>& MeshBoundsMap,
		const TMap<FName, FMenuItem>& MeshFolderMap);

	// 创建目录
	static void EnsureDirectoriesExist(const FString& ImagesDir, const FString& ReportImagesDir);
	// 复制原图到 IMAGEs
	static void CopyOriginalImageToImagesDir(const FString& SrcImagePath, const FString& ImagesDir);
	// 类别名转颜色（可自定义更多类别和颜色）
	static cv::Scalar GetColorForCategory(const FName& CategoryName);
	// 保存单张图片坐标到txt
	static void SavePixelBoundsToTxt(const FString& SaveDirectoryPath, const FImageMetadata& ImageMeta);
	// 保存处理后的带框图片
	static void SavePixelBoundsImage(const FString& SaveDirectoryPath, const FImageMetadata& ImageMeta);
};
