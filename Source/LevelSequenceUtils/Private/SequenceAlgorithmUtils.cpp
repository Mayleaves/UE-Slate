#include "SequenceAlgorithmUtils.h"
#include "MenuTypes.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "CineCameraActor.h"
#include "Engine/StaticMeshActor.h"
#include "Editor.h"
#include "Misc/OutputDevice.h"
#include "CineCameraComponent.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/core.hpp"

void FSequenceAlgorithmUtils::CollectCheckedCineCameraData(const FName& CameraName, FCineCameraData& OutCameraData)
{
	OutCameraData = FCineCameraData(); // 初始化
	const UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World) return;

	for (TActorIterator<ACineCameraActor> It(World); It; ++It)
	{
		if (const ACineCameraActor* CameraActor = *It; CameraActor && !CameraActor->IsPendingKillPending() &&
			CameraActor->GetFName() == CameraName)
		{
			OutCameraData.Name = FName(CameraActor->GetName());
			OutCameraData.Label = CameraActor->GetActorLabel();
			OutCameraData.Location = CameraActor->GetActorLocation();
			OutCameraData.Rotation = CameraActor->GetActorRotation();
			OutCameraData.Scale = CameraActor->GetActorScale3D();
			OutCameraData.WorldMatrix = CameraActor->GetActorTransform().ToMatrixWithScale();
			ExtractCineCameraData(CameraActor, OutCameraData); // 获取物理参数、成像平面四顶点

			UE_LOG(LogTemp, Log, TEXT("CineCameraActor Name: %s, Label: %s"), *OutCameraData.Name.ToString(),
			       *OutCameraData.Label);
			UE_LOG(LogTemp, Log, TEXT("    Location: (%.2f, %.2f, %.2f)"), OutCameraData.Location.X,
			       OutCameraData.Location.Y, OutCameraData.Location.Z);
			UE_LOG(LogTemp, Log, TEXT("    Rotation: (%.2f, %.2f, %.2f)"), OutCameraData.Rotation.Roll,
			       OutCameraData.Rotation.Pitch, OutCameraData.Rotation.Yaw);
			UE_LOG(LogTemp, Log, TEXT("    Scale:    (%.2f, %.2f, %.2f)"), OutCameraData.Scale.X, OutCameraData.Scale.Y,
			       OutCameraData.Scale.Z);
			UE_LOG(LogTemp, Log, TEXT("    Matrix:   %s"), *OutCameraData.WorldMatrix.ToString());
			// for (int i = 0; i < OutCameraData.ImagingPlaneCorners.Num(); ++i)
			// {
			//     const FVector& Corner = OutCameraData.ImagingPlaneCorners[i];
			//     FString CornerName;
			//     switch (i)
			//     {
			//     case 0: CornerName = "TopLeft"; break;
			//     case 1: CornerName = "TopRight"; break;
			//     case 2: CornerName = "BottomRight"; break;
			//     case 3: CornerName = "BottomLeft"; break;
			//     default: CornerName = "Unknown"; break;
			//     }
			//     UE_LOG(LogTemp, Log, TEXT("    %s: 深度X=%.2fcm, 左右Y=%.2fcm, 上下Z=%.2fcm"), 
			//            *CornerName, Corner.X, Corner.Y, Corner.Z);
			// }

			// CameraName唯一，找到后就退出
			break;
		}
	}
}

void FSequenceAlgorithmUtils::ExtractCineCameraData(const ACineCameraActor* CameraActor, FCineCameraData& OutCameraData)
{
	if (!CameraActor || CameraActor->IsPendingKillPending())
	{
		return;
	}

	const UCineCameraComponent* CineComp = CameraActor->GetCineCameraComponent();
	if (!CineComp)
	{
		return;
	}

	// 提取相机物理参数
	OutCameraData.FocalLength = CineComp->CurrentFocalLength;
	OutCameraData.SensorWidth = CineComp->Filmback.SensorWidth;
	OutCameraData.SensorHeight = CineComp->Filmback.SensorHeight;
	OutCameraData.Aperture = CineComp->CurrentAperture;
	OutCameraData.FocusDistance = CineComp->FocusSettings.ManualFocusDistance;

	// 计算成像平面四个角的局部坐标：mm → cm
	HalfW = OutCameraData.SensorWidth * 0.5f * MMToCm;
	HalfH = OutCameraData.SensorHeight * 0.5f * MMToCm;
	F = OutCameraData.FocalLength * MMToCm; // 深度：正方向 = 摄像机前方

	const FVector LocalCorners[4] = {
		// YZX 坐标系（横竖深）
		FVector(F, -HalfW, HalfH), // TopLeft
		FVector(F, HalfW, HalfH), // TopRight
		FVector(F, HalfW, -HalfH), // BottomRight
		FVector(F, -HalfW, -HalfH) // BottomLeft
	};

	OutCameraData.ImagingPlaneCorners.Empty();
	for (int i = 0; i < 4; ++i)
	{
		OutCameraData.ImagingPlaneCorners.Add(LocalCorners[i]);
	}
}

void FSequenceAlgorithmUtils::CollectCheckedStaticMeshData(
	const TArray<FMenuItem>& FolderMenuItems,
	TArray<FStaticMeshData>& OutMeshArray,
	TMap<FName, FMenuItem>& OutMeshFolderMap)
{
	OutMeshArray.Empty(); // 清空输出数组
	const UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World) return;

	for (const FMenuItem& FolderMenuItem : FolderMenuItems)
	{
		const FString& FolderName = FolderMenuItem.Name.ToString();
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (const AStaticMeshActor* SMActor = Cast<AStaticMeshActor>(*It); SMActor && !SMActor->
				IsPendingKillPending() &&
				SMActor->GetFolderPath().ToString() == FolderName)
			{
				FStaticMeshData MeshData;
				MeshData.Name = FName(SMActor->GetName());
				MeshData.Label = SMActor->GetActorLabel();
				MeshData.Location = SMActor->GetActorLocation(); // StaticMeshActor 世界坐标 cm
				MeshData.Rotation = SMActor->GetActorRotation();
				MeshData.Scale = SMActor->GetActorScale3D();
				MeshData.WorldMatrix = SMActor->GetActorTransform().ToMatrixWithScale(); // todo 4×4变换矩阵
				MeshData.LocalVertices.Empty();

				// 获取 StaticMesh 局域顶点坐标
				if (const UStaticMeshComponent* MeshComp = SMActor->GetStaticMeshComponent())
				{
					if (UStaticMesh* StaticMesh = MeshComp->GetStaticMesh())
					{
						// 渲染顶点（PositionBuffer）
						if (StaticMesh->GetRenderData() && StaticMesh->GetRenderData()->LODResources.Num() > 0)
						{
							const FStaticMeshLODResources& LODResource = StaticMesh->GetRenderData()->LODResources[0];
							const FPositionVertexBuffer& PositionBuffer = LODResource.VertexBuffers.
								PositionVertexBuffer;
							const int32 NumRenderVertices = PositionBuffer.GetNumVertices();

							for (int32 i = 0; i < NumRenderVertices; ++i)
							{
								MeshData.LocalVertices.Add(PositionBuffer.VertexPosition(i));
							}
						}
					}
				}
				OutMeshArray.Add(MeshData);
				OutMeshFolderMap.Add(
					FName(SMActor->GetName()),
					FMenuItem(FolderMenuItem.Name, FolderMenuItem.Label, FolderMenuItem.FolderAlias)
				);
			}
		}
	}
	// 按 Label 排序
	OutMeshArray.Sort([](const FStaticMeshData& A, const FStaticMeshData& B)
	{
		return A.Label < B.Label;
	});
	for (const auto& MeshData : OutMeshArray)
	{
		UE_LOG(LogTemp, Log, TEXT("StaticMeshActor Name: %s, Label: %s"), *MeshData.Name.ToString(), *MeshData.Label);
		UE_LOG(LogTemp, Log, TEXT("    Location: (%.2f, %.2f, %.2f)"), MeshData.Location.X, MeshData.Location.Y,
		       MeshData.Location.Z);
		UE_LOG(LogTemp, Log, TEXT("    Rotation: (%.2f, %.2f, %.2f)"), MeshData.Rotation.Roll, MeshData.Rotation.Pitch,
		       MeshData.Rotation.Yaw);
		UE_LOG(LogTemp, Log, TEXT("    Scale:    (%.2f, %.2f, %.2f)"), MeshData.Scale.X, MeshData.Scale.Y,
		       MeshData.Scale.Z);
		UE_LOG(LogTemp, Log, TEXT("    Matrix:   %s"), *MeshData.WorldMatrix.ToString());
		// constexpr int32 MaxPrintCount = 5;
		// for (int32 i = 0; i < FMath::Min(MeshData.LocalVertices.Num(), MaxPrintCount); ++i)
		// {
		//     const FVector3f& LocalVertexF = MeshData.LocalVertices[i];
		//     const FVector Vertex((double)LocalVertexF.X, (double)LocalVertexF.Y, (double)LocalVertexF.Z);
		//
		//     UE_LOG(LogTemp, Log, TEXT("    LocalVertex[%d]: (%.2f, %.2f, %.2f)"), i, Vertex.X, Vertex.Y, Vertex.Z);
		// }
		// UE_LOG(LogTemp, Log, TEXT("    Local Vertex Total Count: %d"), MeshData.LocalVertices.Num());
	}
}

void FSequenceAlgorithmUtils::ProjectMeshVerticesToCameraPlane(
	const FCineCameraData& CameraData,
	const TArray<FStaticMeshData>& MeshArray,
	TMap<FName, FMeshBoundsData>& MeshBoundsMap) const
{
	// 清空旧的 MeshBoundsMap，确保每帧只存当前帧数据
	MeshBoundsMap.Empty();

	// 相机世界变换
	const FTransform CamTransform(CameraData.Rotation, CameraData.Location);
	const FTransform CamInvTransform = CamTransform.Inverse();

	for (const FStaticMeshData& MeshData : MeshArray)
	{
		UE_LOG(LogTemp, Log, TEXT("==== Camera: %s 与 Mesh: %s ===="), *CameraData.Label, *MeshData.Label);

		float LocalNormXMin = FLT_MAX;
		float LocalNormXMax = -FLT_MAX;
		float LocalNormYMin = FLT_MAX;
		float LocalNormYMax = -FLT_MAX;

		for (int32 i = 0; i < MeshData.LocalVertices.Num(); ++i)
		{
			// 1：StaticMesh 坐标变换（cm）
			FVector LocalVertex = FVector(MeshData.LocalVertices[i]);
			FVector WorldPos = MeshData.WorldMatrix.TransformPosition(LocalVertex);
			const FVector VertexCameraSpace = CamInvTransform.TransformPosition(WorldPos);

			UE_LOG(LogTemp, Log,
			       TEXT("   顶点[%d]: 局域坐标(%.2f, %.2f, %.2f) → 世界坐标(%.2f, %.2f, %.2f) → 相机坐标(%.2f, %.2f, %.2f)"),
			       i,
			       LocalVertex.X, LocalVertex.Y, LocalVertex.Z,
			       WorldPos.X, WorldPos.Y, WorldPos.Z,
			       VertexCameraSpace.X, VertexCameraSpace.Y, VertexCameraSpace.Z);

			// 2：投影变化（相似三角形计算）cm
			if (VertexCameraSpace.X > 0.f) // Camera 局域坐标系：X > 0 表示在相机前方
			{
				// 成像平面坐标 cm
				const float ImagePlaneX = F * (VertexCameraSpace.Y / VertexCameraSpace.X);
				const float ImagePlaneY = F * (VertexCameraSpace.Z / VertexCameraSpace.X);

				// 图片默认坐标系 cm
				const float ImageX = ImagePlaneX + HalfW;
				const float ImageY = HalfH - ImagePlaneY;

				// 归一化（0-1范围）cm
				const float NormX = ImageX / (2.0f * HalfW);
				const float NormY = ImageY / (2.0f * HalfH);

				UE_LOG(LogTemp, Log, TEXT("           成像平面坐标(%.2f, %.2f) → 图片默认坐标系(%.2f, %.2f) → 归一化坐标(%.2f, %.2f)"),
				       ImagePlaneX, ImagePlaneY, ImageX, ImageY, NormX, NormY);

				LocalNormXMin = FMath::Min(LocalNormXMin, NormX);
				LocalNormXMax = FMath::Max(LocalNormXMax, NormX);
				LocalNormYMin = FMath::Min(LocalNormYMin, NormY);
				LocalNormYMax = FMath::Max(LocalNormYMax, NormY);

				// todo 可选：调试画出投影点到图片上
				// 可将所有 (NormX, NormY) 保存下来用于后续可视化
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("           [被剔除：X <= 0]"));
			}
		}

		// Step 1: 判断是否完全越界（均小于0或均大于1直接舍弃）
		if (LocalNormXMax < 0.f || LocalNormXMin > 1.f || LocalNormYMax < 0.f || LocalNormYMin > 1.f)
		{
			UE_LOG(LogTemp, Warning, TEXT("   %s Mesh 超出成像平面，舍弃。"), *MeshData.Label);
			continue; // 舍弃本 mesh
		}

		// Step 2: 修正边界到 [0, 1] 范围
		const float ClampedNormXMin = FMath::Clamp(LocalNormXMin, 0.f, 1.f);
		const float ClampedNormXMax = FMath::Clamp(LocalNormXMax, 0.f, 1.f);
		const float ClampedNormYMin = FMath::Clamp(LocalNormYMin, 0.f, 1.f);
		const float ClampedNormYMax = FMath::Clamp(LocalNormYMax, 0.f, 1.f);

		// Step 3: 存入 MeshBoundsMap
		MeshBoundsMap.Add(
			MeshData.Name,
			FMeshBoundsData(
				MeshData.Name,
				MeshData.Label,
				ClampedNormXMin,
				ClampedNormXMax,
				ClampedNormYMin,
				ClampedNormYMax
			)
		);
		UE_LOG(LogTemp, Log, TEXT("   边界统计: %s NormXMin=%.3f, NormXMax=%.3f, NormYMin=%.3f, NormYMax=%.3f"),
		       *MeshData.Label, ClampedNormXMin, ClampedNormXMax, ClampedNormYMin, ClampedNormYMax);
	}
}

EImageFormat FSequenceAlgorithmUtils::GetImageFormat(const FString& FilePath,
                                                     const TMap<FString, EImageFormat>& FormatMap)
{
	if (const FString Ext = FPaths::GetExtension(FilePath).ToLower(); FormatMap.Contains(Ext))
		return FormatMap[Ext];
	return EImageFormat::Invalid;
}

void FSequenceAlgorithmUtils::CollectImageMetadata(const FString& SourceDirectoryPath,
                                                   TArray<FImageMetadata>& OutImageMetadataArray)
{
	// 提取为静态常量，避免每次调用都创建
	static const TArray<FString> ImageExtensions = {
		TEXT(".jpg"), TEXT(".jpeg"), TEXT(".png"),
		TEXT(".bmp"), TEXT(".tga"), TEXT(".exr")
	};
	static const TMap<FString, EImageFormat> ExtensionToFormatMap = {
		{TEXT("jpg"), EImageFormat::JPEG},
		{TEXT("jpeg"), EImageFormat::JPEG},
		{TEXT("png"), EImageFormat::PNG},
		{TEXT("bmp"), EImageFormat::BMP},
		{TEXT("exr"), EImageFormat::EXR},
		{TEXT("tga"), EImageFormat::TGA}
	};

	// 获取路径下的所有文件
	TArray<FString> FoundFiles;
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.FindFilesRecursively(FoundFiles, *SourceDirectoryPath, nullptr);

	// 过滤出图片文件
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(
		FName("ImageWrapper"));

	for (const FString& FilePath : FoundFiles)
	{
		// 检查文件扩展名
		if (FString LowercaseExt = FPaths::GetExtension(FilePath, true).ToLower(); !ImageExtensions.Contains(
			LowercaseExt))
			continue;

		// FString FileName = FPaths::GetCleanFilename(FilePath);  // image001.png
		FString FileNameWithoutExt = FPaths::GetBaseFilename(FilePath); // image001

		// 读取图片尺寸
		int32 Width = 0, Height = 0;
		if (const auto ImageFormat = GetImageFormat(FilePath, ExtensionToFormatMap); ImageFormat !=
			EImageFormat::Invalid)
		{
			if (TArray<uint8> RawFileData; FFileHelper::LoadFileToArray(RawFileData, *FilePath))
			{
				if (auto ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);
					ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
				{
					Width = ImageWrapper->GetWidth();
					Height = ImageWrapper->GetHeight();
				}
			}
		}
		OutImageMetadataArray.Add(FImageMetadata(FilePath, FileNameWithoutExt, Width, Height));
	}
}

void FSequenceAlgorithmUtils::CalculateMeshPixelBounds(
	FImageMetadata& ImageMeta,
	const TMap<FName, FMeshBoundsData>& MeshBoundsMap,
	const TMap<FName, FMenuItem>& MeshFolderMap)
{
	for (const TPair<FName, FMeshBoundsData>& MeshPair : MeshBoundsMap)
	{
		const FMeshBoundsData& MeshData = MeshPair.Value;
		const float XRange = MeshData.NormXMax - MeshData.NormXMin;
		const float YRange = MeshData.NormYMax - MeshData.NormYMin;
		const float XThreshold = ImageMeta.Width * 0.00001f; // 0.001% 的阈值
		const float YThreshold = ImageMeta.Height * 0.00001f;

		if (XRange >= XThreshold && YRange >= YThreshold)
		{
			FPixelBoundResult PixelBound;
			PixelBound.Name = MeshPair.Key;
			PixelBound.Label = MeshData.Label;
			PixelBound.XMinPixel = MeshData.NormXMin * ImageMeta.Width;
			PixelBound.XMaxPixel = MeshData.NormXMax * ImageMeta.Width;
			PixelBound.YMinPixel = MeshData.NormYMin * ImageMeta.Height;
			PixelBound.YMaxPixel = MeshData.NormYMax * ImageMeta.Height;
			// 为 Mesh 补充 Folder 信息
			if (const FMenuItem* FolderInfo = MeshFolderMap.Find(MeshPair.Key))
			{
				PixelBound.CategoryName = FolderInfo->Name;
				PixelBound.CategoryLabel = FolderInfo->Label;
				PixelBound.CategoryAlias = FolderInfo->FolderAlias;
			}
			else
			{
				PixelBound.CategoryName = NAME_None;
				PixelBound.CategoryLabel = TEXT("");
				PixelBound.CategoryAlias = -1;
			}
			PixelBound.CenterX = (PixelBound.XMinPixel + PixelBound.XMaxPixel) * 0.5f;
			PixelBound.CenterY = (PixelBound.YMinPixel + PixelBound.YMaxPixel) * 0.5f;
			PixelBound.BoxWidth = PixelBound.XMaxPixel - PixelBound.XMinPixel;
			PixelBound.BoxHeight = PixelBound.YMaxPixel - PixelBound.YMinPixel;
			ImageMeta.PixelBoundsArray.Add(PixelBound);

			UE_LOG(LogTemp, Log, TEXT("   %s Mesh: XMin=%.2f, XMax=%.2f, YMin=%.2f, YMax=%.2f"),
			       *PixelBound.Label, PixelBound.XMinPixel, PixelBound.XMaxPixel, PixelBound.YMinPixel,
			       PixelBound.YMaxPixel);
		}
	}
}

void FSequenceAlgorithmUtils::EnsureDirectoriesExist(const FString& ImagesDir, const FString& ReportImagesDir)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*ImagesDir))
	{
		PlatformFile.CreateDirectoryTree(*ImagesDir);
	}
	if (!PlatformFile.DirectoryExists(*ReportImagesDir))
	{
		PlatformFile.CreateDirectoryTree(*ReportImagesDir);
	}
}

void FSequenceAlgorithmUtils::CopyOriginalImageToImagesDir(const FString& SrcImagePath, const FString& ImagesDir)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FString ImageFileName = FPaths::GetCleanFilename(SrcImagePath);
	FString DstImagePath = FPaths::Combine(ImagesDir, ImageFileName);
	if (!PlatformFile.FileExists(*DstImagePath))
	{
		PlatformFile.CopyFile(*DstImagePath, *SrcImagePath);
	}
}

void FSequenceAlgorithmUtils::SavePixelBoundsToTxt(const FString& SaveDirectoryPath, const FImageMetadata& ImageMeta)
{
	// 确保目录存在
	if (IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile(); !PlatformFile.DirectoryExists(
		*SaveDirectoryPath))
	{
		PlatformFile.CreateDirectoryTree(*SaveDirectoryPath);
	}

	TArray<FString> OutputLines;
	for (const FPixelBoundResult& PixelBound : ImageMeta.PixelBoundsArray)
	{
		// 写入文本行：CategoryAlias, CenterX, CenterY, BoxWidth, BoxHeight, MeshLabel
		FString Line = FString::Printf(TEXT("%d, %.2f, %.2f, %.2f, %.2f, %s"),
	       PixelBound.CategoryAlias,
	       PixelBound.CenterX,
	       PixelBound.CenterY,
	       PixelBound.BoxWidth,
	       PixelBound.BoxHeight,
	       *PixelBound.Label);
		OutputLines.Add(Line);
	}

	// 拼接文件名: SaveDirectoryPath/image001.txt
	const FString SaveFilePath = FPaths::Combine(SaveDirectoryPath, ImageMeta.Label + TEXT(".txt"));
	FFileHelper::SaveStringArrayToFile(OutputLines, *SaveFilePath);
}

cv::Scalar FSequenceAlgorithmUtils::GetColorForCategory(const FName& CategoryName)
{
	// 匹配到 Key，返回对应颜色
	static TMap<FName, cv::Scalar> ColorMap = {
		{FName("Atest"), cv::Scalar(255, 0, 0)}, // 蓝色
		{FName("Atest2"), cv::Scalar(0, 255, 0)}, // 绿色
		{FName("Atest3"), cv::Scalar(0, 0, 255)}, // 红色
		// 其他默认类别颜色
	};
	if (const cv::Scalar* FoundColor = ColorMap.Find(CategoryName))
		return *FoundColor;

	// 若未找到 Key，生成 hash 色
	const uint32 Hash = GetTypeHash(CategoryName);
	return cv::Scalar((Hash >> 0) & 255, (Hash >> 8) & 255, (Hash >> 16) & 255);
}

void FSequenceAlgorithmUtils::SavePixelBoundsImage(const FString& SaveDirectoryPath, const FImageMetadata& ImageMeta)
{
	const FString ImagePath = ImageMeta.Name;
	const FString ImageName = ImageMeta.Label;
	const FString BoxedImagePath = FPaths::Combine(SaveDirectoryPath, ImageName + TEXT("_boxed.png"));

	// 加载图片
	cv::Mat Img = cv::imread(TCHAR_TO_UTF8(*ImagePath));
	if (Img.empty()) return;

	for (const FPixelBoundResult& PixelBound : ImageMeta.PixelBoundsArray)
	{
		const int Xmin = FMath::RoundToInt(PixelBound.XMinPixel);
		const int Xmax = FMath::RoundToInt(PixelBound.XMaxPixel);
		const int Ymin = FMath::RoundToInt(PixelBound.YMinPixel);
		const int Ymax = FMath::RoundToInt(PixelBound.YMaxPixel);

		// 1. 根据类别获取颜色
		cv::Scalar BoxColor = GetColorForCategory(PixelBound.CategoryName);

		// 2. 绘制主大框
		cv::rectangle(
			Img,
			cv::Point(Xmin, Ymin),
			cv::Point(Xmax, Ymax),
			BoxColor,
			2
		);

		// 3. 左上角小矩形+类别文字
		constexpr int FontFace = cv::FONT_HERSHEY_SIMPLEX;
		constexpr double FontScale = 0.7;
		constexpr int Thickness = 2;
		int Baseline = 0;
		std::string LabelStr = TCHAR_TO_UTF8(*PixelBound.CategoryLabel);

		// 获取文字尺寸
		const cv::Size TextSize = cv::getTextSize(LabelStr, FontFace, FontScale, Thickness, &Baseline);
		const int RectWidth = TextSize.width + 8;
		const int RectHeight = TextSize.height + 8;

		// 绘制背景小矩形
		cv::rectangle(
			Img,
			cv::Point(Xmin, Ymin),
			cv::Point(Xmin + RectWidth, Ymin + RectHeight),
			BoxColor,
			cv::FILLED
		);

		// 文字颜色可自适应（简单示例：深色框用白字）
		const cv::Scalar TextColor = (BoxColor[0] + BoxColor[1] + BoxColor[2] > 400)
                 ? cv::Scalar(0, 0, 0)
                 : cv::Scalar(255, 255, 255);

		// 写文字
		cv::putText(
			Img,
			LabelStr,
			cv::Point(Xmin + 4, Ymin + RectHeight - 4), // 左下角起点
			FontFace,
			FontScale,
			TextColor,
			2
		);
	}

	// 保存处理后的图片
	cv::imwrite(TCHAR_TO_UTF8(*BoxedImagePath), Img);
}

void FSequenceAlgorithmUtils::DoLabelingForFrame(
	const FName& CameraMenuRadioButton,
	const TArray<FMenuItem>& CheckedFolders,
	FImageMetadata& ImageMeta,
	const FString& SavePath)
{
	FCineCameraData CameraData;
	TArray<FStaticMeshData> MeshArray;
	// 存储 Mesh 对应于的 Folder 信息
	TMap<FName, FMenuItem> MeshFolderMap;
	// 存储 Mesh 的边界数据
	TMap<FName, FMeshBoundsData> MeshBoundsMap;
	// 标注前、标注后
	const FString ImagesDir = FPaths::Combine(SavePath, TEXT("IMAGEs"));
	const FString ReportImagesDir = FPaths::Combine(SavePath, TEXT("REPORT"), TEXT("IMAGEs"));

	CollectCheckedCineCameraData(CameraMenuRadioButton, CameraData);
	CollectCheckedStaticMeshData(CheckedFolders, MeshArray, MeshFolderMap);
	ProjectMeshVerticesToCameraPlane(CameraData, MeshArray, MeshBoundsMap);

    UE_LOG(LogTemp, Log, TEXT("[图片: %s] 宽=%d, 高=%d"), *ImageMeta.Label, ImageMeta.Width, ImageMeta.Height);
    CalculateMeshPixelBounds(ImageMeta, MeshBoundsMap, MeshFolderMap);

	EnsureDirectoriesExist(ImagesDir, ReportImagesDir);
	CopyOriginalImageToImagesDir(ImageMeta.Name, ImagesDir);
    SavePixelBoundsToTxt(ReportImagesDir, ImageMeta);
    SavePixelBoundsImage(ReportImagesDir, ImageMeta);
}
