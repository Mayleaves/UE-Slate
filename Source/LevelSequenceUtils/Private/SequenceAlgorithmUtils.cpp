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

			// CameraName 唯一，找到后就退出
			break;
		}
	}
}

void FSequenceAlgorithmUtils::ExtractCineCameraData(const ACineCameraActor* CameraActor, FCineCameraData& OutCameraData)
{
	if (!CameraActor || CameraActor->IsPendingKillPending()) return;

	const UCineCameraComponent* CineComp = CameraActor->GetCineCameraComponent();
	if (!CineComp) return;

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
			if (const AStaticMeshActor* SMActor = Cast<AStaticMeshActor>(*It);
				SMActor && !SMActor->IsPendingKillPending() &&
				SMActor->GetFolderPath().ToString() == FolderName)
			{
				FStaticMeshData MeshData;
				MeshData.Name = FName(SMActor->GetName());
				MeshData.Label = SMActor->GetActorLabel();
				MeshData.Location = SMActor->GetActorLocation(); // StaticMeshActor 世界坐标 cm
				MeshData.Rotation = SMActor->GetActorRotation();
				MeshData.Scale = SMActor->GetActorScale3D();
				MeshData.WorldMatrix = SMActor->GetActorTransform().ToMatrixWithScale(); // 4×4变换矩阵
				MeshData.LocalVertices.Empty();
				MeshData.FaceSamplePoints.Empty();
				MeshData.ActorPointer = const_cast<AStaticMeshActor*>(SMActor);

				// 获取 StaticMesh 局域坐标
				if (const UStaticMeshComponent* MeshComp = SMActor->GetStaticMeshComponent())
				{
					if (UStaticMesh* StaticMesh = MeshComp->GetStaticMesh())
					{
						// 渲染顶点（PositionBuffer）
						if (StaticMesh->GetRenderData() && StaticMesh->GetRenderData()->LODResources.Num() > 0)
						{
							const FStaticMeshLODResources& LODResource = StaticMesh->GetRenderData()->LODResources[0];
							const FPositionVertexBuffer& PositionBuffer = LODResource.VertexBuffers.PositionVertexBuffer;
							const int32 NumRenderVertices = PositionBuffer.GetNumVertices();
							
							// 1. 顶点坐标
							for (int32 i = 0; i < NumRenderVertices; ++i)
							{
								MeshData.LocalVertices.Add(FVector(PositionBuffer.VertexPosition(i)));
							}
							
							// 2. 三角面重心均匀采样（面内采样点）
							TArray<uint32> Indices;
							LODResource.IndexBuffer.GetCopy(Indices);
							
							if (Indices.Num() >= 3)
							{
								for (int32 TriIdx = 0; TriIdx + 2 < Indices.Num(); TriIdx += 3)
								{
									constexpr int SampleN = 10;  // 密度，越高面内采样点越多
									const FVector V0 = FVector(PositionBuffer.VertexPosition(Indices[TriIdx]));
									const FVector V1 = FVector(PositionBuffer.VertexPosition(Indices[TriIdx + 1]));
									const FVector V2 = FVector(PositionBuffer.VertexPosition(Indices[TriIdx + 2]));

									for (int i = 0; i <= SampleN; ++i)
									{
										for (int j = 0; j <= SampleN - i; ++j)
										{
											int k = SampleN - i - j;
											float a = static_cast<float>(i) / SampleN;
											float b = static_cast<float>(j) / SampleN;
											float c = static_cast<float>(k) / SampleN;
											FVector P = a * V0 + b * V1 + c * V2;
											MeshData.FaceSamplePoints.Add(P);
										}
									}
								}
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
		// // 打印局域顶点
		// UE_LOG(LogTemp, Log, TEXT("    Local Vertex Total Count: %d"), MeshData.LocalVertices.Num());
		// for (int32 i = 0; i < FMath::Min(MeshData.LocalVertices.Num(), MaxPrintCount); ++i)
		// {
		// 	const FVector& LocalVertex = MeshData.LocalVertices[i];
		// 	UE_LOG(LogTemp, Log, TEXT("        LocalVertex[%d]: (%.2f, %.2f, %.2f)"), i, LocalVertex.X, LocalVertex.Y, LocalVertex.Z);
		// }
		// // 打印面内采样点
		// UE_LOG(LogTemp, Log, TEXT("    Face Sample Point Total Count: %d"), MeshData.FaceSamplePoints.Num());
		// for (int32 i = 0; i < FMath::Min(MeshData.FaceSamplePoints.Num(), MaxPrintCount); ++i)
		// {
		// 	const FVector& SamplePt = MeshData.FaceSamplePoints[i];
		// 	UE_LOG(LogTemp, Log, TEXT("        FaceSamplePoint[%d]: (%.2f, %.2f, %.2f)"), i, SamplePt.X, SamplePt.Y, SamplePt.Z);
		// }
	}
}

void FSequenceAlgorithmUtils::ProjectMeshVisiblePixels(
	const FCineCameraData& CameraData,
	const TArray<FStaticMeshData>& MeshArray,
	TMap<FName, FMeshBoundsData>& MeshBoundsMap,
	const float MaximumDistance,
	const UWorld* World
) const
{
	// 清空旧的 MeshBoundsMap，确保每帧只存当前帧数据
	MeshBoundsMap.Empty();
	// 单位转换：m → cm
	const float MaxDistanceCm = MaximumDistance * 100.0f;

	// 遍历所有 Mesh
	for (const FStaticMeshData& MeshData : MeshArray)
	{
		if (!MeshData.ActorPointer) continue;

		// |SMA世界坐标 - Camera世界坐标| > 最大检测距离
		if (const float Dist = FVector::Dist(MeshData.Location, CameraData.Location); Dist > MaxDistanceCm)
		{
			UE_LOG(LogTemp, Warning, TEXT("Mesh [%s] 距离相机 %.2f m，超过最大检测距离 %.2f m，跳过"), *MeshData.Label, Dist / 100.0f, MaximumDistance);
			continue;  // 跳过本 Mesh
		}
		
 		UE_LOG(LogTemp, Log, TEXT("==== Camera: %s 与 Mesh: %s ===="), *CameraData.Label, *MeshData.Label);

		TArray<FVector2D> VisibleNormXY;
		TArray<FVector> AllSamplePoints = MeshData.LocalVertices;
		AllSamplePoints.Append(MeshData.FaceSamplePoints);
		// 可见性判定：局域顶点+面内采样点
		for (const FVector& LocalPt : AllSamplePoints)
		{
			FVector WorldPt = MeshData.WorldMatrix.TransformPosition(LocalPt);

			// SphereTrace：防止浮点误差
			constexpr float SphereRadius = 0.2f; // 越大射线越粗
			FHitResult HitResult;  // 用于存储射线检测碰撞结果
			const bool bHit = World->SweepSingleByChannel(
				HitResult,
				CameraData.Location,
				WorldPt,
				FQuat::Identity,
				ECC_Visibility,
				FCollisionShape::MakeSphere(SphereRadius)
			);
			
			// 命中的 Mesh 是当前遍历 Mesh
			if (bHit && HitResult.GetActor() == MeshData.ActorPointer)
			{
				if (FVector2D NormXY; ProjectWorldPointToImagePlane(WorldPt, CameraData, NormXY)) {
					UE_LOG(LogTemp, Log, TEXT("   [%s] 归一化投影坐标: (%.3f, %.3f)"), *MeshData.Label, NormXY.X, NormXY.Y);
					VisibleNormXY.Add(NormXY);
				} else {
					UE_LOG(LogTemp, Warning, TEXT("   [%s] 投影点被丢弃，归一化坐标超出画面: (%.3f, %.3f)"), *MeshData.Label, NormXY.X, NormXY.Y);
				}
			}
			else if (bHit) {
				UE_LOG(LogTemp, Warning, TEXT("   [%s] 采样命中其他物体: %s"), *MeshData.Label, *GetNameSafe(HitResult.GetActor()));
			}
			else { // Mesh 未添加碰撞体
				UE_LOG(LogTemp, Warning, TEXT("   [%s] 采样射线未命中任何物体"), *MeshData.Label);
			}
		}
		
		if (VisibleNormXY.Num() > 0)
		{
			// 初始化
			float XMin = TNumericLimits<float>::Max();
			float XMax = TNumericLimits<float>::Lowest();
			float YMin = TNumericLimits<float>::Max();
			float YMax = TNumericLimits<float>::Lowest();
			// 获取最大最小 X、Y
			for (const FVector2D& UV : VisibleNormXY)
			{
				XMin = FMath::Min(XMin, UV.X);
				XMax = FMath::Max(XMax, UV.X);
				YMin = FMath::Min(YMin, UV.Y);
				YMax = FMath::Max(YMax, UV.Y);
			}
			MeshBoundsMap.Add(
				MeshData.Name,
				FMeshBoundsData(MeshData.Name, MeshData.Label, XMin, XMax, YMin, YMax, VisibleNormXY)
			);
			UE_LOG(LogTemp, Log, TEXT("   [%s] 可见包围盒: (%.3f, %.3f)-(%.3f, %.3f)"), *MeshData.Label, XMin, XMax, YMin, YMax);
		}
	}
}

bool FSequenceAlgorithmUtils::ProjectWorldPointToImagePlane(const FVector& WorldPoint, const FCineCameraData& CameraData, FVector2D& OutNormXY) const
{
	// 1：StaticMesh 坐标变换（cm）
	const FTransform CamTransform(CameraData.Rotation, CameraData.Location);
	// 基于相机坐标系的 Mesh 坐标
	const FVector VertexCameraSpace = CamTransform.InverseTransformPosition(WorldPoint);
	
	// 2：投影变化（相似三角形计算）cm
	if (VertexCameraSpace.X <= 0) return false; // 位于相机后方
	// 成像平面坐标 cm
	const float ImagePlaneX = F * (VertexCameraSpace.Y / VertexCameraSpace.X);
	const float ImagePlaneY = F * (VertexCameraSpace.Z / VertexCameraSpace.X);

	// 图片默认坐标系 cm
	const float ImageX = ImagePlaneX + HalfW;
	const float ImageY = HalfH - ImagePlaneY;

	// 归一化（0-1范围）cm
	const float NormX = ImageX / (2.0f * HalfW);
	const float NormY = ImageY / (2.0f * HalfH);

	OutNormXY.X = NormX;
	OutNormXY.Y = NormY;

	// 超出成像面范围返回 false
	if (NormX < 0.f || NormX > 1.f || NormY < 0.f || NormY > 1.f)
		return false;

	return true;
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
		const float XThreshold = ImageMeta.Width * 1E-07; // 0.00001% 的阈值
		const float YThreshold = ImageMeta.Height * 1E-07;

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
			// 局域顶点、面内采样点
			PixelBound.LocalPoints = MeshData.LocalPoints; 
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
	if (const FString DstImagePath = FPaths::Combine(ImagesDir, ImageFileName); !PlatformFile.FileExists(*DstImagePath))
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
		{FName("Atest4"), cv::Scalar(0, 255, 255)}, // 黄色
		// 其他默认类别颜色...
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
		// float → int
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
		
		// // 4. 画采样点
		// for (const FVector2D& Pt : PixelBound.LocalPoints)
		// {
		// 	const int Px = FMath::RoundToInt(Pt.X * Img.cols);
		// 	const int Py = FMath::RoundToInt(Pt.Y * Img.rows);
		// 	if (Px >= 0 && Px < Img.cols && Py >= 0 && Py < Img.rows)
		// 	{
		// 		cv::circle(Img, cv::Point(Px, Py), 4, cv::Scalar(0, 0, 255), cv::FILLED);
		// 	}
		// }
	}

	// 保存处理后的图片
	cv::imwrite(TCHAR_TO_UTF8(*BoxedImagePath), Img);
}

void FSequenceAlgorithmUtils::DoLabelingForFrame(
	const FName& CameraMenuRadioButton,
	const TArray<FMenuItem>& CheckedFolders,
	float MaximumDistance,
	FImageMetadata& ImageMeta,
	const FString& SavePath)
{
	FCineCameraData CameraData;
	TArray<FStaticMeshData> MeshArray;
	// 存储 Mesh 对应于的 Folder 信息
	TMap<FName, FMenuItem> MeshFolderMap;
	// 用可见性剔除的包围盒
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	// 存储 Mesh 的边界数据
	TMap<FName, FMeshBoundsData> MeshBoundsMap;
	// 标注前、标注后
	const FString ImagesDir = FPaths::Combine(SavePath, TEXT("IMAGEs"));
	const FString ReportImagesDir = FPaths::Combine(SavePath, TEXT("REPORT"), TEXT("IMAGEs"));

	CollectCheckedCineCameraData(CameraMenuRadioButton, CameraData);
	CollectCheckedStaticMeshData(CheckedFolders, MeshArray, MeshFolderMap);

	ProjectMeshVisiblePixels(CameraData, MeshArray, MeshBoundsMap, MaximumDistance, World);
	
    UE_LOG(LogTemp, Log, TEXT("[图片: %s] 宽=%d, 高=%d"), *ImageMeta.Label, ImageMeta.Width, ImageMeta.Height);
    CalculateMeshPixelBounds(ImageMeta, MeshBoundsMap, MeshFolderMap);

	EnsureDirectoriesExist(ImagesDir, ReportImagesDir);
	CopyOriginalImageToImagesDir(ImageMeta.Name, ImagesDir);
    SavePixelBoundsToTxt(ReportImagesDir, ImageMeta);
    SavePixelBoundsImage(ReportImagesDir, ImageMeta);
}
