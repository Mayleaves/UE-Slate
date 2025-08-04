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

void FSequenceAlgorithmUtils::CollectCheckedCineCameraData(const FName& CameraName, FCineCameraData& OutCameraData)
{
    OutCameraData = FCineCameraData(); // 初始化
    const UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (!World) return;

    for (TActorIterator<ACineCameraActor> It(World); It; ++It)
    {
        if (const ACineCameraActor* CameraActor = *It; CameraActor && !CameraActor->IsPendingKillPending() && CameraActor->GetFName() == CameraName)
        {
            OutCameraData.Name = FName(CameraActor->GetName());
            OutCameraData.Label = CameraActor->GetActorLabel();
            OutCameraData.Location = CameraActor->GetActorLocation();
            OutCameraData.Rotation = CameraActor->GetActorRotation();
            OutCameraData.Scale = CameraActor->GetActorScale3D();
            OutCameraData.WorldMatrix = CameraActor->GetActorTransform().ToMatrixWithScale();
            ExtractCineCameraData(CameraActor, OutCameraData);  // 获取物理参数、成像平面四顶点
            
            UE_LOG(LogTemp, Log, TEXT("CineCameraActor Name: %s, Label: %s"), *OutCameraData.Name.ToString(), *OutCameraData.Label);
            UE_LOG(LogTemp, Log, TEXT("    Location: (%.2f, %.2f, %.2f)"), OutCameraData.Location.X, OutCameraData.Location.Y, OutCameraData.Location.Z);
            UE_LOG(LogTemp, Log, TEXT("    Rotation: (%.2f, %.2f, %.2f)"), OutCameraData.Rotation.Roll, OutCameraData.Rotation.Pitch, OutCameraData.Rotation.Yaw);
            UE_LOG(LogTemp, Log, TEXT("    Scale:    (%.2f, %.2f, %.2f)"), OutCameraData.Scale.X, OutCameraData.Scale.Y, OutCameraData.Scale.Z);
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
    F = OutCameraData.FocalLength * MMToCm;  // 深度：正方向 = 摄像机前方

    const FVector LocalCorners[4] = {           // YZX 坐标系（横竖深）
        FVector(F, -HalfW,  HalfH), // TopLeft
        FVector(F,  HalfW,  HalfH), // TopRight
        FVector(F,  HalfW, -HalfH), // BottomRight
        FVector(F, -HalfW, -HalfH)  // BottomLeft
    };

    OutCameraData.ImagingPlaneCorners.Empty();
    for (int i = 0; i < 4; ++i)
    {
        OutCameraData.ImagingPlaneCorners.Add(LocalCorners[i]);
    }
}

void FSequenceAlgorithmUtils::CollectCheckedStaticMeshData(const TArray<FName>& FolderNames, TArray<FStaticMeshData>& OutMeshArray)
{
    OutMeshArray.Empty();  // 清空输出数组
    const UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (!World) return;

    for (const FName& FolderName : FolderNames)
    {
        for (TActorIterator<AActor> It(World); It; ++It)
        {
            if (const AStaticMeshActor* SMActor = Cast<AStaticMeshActor>(*It); SMActor && !SMActor->IsPendingKillPending() &&
                SMActor->GetFolderPath().ToString() == FolderName)
            {
                FStaticMeshData MeshData;
                MeshData.Name = FName(SMActor->GetName());
                MeshData.Label = SMActor->GetActorLabel();
                MeshData.Location = SMActor->GetActorLocation();  // StaticMeshActor 世界坐标 cm
                MeshData.Rotation = SMActor->GetActorRotation();
                MeshData.Scale = SMActor->GetActorScale3D();
                MeshData.WorldMatrix = SMActor->GetActorTransform().ToMatrixWithScale();  // todo 4×4变换矩阵
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
                            const FPositionVertexBuffer& PositionBuffer = LODResource.VertexBuffers.PositionVertexBuffer;
                            const int32 NumRenderVertices = PositionBuffer.GetNumVertices();
                            
                            for (int32 i = 0; i < NumRenderVertices; ++i)
                            {
                                MeshData.LocalVertices.Add(PositionBuffer.VertexPosition(i));
                            }
                        }
                    }
                }
                OutMeshArray.Add(MeshData);
            }
        }
    }
    // 按Label排序
    OutMeshArray.Sort([](const FStaticMeshData& A, const FStaticMeshData& B) 
    {
        return A.Label < B.Label;
    });
    for (const auto& MeshData : OutMeshArray)
    {
        UE_LOG(LogTemp, Log, TEXT("StaticMeshActor Name: %s, Label: %s"), *MeshData.Name.ToString(), *MeshData.Label);
        UE_LOG(LogTemp, Log, TEXT("    Location: (%.2f, %.2f, %.2f)"), MeshData.Location.X, MeshData.Location.Y, MeshData.Location.Z);
        UE_LOG(LogTemp, Log, TEXT("    Rotation: (%.2f, %.2f, %.2f)"), MeshData.Rotation.Roll, MeshData.Rotation.Pitch, MeshData.Rotation.Yaw);
        UE_LOG(LogTemp, Log, TEXT("    Scale:    (%.2f, %.2f, %.2f)"), MeshData.Scale.X, MeshData.Scale.Y, MeshData.Scale.Z);
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

void FSequenceAlgorithmUtils::ProjectMeshVerticesToCameraPlane(const FCineCameraData& CameraData, const TArray<FStaticMeshData>& MeshArray)
{
    for (const FStaticMeshData& MeshData : MeshArray)
    {
        UE_LOG(LogTemp, Log, TEXT("==== Camera: %s 与 Mesh: %s ===="), *CameraData.Label, *MeshData.Label);
        
        float LocalNormXMin = FLT_MAX;
        float LocalNormXMax = -FLT_MAX;
        float LocalNormYMin = FLT_MAX;
        float LocalNormYMax = -FLT_MAX;
        
        FTransform CamTransform(CameraData.Rotation, CameraData.Location);
        FTransform CamInvTransform = CamTransform.Inverse();

        for (int32 i = 0; i < MeshData.LocalVertices.Num(); ++i)
        {
            // 1：StaticMesh 坐标变换（cm）
            FVector WorldPos = MeshData.WorldMatrix.TransformPosition(FVector(MeshData.LocalVertices[i]));  // 补 1
            const FVector VertexCameraSpace = CamInvTransform.TransformPosition(WorldPos);
            
            // UE_LOG(LogTemp, Log, TEXT("   顶点[%d]: 局域坐标(%.2f, %.2f, %.2f) → 世界坐标(%.2f, %.2f, %.2f) → 相机坐标(%.2f, %.2f, %.2f)"),
            //     i,
            //     MeshData.LocalVertices[i].X, MeshData.LocalVertices[i].Y, MeshData.LocalVertices[i].Z,
            //     WorldPos.X, WorldPos.Y, WorldPos.Z,
            //     VertexCameraSpace.X, VertexCameraSpace.Y, VertexCameraSpace.Z);
            
            // 2：投影变化（相似三角形计算）cm
            if (VertexCameraSpace.X > 0.f)  // Camera 局域坐标系：X > 0 表示在相机前方
            {
                // 成影平面坐标 cm
                const float ImagePlaneX = F * (VertexCameraSpace.Y / VertexCameraSpace.X);
                const float ImagePlaneY = F * (VertexCameraSpace.Z / VertexCameraSpace.X);

                // 图片默认坐标系 cm
                const float ImageX = ImagePlaneX + HalfW; 
                const float ImageY = HalfH - ImagePlaneY;
                
                // 归一化（0-1范围）cm
                const float NormX = ImageX / (2.0f * HalfW);
                const float NormY = ImageY / (2.0f * HalfH);
                
                // UE_LOG(LogTemp, Log, TEXT("           成像平面坐标(%.2f, %.2f) → 图片默认坐标系(%.2f, %.2f) → 归一化坐标(%.2f, %.2f)"),
                //     ImagePlaneX, ImagePlaneY, ImageX, ImageY, NormX, NormY);
                
                LocalNormXMin = FMath::Min(LocalNormXMin, NormX);
                LocalNormXMax = FMath::Max(LocalNormXMax, NormX);
                LocalNormYMin = FMath::Min(LocalNormYMin, NormY);
                LocalNormYMax = FMath::Max(LocalNormYMax, NormY);
            }
            else
            {
                // UE_LOG(LogTemp, Log, TEXT("           [被剔除：X <= 0]"));
            }
        }
        // Step 1: 判断是否完全越界（均小于0或均大于1直接舍弃）
        if (LocalNormXMax < 0.f || LocalNormXMin > 1.f || LocalNormYMax < 0.f || LocalNormYMin > 1.f)
        {
            UE_LOG(LogTemp, Warning, TEXT("   %s Mesh 超出成像平面，舍弃。"), *MeshData.Label);
            continue; // 舍弃本 mesh
        }
        
        // Step 2: 修正边界到[0, 1]范围
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
        UE_LOG(LogTemp, Log, TEXT("   边界统计: NormXMin=%.3f, NormXMax=%.3f, NormYMin=%.3f, NormYMax=%.3f"),
                ClampedNormXMin, ClampedNormXMax, ClampedNormYMin, ClampedNormYMax);
    }
}

EImageFormat FSequenceAlgorithmUtils::GetImageFormat(const FString& FilePath, const TMap<FString, EImageFormat>& FormatMap)
{
    if (const FString Ext = FPaths::GetExtension(FilePath).ToLower(); FormatMap.Contains(Ext))
        return FormatMap[Ext];
    return EImageFormat::Invalid;
}

void FSequenceAlgorithmUtils::CollectImageMetadata(const FString& SourceDirectoryPath, TArray<FImageMetadata>& OutImageMetadataArray)
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
    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));

    for (const FString& FilePath : FoundFiles)
    {
        // 检查文件扩展名
        if (FString LowercaseExt = FPaths::GetExtension(FilePath, true).ToLower(); !ImageExtensions.Contains(LowercaseExt))
            continue;

        // FString FileName = FPaths::GetCleanFilename(FilePath);  // image001.png
        FString FileNameWithoutExt = FPaths::GetBaseFilename(FilePath);  // image001
    
        // 读取图片尺寸
        int32 Width = 0, Height = 0;
        if (const auto ImageFormat = GetImageFormat(FilePath, ExtensionToFormatMap); ImageFormat != EImageFormat::Invalid)
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

void FSequenceAlgorithmUtils::CalculateMeshPixelBounds(FImageMetadata& ImageMeta, const TMap<FName, FMeshBoundsData>& MeshBoundsMap)
{
    for (const TPair<FName, FMeshBoundsData>& MeshPair : MeshBoundsMap)
    {
        const FMeshBoundsData& MeshData = MeshPair.Value;
        const float XRange = MeshData.NormXMax - MeshData.NormXMin;
        const float YRange = MeshData.NormYMax - MeshData.NormYMin;
        const float XThreshold = ImageMeta.Width * 0.00001f;   // 0.001% 的阈值
        const float YThreshold = ImageMeta.Height * 0.00001f;

        if (XRange >= XThreshold && YRange >= YThreshold)
        {
            FPixelBoundResult PixelBound;
            PixelBound.MeshName = MeshPair.Key;
            PixelBound.MeshLabel = MeshData.Label;
            PixelBound.XMinPixel = MeshData.NormXMin * ImageMeta.Width;
            PixelBound.XMaxPixel = MeshData.NormXMax * ImageMeta.Width;
            PixelBound.YMinPixel = MeshData.NormYMin * ImageMeta.Height;
            PixelBound.YMaxPixel = MeshData.NormYMax * ImageMeta.Height;
            ImageMeta.PixelBoundsArray.Add(PixelBound);
            
            UE_LOG(LogTemp, Log, TEXT("   %s Mesh: XMin=%.2f, XMax=%.2f, YMin=%.2f, YMax=%.2f"),
               *MeshData.Label, PixelBound.XMinPixel, PixelBound.XMaxPixel, PixelBound.YMinPixel, PixelBound.YMaxPixel);
        }
    }
}

void FSequenceAlgorithmUtils::SavePixelBoundsToTxt(const FString& SaveDirectoryPath, const FImageMetadata& ImageMeta)
{
    // 确保目录存在
    if (IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile(); !PlatformFile.DirectoryExists(*SaveDirectoryPath))
    {
        PlatformFile.CreateDirectoryTree(*SaveDirectoryPath);
    }

    TArray<FString> OutputLines;
    for (const FPixelBoundResult& PixelBound : ImageMeta.PixelBoundsArray)
    {
        // 写入文本行：MeshLabel, XMinPixel, XMaxPixel, YMinPixel, YMaxPixel
        FString Line = FString::Printf(TEXT("%s, %.2f, %.2f, %.2f, %.2f"),
            *PixelBound.MeshLabel, PixelBound.XMinPixel, PixelBound.XMaxPixel, PixelBound.YMinPixel, PixelBound.YMaxPixel);
        OutputLines.Add(Line);
    }

    // 拼接文件名: SaveDirectoryPath/image001.txt
    const FString SaveFilePath = FPaths::Combine(SaveDirectoryPath, ImageMeta.Label + TEXT(".txt"));
    FFileHelper::SaveStringArrayToFile(OutputLines, *SaveFilePath);
}

void FSequenceAlgorithmUtils::CollectCameraSequenceFrameMetadata(const FString& SourceDirectoryPath, const FString& SaveDirectoryPath) const
{
    TArray<FImageMetadata> ImageMetadataArray;
    CollectImageMetadata(SourceDirectoryPath, ImageMetadataArray);

    for (FImageMetadata& ImageMeta : ImageMetadataArray)
    {
        UE_LOG(LogTemp, Log, TEXT("[图片: %s] 宽=%d, 高=%d"), *ImageMeta.Label, ImageMeta.Width, ImageMeta.Height);
        
        CalculateMeshPixelBounds(ImageMeta, MeshBoundsMap);
        SavePixelBoundsToTxt(SaveDirectoryPath, ImageMeta);
    }
    UE_LOG(LogTemp, Log, TEXT("在目录 '%s' 中找到并处理了 %d 张图片"), *SourceDirectoryPath, ImageMetadataArray.Num());
}

// todo 用 opencv 根据处理后的像素坐标边界生成标注图片
