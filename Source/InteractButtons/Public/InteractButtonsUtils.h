#pragma once

#include "CoreMinimal.h"
#include "Internationalization/Text.h"
#include "ISequencer.h"
#include "MoviePipelineQueue.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SSpinBox.h"
#include "MenuTypes.h"
#include "InteractButtons/Private/MyDirectoryData.h"

class INTERACTBUTTONS_API SInteractButtonsUtils final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SInteractButtonsUtils)
			: _DefaultJob(nullptr)
			  , _CurrentStartFrame(0)
			  , _CurrentEndFrame(0)
			  , _DisplayRate(FFrameRate(1, 1))
			  , _TickResolution(FFrameRate(1, 1))
			  , _CurrentSequencer(nullptr)
		{
		}

		SLATE_ARGUMENT(UMoviePipelineExecutorJob*, DefaultJob) // 获取父模块传入的当前 Job
		SLATE_ARGUMENT(int32, CurrentStartFrame) // 当前 Sequencer 的起始显示帧
		SLATE_ARGUMENT(int32, CurrentEndFrame) // 当前 Sequencer 的结束显示帧
		SLATE_ARGUMENT(FFrameRate, DisplayRate) // 当前 Sequencer 的显示帧率
		SLATE_ARGUMENT(FFrameRate, TickResolution) // 当前 Sequencer 的内部帧率
		SLATE_ARGUMENT(TWeakPtr<ISequencer>, CurrentSequencer) // 获取父模块传入的 Sequencer 指针
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	// 从项目设置中获取默认图形预设，并将其分配给给定工作 
	static void AssignDefaultGraphPresetToJob(UMoviePipelineExecutorJob* InJob);

private:
	// 创建提示文本框
	static TSharedRef<SWidget> CreateLevelSequencePromptText();

	// 下拉菜单：“+” 
	TSharedRef<SWidget> MakeAddSequenceJobButton();
	// 删除任务，“-”
	TSharedRef<SWidget> RemoveLatestJobButton();
	// 选择资产
	TSharedRef<SWidget> OnGenerateNewJobFromAssetMenu();

	// Sequencer 默认 Job
	UMoviePipelineExecutorJob* DefaultJob = nullptr;
	// 私有 Sequence Job 队列：只会存储一个 Job
	TWeakObjectPtr<UMoviePipelineQueue> SequenceJobQueue;

	// 从资产（Asset）创建作业（Job）：替换
	void OnCreateJobFromAsset(const FAssetData& InAsset);
	// 显示帧率（Display Rate）、内部帧率（Tick Resolution）
	FFrameRate DisplayRate, TickResolution; // 自动初始化为1/1（1fps）
	// 设置 Job 的渲染帧范围
	void SetupJobFrameRangeFromLevelSequence(const UMoviePipelineExecutorJob* NewJob,
	                                         const ULevelSequence* LevelSequence);
	// 存储待选渲染作业指针的动态数组，用于在编辑器界面中批量选中新创建的渲染作业
	TArray<UMoviePipelineExecutorJob*> PendingJobsToSelect;

	// 队伍有效且有任务，即判断是否能够删除最新项目
	bool IsJobValid() const;
	// 删除最新项目
	FReply DeleteLatest();

	// 初始化一个空的任务细节面板
	void InitializeJobDetailsPanel();
	// 任务细节面板控件
	TSharedPtr<IDetailsView> JobDetailsPanelWidget;

	// 文件目录面板控件
	TSharedPtr<IDetailsView> FileDirectoryWidget;
	// 当前文件路径
	TWeakObjectPtr<UMyDirectoryData> FileSettingObject;
	// 初始化文件目录面板
	void InitializeFileDirectory();

	// 当前起始帧、结束帧
	int32 CurrentStartFrame = 0, CurrentEndFrame = 0;
	// 最小帧、最大帧
	int32 MinFrame = -100, MaxFrame = 10000;
	// 确保起始帧 <= 结束帧
	TSharedRef<SSpinBox<int32>> CreateFrameSpinBox(bool bIsStartFrame);
	// 气泡提示：未添加任务
	FText GetJobToolTip() const;
	// 帧范围输入面板
	TSharedRef<SWidget> CreateFrameRangeInputPanel();

	// 当前 Sequencer
	TWeakPtr<ISequencer> CurrentSequencer;
	// 获取 ISequencer 接口指针，该指针控制时序编辑器（Sequencer）的播放功能
	TSharedPtr<ISequencer> GetActiveSequencer() const;

	// 是否正在播放帧区间
	bool bIsPlayingRange = false;
	// 当前播放到的显示帧号 
	FFrameNumber CurrentPlayFrame;
	// Ticker的句柄，用于管理定时器
	FTSTicker::FDelegateHandle PlayRangeTickerHandle;
	// 开始区间播放
	void StartPlayRange();
	// 停止区间播放
	void StopPlayRange();
	// Ticker定时回调，每次跳转到下一帧（DeltaTime为回调间隔）
	bool OnPlayRangeTick(float DeltaTime);

	// 当前 Job->Sequence 是否等于当前 Sequencer->Sequence
	bool IsPlayRangeButtonEnabled() const;
	// 气泡提示：Job->Sequence != Sequencer->Sequence
	FText GetPlayRangeButtonToolTip() const;
	// 动态按钮文本显示：Play、Pause
	FText GetPlayRangeButtonText() const;
	// 更新播放范围
	void UpdatePlayRange(int32 NewStartFrame, int32 NewEndFrame);
	// 帧播放按钮
	TSharedRef<SWidget> CreateFramePlayRangeButton();


	// 切换按钮标签
	enum class EToolbarType
	{
		Camera,
		Label
	};
	// 切换按钮是否为激活状态
	bool bIsCameraToggled = false, bIsLabelToggled = false;
	// 映射：唯一标识 - 菜单项
	TMap<FName, FMenuItem> CameraMenuItemMap, FolderMenuItemMap;
	// 用于显示菜单项（同时包含唯一标识）
	TArray<FMenuItem> CameraMenuItems, FolderMenuItems;
	// 映射：唯一标识 - 勾选状态
	FName CameraMenuRadioButton;
	TMap<FName, ECheckBoxState> FolderMenuCheckBox;
	// 保证单选组 CameraMenuRadioButton 的互斥性
	TMap<FName, TSharedPtr<SCheckBox>> CameraCheckBoxMap;
	// 映射：唯一标识 - 4×4矩阵
	TMap<FName, FMatrix> CameraMatrices, StaticMeshMatrices;

	// 文本高亮 + 复选框
	TSharedRef<SWidget> CreateMenuEntryWidget(
		FName& Name,
		const FString& Label,
		const FString& FilterString,
		FName* CameraSelectedKey,   
		TMap<FName, ECheckBoxState>* FolderCheckStates
	);
	
	// 更新菜单项区域
	void UpdateMenuEntries(
		TArray<FMenuItem>* MenuItems,         
		FName* CameraSelectedKey,                     
		TMap<FName, ECheckBoxState>* FolderCheckStates,
		const FText& FilterText,
		const TSharedPtr<SVerticalBox>& MenuEntriesBox
	);

	// 当前搜索框内容
	FText CameraFilterText, FolderFilterText;
	// 菜单项区域
	TSharedPtr<SVerticalBox> CameraMenuEntriesBox, FolderMenuEntriesBox;
	// 构造下拉按钮：搜索框 + 菜单项
	TSharedRef<SWidget> GenerateDropdownMenu(
		TArray<FMenuItem>* MenuItems,
		FName* CameraSelectedKey,                  
		TMap<FName, ECheckBoxState>* FolderCheckStates
	);

	// 获取编辑器场景中的所有菜单项
	static void GetLevelAllMenuItems(EToolbarType MenuType, TMap<FName, FMenuItem>& OutMenuItemMap);
	// Toolbar按钮：切换按钮 + 下拉按钮
	TSharedRef<SWidget> BuildToolbar(EToolbarType ToolbarType);

	// 进度
	float ProgressPercent = 0.0f;
	// 进度条绑定
	TOptional<float> GetLabelingProgress() const;
	// 获取进度条
	TSharedRef<SWidget> CreateProgressBar();

	// 开始标注
	TSharedRef<SWidget> CreateStartLabelingButton();
	// 源路径、保存路径
	FString SourcePath, SavePath;
	// 当前标注文件夹
	TArray<FName> CheckedFolders;
    // 当前标注步骤
	int32 CurrentFrame = 0;
	// 定时器
	FTimerHandle LabelingTimerHandle;
	// 开始标注按钮点击事件
	FReply OnStartLabelingClicked();
	// 定时器驱动的标注步骤
	void DoLabelingStep();

};
