#include "MyDirectoryData.h"

UMyDirectoryData::UMyDirectoryData()
{
    // 初始化默认目录路径
    // SourceDirectory.Path = TEXT("(No Directory Selected)");
    // SaveDirectory.Path   = TEXT("(No Directory Selected)");
    SourceDirectory.Path = TEXT("D:/DevelopmentTools/Unreal Projects/CISAI_SD_Factory/Saved/MovieRenders");  // todo 仅为开发，后期删除
    SaveDirectory.Path   = TEXT("D:/DevelopmentTools/Unreal Projects/CISAI_SD_Factory/Saved/MovieRenders");
}