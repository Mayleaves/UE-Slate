#include "MyDirectoryData.h"

UMyDirectoryData::UMyDirectoryData()
{
    // 初始化默认目录路径
    SourceDirectory.Path = TEXT("(No Directory Selected)");
    SaveDirectory.Path   = TEXT("(No Directory Selected)");
}