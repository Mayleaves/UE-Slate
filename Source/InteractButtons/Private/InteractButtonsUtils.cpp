#include "InteractButtonsUtils.h"

// 自定义模块
#include "MyJobCustomization.h"
#include "MyPositiveActionButton.h"
#include "MenuTypes.h"
#include "SequenceAlgorithmUtils.h"

// Unreal Engine 核心模块
#include "CineCameraActor.h"
#include "ContentBrowserModule.h"
#include "Editor.h"
#include "EditorFontGlyphs.h"
#include "Editor/Sequencer/Public/ISequencerModule.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Graph/MovieGraphPipeline.h"
#include "IContentBrowserSingleton.h"
#include "LevelSequence.h"
#include "Modules/ModuleManager.h"
#include "MoviePipelineEditorBlueprintLibrary.h"
#include "MoviePipelineOutputSetting.h"
#include "MoviePipelineQueue.h"
#include "MovieRenderPipelineSettings.h"
#include "MovieScene.h"
#include "MyDirectoryData.h"
#include "MyOutputFormatDetailsCustomization.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "InteractButtonsUtils"

void SInteractButtonsUtils::Construct(const FArguments& InArgs)
{
	DefaultJob = InArgs._DefaultJob;
	CurrentStartFrame = InArgs._CurrentStartFrame;
	CurrentEndFrame = InArgs._CurrentEndFrame;
	DisplayRate = InArgs._DisplayRate;
	TickResolution = InArgs._TickResolution;
	CurrentSequencer = InArgs._CurrentSequencer;

	InitializeJobDetailsPanel();
	InitializeFileDirectory();
	
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10, 10, 5, 5)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.FillWidth(1.0f) // 填充
			[
				CreateLevelSequencePromptText() // "Select a Level Sequence:"
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2, 0)
			[
				MakeAddSequenceJobButton() // + Select a Scene
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2, 0)
			[
				RemoveLatestJobButton() // -
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10, 5, 10, 0)
		[
			JobDetailsPanelWidget.ToSharedRef() // Job Info
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10, 0, 10, 5)
		[
			FileDirectoryWidget.ToSharedRef() // Source Directory
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5, 5, 10, 5)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				BuildToolbar(EToolbarType::Camera) // CineCameraActor
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(10, 0)
			.FillWidth(1.0f) // 填充
			[
				BuildToolbar(EToolbarType::Label) // Folder
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(10, 0)
			[
				CreateFrameRangeInputPanel() // Start Frame + End Frame
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 10)
			[
				CreateFramePlayRangeButton() // Play/Pause Range
			]
		]
		
		+ SVerticalBox::Slot()
		.Padding(10, 5, 10, 10)
		.FillHeight(1.0f)
		.VAlign(VAlign_Bottom)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.FillWidth(1.0f)
			.Padding(0, 0, 10, 0)
			[
				CreateProgressBar()  // Progress Bar
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				CreateStartLabelingButton() // Start Labeling
			]
		]

	];
}

void SInteractButtonsUtils::InitializeJobDetailsPanel()
{
	// 1. 创建细节视图（Detail Panel）
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::Get().LoadModuleChecked<FPropertyEditorModule>(
		"PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.ColumnWidth = 0.7f;

	JobDetailsPanelWidget = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	// 2. 注册细节定制类
	JobDetailsPanelWidget->RegisterInstancedCustomPropertyLayout(
		UMoviePipelineExecutorJob::StaticClass(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FMyJobDetailsCustomization::MakeInstance)
	);

	// 3. 初始化队列
	SequenceJobQueue = NewObject<UMoviePipelineQueue>();
	check(SequenceJobQueue.IsValid());

	// 4. 设置初始化 Job
	if (DefaultJob) // 将父类传递的 DefaultJob 加入队列
	{
		// 用 DuplicateJob 复制一份 DefaultJob，并添加到队列中
		if (UMoviePipelineExecutorJob* JobCopy = SequenceJobQueue->DuplicateJob(DefaultJob))
		{
			JobDetailsPanelWidget->SetObject(JobCopy);
			UMoviePipelineOutputSetting* OutputSetting = JobCopy->GetConfiguration()->FindSetting<
				UMoviePipelineOutputSetting>();
			if (OutputSetting)
			{
				OutputSetting->CustomStartFrame = CurrentStartFrame;
				OutputSetting->CustomEndFrame = CurrentEndFrame;
				OutputSetting->bUseCustomPlaybackRange = true; // 自定义渲染范围
			}
		}
	}
	else
	{
		// 如果父类没有传递 DefaultJob，则创建一个空的占位 Job
		const TObjectPtr<UMoviePipelineExecutorJob> InitialJob = NewObject<UMoviePipelineExecutorJob>();
		InitialJob->JobName = TEXT("(No Job Selected)");
		JobDetailsPanelWidget->SetObject(InitialJob);
	}
}

void SInteractButtonsUtils::InitializeFileDirectory()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(
		"PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.ColumnWidth = 0.7f;

	FileDirectoryWidget = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	
	// 注册自定义属性布局
	FileDirectoryWidget->RegisterInstancedCustomPropertyLayout(
		UMyDirectoryData::StaticClass(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FMyOutputFormatDetailsCustomization::MakeInstance)
	);
	
	// 用自定义默认对象填充
	FileSettingObject = NewObject<UMyDirectoryData>();  // todo 获取地址：FileSettingObject->SourceDirectory.Path、FileSettingObject->SaveDirectory.Path
	FileDirectoryWidget->SetObject(FileSettingObject.Get());
}

TSharedRef<SWidget> SInteractButtonsUtils::CreateLevelSequencePromptText()
{
	return SNew(STextBlock)
	                       .Text(NSLOCTEXT("InteractButtons", "LevelSequencePrompt",
	                                       "Select a Level Sequence")) // 使用本地化文本
	                       .Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
	                       .ColorAndOpacity(FLinearColor::White);
}

TSharedRef<SWidget> SInteractButtonsUtils::MakeAddSequenceJobButton()
{
	return SNew(SMyPositiveActionButton)
		.Text(LOCTEXT("AddNewJob_Text", " Select a Scene"))
		.Icon(FAppStyle::Get().GetBrush("Icons.Plus"))
		.OnGetMenuContent(this, &SInteractButtonsUtils::OnGenerateNewJobFromAssetMenu);
}

TSharedRef<SWidget> SInteractButtonsUtils::RemoveLatestJobButton()
{
	return SNew(SButton)
	                    .Content()
	                    [
		                    SNew(STextBlock)
		                                    .Justification(ETextJustify::Center) // 显式居中
		                                    .TextStyle(FAppStyle::Get(), "NormalText.Important")
		                                    .Font(FAppStyle::Get().GetFontStyle("FontAwesome.10"))
		                                    .Text(FEditorFontGlyphs::Minus)
	                    ]
	                    .ContentPadding(FMargin(4.f)) // 等同于 MoviePipeline::ButtonPadding
	                    .IsEnabled(this, &SInteractButtonsUtils::IsJobValid)
	                    .OnClicked(this, &SInteractButtonsUtils::DeleteLatest)
	                    .VAlign(VAlign_Center);
}

TSharedRef<SWidget> SInteractButtonsUtils::OnGenerateNewJobFromAssetMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);
	IContentBrowserSingleton& ContentBrowser = FModuleManager::LoadModuleChecked<FContentBrowserModule>(
		"ContentBrowser").Get();

	FAssetPickerConfig AssetPickerConfig;
	{
		AssetPickerConfig.SelectionMode = ESelectionMode::Single;
		AssetPickerConfig.InitialAssetViewType = EAssetViewType::List;
		AssetPickerConfig.bFocusSearchBoxWhenOpened = true;
		AssetPickerConfig.bAllowNullSelection = false;
		AssetPickerConfig.bShowBottomToolbar = true;
		AssetPickerConfig.bAutohideSearchBar = false;
		AssetPickerConfig.bAllowDragging = false;
		AssetPickerConfig.bCanShowClasses = false;
		AssetPickerConfig.bShowPathInColumnView = true;
		AssetPickerConfig.bShowTypeInColumnView = false;
		AssetPickerConfig.bSortByPathInColumnView = false;
		AssetPickerConfig.ThumbnailScale = 0.4f;
		AssetPickerConfig.SaveSettingsName = TEXT("MoviePipelineQueueJobAsset");

		AssetPickerConfig.AssetShowWarningText = LOCTEXT("NoSequences_Warning", "No Level Sequences Found");
		AssetPickerConfig.Filter.ClassPaths.Add(ULevelSequence::StaticClass()->GetClassPathName());
		AssetPickerConfig.Filter.bRecursiveClasses = true;
		AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateSP(
			this, &SInteractButtonsUtils::OnCreateJobFromAsset);
	}

	MenuBuilder.BeginSection(NAME_None, LOCTEXT("NewJob_MenuSection", "New Render Job"));
	{
		TSharedRef<SWidget> PresetPicker = SNew(SBox)
			.WidthOverride(300.f)
			.HeightOverride(300.f)
			[
				ContentBrowser.CreateAssetPicker(AssetPickerConfig)
			];

		MenuBuilder.AddWidget(PresetPicker, FText(), true, false);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void SInteractButtonsUtils::OnCreateJobFromAsset(const FAssetData& InAsset)
{
	FSlateApplication::Get().DismissAllMenus();

	UMoviePipelineQueue* ActiveQueue = SequenceJobQueue.Get();
	check(ActiveQueue);

	FScopedTransaction Transaction(LOCTEXT("ReplaceJob_Transaction", "Replace Job with new one"));

	ActiveQueue->Modify();

	// 通过反射清空私有 Jobs 数组
	const FArrayProperty* ArrayProp = FindFProperty<FArrayProperty>(
		UMoviePipelineQueue::StaticClass(),
		TEXT("Jobs")
	);

	if (ArrayProp)
	{
		void* ArrayAddr = ArrayProp->ContainerPtrToValuePtr<void>(ActiveQueue);
		if (TArray<UMoviePipelineExecutorJob*>* JobsPtr = static_cast<TArray<UMoviePipelineExecutorJob*>*>(ArrayAddr))
		{
			JobsPtr->Empty();
		}
	}

	TArray<UMoviePipelineExecutorJob*> NewJobs;

	if (ULevelSequence* LevelSequence = Cast<ULevelSequence>(InAsset.GetAsset()))
	{
		UMoviePipelineExecutorJob* NewJob = UMoviePipelineEditorBlueprintLibrary::CreateJobFromSequence(
			ActiveQueue, LevelSequence);
		if (!NewJob)
		{
			return;
		}

		// 将播放区帧范围（Playback Range）设置为 Job 渲染帧范围
		SetupJobFrameRangeFromLevelSequence(NewJob, LevelSequence);

		NewJobs.Add(NewJob);
	}
	else if (UMoviePipelineQueue* Queue = Cast<UMoviePipelineQueue>(InAsset.GetAsset()))
	{
		for (UMoviePipelineExecutorJob* Job : Queue->GetJobs())
		{
			if (UMoviePipelineExecutorJob* NewJob = ActiveQueue->DuplicateJob(Job))
			{
				NewJobs.Add(NewJob);
			}
		}
	}

	const UMovieRenderPipelineProjectSettings* ProjectSettings = GetDefault<UMovieRenderPipelineProjectSettings>();
	const UClass* DefaultPipeline = Cast<UClass>(ProjectSettings->DefaultPipeline.TryLoad());

	for (UMoviePipelineExecutorJob* NewJob : NewJobs)
	{
		PendingJobsToSelect.Add(NewJob);

		if (ProjectSettings->LastPresetOrigin.IsValid())
		{
			NewJob->SetPresetOrigin(ProjectSettings->LastPresetOrigin.Get());
		}

		UMoviePipelineEditorBlueprintLibrary::EnsureJobHasDefaultSettings(NewJob);

		if (DefaultPipeline && DefaultPipeline == UMovieGraphPipeline::StaticClass())
		{
			AssignDefaultGraphPresetToJob(NewJob);
		}
	}

	if (NewJobs.Num() > 0 && JobDetailsPanelWidget.IsValid())
	{
		JobDetailsPanelWidget->SetObject(NewJobs.Last());
	}
}

void SInteractButtonsUtils::SetupJobFrameRangeFromLevelSequence(const UMoviePipelineExecutorJob* NewJob,
                                                                const ULevelSequence* LevelSequence)
{
	if (!NewJob || !LevelSequence) return;

	// 获取播放区帧范围
	if (const UMovieScene* MovieScene = LevelSequence->GetMovieScene())
	{
		DisplayRate = MovieScene->GetDisplayRate();
		TickResolution = MovieScene->GetTickResolution();
		const TRange<FFrameNumber> PlaybackRange = MovieScene->GetPlaybackRange();

		// 内部帧号、Tick帧、原始帧、底层帧
		const FFrameNumber StartInternalFrame = PlaybackRange.GetLowerBoundValue();
		const FFrameNumber EndInternalFrame = PlaybackRange.GetUpperBoundValue();
		UE_LOG(LogTemp, Log, TEXT("Playback Range: 起始内部帧 = %d, 结束内部帧 = %d"), StartInternalFrame.Value,
		       EndInternalFrame.Value);

		// 显示帧号、可视化帧号、Display帧、UI帧、Timeline帧
		const FFrameNumber StartDisplayFrame = FFrameRate::TransformTime(
			FFrameTime(StartInternalFrame), TickResolution, DisplayRate).FloorToFrame();
		const FFrameNumber EndDisplayFrame = FFrameRate::TransformTime(FFrameTime(EndInternalFrame), TickResolution,
		                                                               DisplayRate).FloorToFrame();
		UE_LOG(LogTemp, Log, TEXT("Playback Range: 起始显示帧 = %d, 结束显示帧 = %d"), StartDisplayFrame.Value,
		       EndDisplayFrame.Value);

		UMoviePipelineOutputSetting* OutputSetting = NewJob->GetConfiguration()->FindSetting<
			UMoviePipelineOutputSetting>();
		if (!OutputSetting)
		{
			if (UFunction* AddSettingFunc = NewJob->GetConfiguration()->FindFunction(FName("AddSetting")))
			{
				struct
				{
					UClass* InSettingType;
					UObject* ReturnValue;
				} Params;
				Params.InSettingType = UMoviePipelineOutputSetting::StaticClass();
				Params.ReturnValue = nullptr;

				NewJob->GetConfiguration()->ProcessEvent(AddSettingFunc, &Params);
				OutputSetting = Cast<UMoviePipelineOutputSetting>(Params.ReturnValue);
			}
		}
		if (OutputSetting)
		{
			// 设置 Job 渲染起始和结束帧为“显示帧”
			OutputSetting->CustomStartFrame = CurrentStartFrame = StartDisplayFrame.Value; // CustomStartFrame 默认为 0
			OutputSetting->CustomEndFrame = CurrentEndFrame = EndDisplayFrame.Value; // CustomEndFrame 默认为 0
			OutputSetting->bUseCustomPlaybackRange = true; // 自定义渲染范围
			// 调用帧范围输入面板
			CreateFrameRangeInputPanel();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("未找到输出设置，无法设置帧范围!"));
		}
	}
}

void SInteractButtonsUtils::AssignDefaultGraphPresetToJob(UMoviePipelineExecutorJob* InJob)
{
	if (!InJob) return;

	FScopedTransaction Transaction(LOCTEXT("ConvertJobToGraphConfig_Transaction", "Convert Job to Graph Config"));
	InJob->Modify();

	const UMovieRenderPipelineProjectSettings* ProjectSettings = GetDefault<UMovieRenderPipelineProjectSettings>();
	const TSoftObjectPtr<UMovieGraphConfig> ProjectDefaultGraph = ProjectSettings->DefaultGraph;
	if (const UMovieGraphConfig* DefaultGraph = ProjectDefaultGraph.LoadSynchronous())
	{
		InJob->SetGraphPreset(DefaultGraph);
	}
	else
	{
		FNotificationInfo Info(LOCTEXT("ConvertJobToGraphConfig_InvalidGraphNotification", "Unable to Convert Job"));
		Info.SubText = LOCTEXT("ConvertJobToGraphConfig_InvalidGraphNotificationSubtext",
		                       "The Graph Asset specified in Project Settings (Movie Render Pipeline > Default Graph) could not be loaded.");
		Info.Image = FAppStyle::GetBrush(TEXT("Icons.Warning"));
		Info.ExpireDuration = 5.0f;

		FSlateNotificationManager::Get().AddNotification(Info);
	}
}

bool SInteractButtonsUtils::IsJobValid() const
{
	return SequenceJobQueue.IsValid() && SequenceJobQueue->GetJobs().Num() > 0;
}

FReply SInteractButtonsUtils::DeleteLatest()
{
	if (!SequenceJobQueue.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("SequenceJobQueue 无效!"));
		return FReply::Handled();
	}

	if (SequenceJobQueue->GetJobs().Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("SequenceJobQueue 为空!"));
		return FReply::Handled();
	}

	// 通过反射绕过 protected 访问限制，直接访问 Jobs 数组
	const FArrayProperty* ArrayProp = FindFProperty<FArrayProperty>(
		UMoviePipelineQueue::StaticClass(),
		TEXT("Jobs") // 用字符串避免宏导致的访问错误
	);

	if (!ArrayProp) return FReply::Handled();

	void* ArrayAddr = ArrayProp->ContainerPtrToValuePtr<void>(SequenceJobQueue.Get());
	TArray<UMoviePipelineExecutorJob*>* JobsPtr = static_cast<TArray<UMoviePipelineExecutorJob*>*>(ArrayAddr);

	if (!JobsPtr || JobsPtr->Num() == 0)
	{
		return FReply::Handled();
	}

	FScopedTransaction Transaction(LOCTEXT("DeleteLatestJob", "Delete Latest Job"));
	SequenceJobQueue->Modify();

	if (UMoviePipelineExecutorJob* LatestJob = JobsPtr->Last())
	{
		LatestJob->Modify();
	}

	JobsPtr->RemoveAt(JobsPtr->Num() - 1);

	if (JobDetailsPanelWidget.IsValid())
	{
		// 没有 Job 时显示一个空占位对象（只创建一次，防止重复创建）
		static TWeakObjectPtr<UMoviePipelineExecutorJob> EmptyJob = nullptr;
		if (!EmptyJob.IsValid())
		{
			EmptyJob = NewObject<UMoviePipelineExecutorJob>();
		}
		EmptyJob->JobName = TEXT("(No Job Selected)"); // 方便区分
		JobDetailsPanelWidget->SetObject(EmptyJob.Get());
	}

	// 重置
	CurrentStartFrame = 0, CurrentEndFrame = 0;
	UE_LOG(LogTemp, Log, TEXT("已重置: StartFrame = %d, EndFrame = %d"), CurrentStartFrame, CurrentEndFrame);
	// 调用帧范围输入面板
	CreateFrameRangeInputPanel();

	return FReply::Handled();
}

TSharedRef<SSpinBox<int32>> SInteractButtonsUtils::CreateFrameSpinBox(bool bIsStartFrame)
{
	return SNew(SSpinBox<int32>)
		.MinValue(MinFrame)
		.MaxValue(MaxFrame)
		.IsEnabled(this, &SInteractButtonsUtils::IsJobValid)
		.ToolTipText(this, &SInteractButtonsUtils::GetJobToolTip)
		.Value_Lambda([this, bIsStartFrame]()
		{
			return bIsStartFrame ? CurrentStartFrame : CurrentEndFrame;
		})
		.OnValueChanged_Lambda([this, bIsStartFrame](const int32 NewValue)
		{
			const int32 BeforeStartFrame = CurrentStartFrame;
			const int32 BeforeEndFrame = CurrentEndFrame;
			bool bLegal = true; // 合法操作
			if (bIsStartFrame)
			{
				CurrentStartFrame = NewValue;
				if (CurrentStartFrame > CurrentEndFrame) // 非法操作
				{
					CurrentEndFrame = CurrentStartFrame;
					UE_LOG(LogTemp, Warning,
					       TEXT(
						       "StartFrame 修改前: %d, EndFrame 修改前: %d; StartFrame 修改后: %d, EndFrame 修改后: %d (非法，自动修正 EndFrame=StartFrame)"
					       ),
					       BeforeStartFrame, BeforeEndFrame, CurrentStartFrame, CurrentEndFrame);
					bLegal = false;
				}
			}
			else
			{
				CurrentEndFrame = NewValue;
				if (CurrentEndFrame < CurrentStartFrame) // 非法操作
				{
					CurrentStartFrame = CurrentEndFrame;
					UE_LOG(LogTemp, Warning,
					       TEXT(
						       "StartFrame 修改前: %d, EndFrame 修改前: %d; StartFrame 修改后: %d, EndFrame 修改后: %d (非法，自动修正 StartFrame=EndFrame)"
					       ),
					       BeforeStartFrame, BeforeEndFrame, CurrentStartFrame, CurrentEndFrame);
					bLegal = false;
				}
			}
			if (bLegal) // 合法操作
			{
				UE_LOG(LogTemp, Log,
				       TEXT(
					       "StartFrame 修改前: %d, EndFrame 修改前: %d; StartFrame 修改后: %d, EndFrame 修改后: %d"
				       ),
				       BeforeStartFrame, BeforeEndFrame, CurrentStartFrame, CurrentEndFrame);
			}
			// 更新播放范围
			UpdatePlayRange(CurrentStartFrame, CurrentEndFrame);
		});
}

FText SInteractButtonsUtils::GetJobToolTip() const
{
	return IsJobValid() ? FText::GetEmpty() : FText::FromString(TEXT("Please select a scene first"));
}

TSharedRef<SWidget> SInteractButtonsUtils::CreateFrameRangeInputPanel()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(5, 0)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock).Text(FText::FromString(TEXT("Start Frame")))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4, 10, 10, 10)
		[
			CreateFrameSpinBox(true) // 起始帧
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(5, 0)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock).Text(FText::FromString(TEXT("End Frame")))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4, 10, 10, 10)
		[
			CreateFrameSpinBox(false) // 结束帧
		];
}

TSharedPtr<ISequencer> SInteractButtonsUtils::GetActiveSequencer() const
{
	return CurrentSequencer.Pin();
}

bool SInteractButtonsUtils::IsPlayRangeButtonEnabled() const
{
	// 检查当前 Job 是否是 DefaultJob
	if (SequenceJobQueue.IsValid() && SequenceJobQueue->GetJobs().Num() > 0)
	{
		TWeakObjectPtr<UMoviePipelineExecutorJob> CurrentJob = SequenceJobQueue->GetJobs()[0]; // 检查第一个 Job

		if (CurrentJob.IsValid() && DefaultJob)
		{
			// 判断是否引用同一个 Level Sequence
			const bool isSameSequence = CurrentJob->Sequence == DefaultJob->Sequence;

			// 返回是否相同，true 时按钮启用
			return isSameSequence;
		}
	}

	// 任何异常或队列空时，按钮禁用
	return false;
}

FText SInteractButtonsUtils::GetPlayRangeButtonToolTip() const
{
	if (IsPlayRangeButtonEnabled())
	{
		return FText::FromString(TEXT("")); // 启用时不显示提示
	}
	return FText::FromString(TEXT("Please select the matching level sequence")); // 禁用时提示
}

FText SInteractButtonsUtils::GetPlayRangeButtonText() const
{
	return bIsPlayingRange ? FText::FromString("Pause Range") : FText::FromString("Play Range");
}

void SInteractButtonsUtils::StartPlayRange()
{
	if (bIsPlayingRange) return;
	bIsPlayingRange = true;

	PlayRangeTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateRaw(this, &SInteractButtonsUtils::OnPlayRangeTick), 1.0f);
}

void SInteractButtonsUtils::StopPlayRange()
{
	if (!bIsPlayingRange) return;
	bIsPlayingRange = false;
	FTSTicker::GetCoreTicker().RemoveTicker(PlayRangeTickerHandle);
}

bool SInteractButtonsUtils::OnPlayRangeTick(float DeltaTime)
{
	// 将显示帧转为内部帧
	const FFrameTime DisplayFrame(CurrentPlayFrame);
	const FFrameNumber InternalFrame = FFrameRate::TransformTime(DisplayFrame, DisplayRate, TickResolution).
		FloorToFrame();

	if (const TSharedPtr<ISequencer> SequencerToUse = GetActiveSequencer(); SequencerToUse.IsValid())
	{
		SequencerToUse->SetLocalTime(InternalFrame.Value);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("当前 Sequencer 无效！请确保已打开 Sequencer 面板。"));
	}

	// 下一帧
	if (CurrentPlayFrame == CurrentEndFrame)
	{
		UE_LOG(LogTemp, Log, TEXT("到达结束帧 %d，循环回到起始帧 %d"), CurrentPlayFrame.Value, CurrentStartFrame);
		CurrentPlayFrame = CurrentStartFrame; // 循环回去
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("当前显示帧: %d"), CurrentPlayFrame.Value);
		++CurrentPlayFrame;
	}

	// 返回true表示继续调用，false会自动移除Ticker
	return bIsPlayingRange;
}

void SInteractButtonsUtils::UpdatePlayRange(int32 NewStartFrame, int32 NewEndFrame)
{
	// 更新起始帧和结束帧
	CurrentStartFrame = NewStartFrame;
	CurrentEndFrame = NewEndFrame;

	// 检查当前播放头是否需要调整
	if (CurrentPlayFrame < CurrentStartFrame)
	{
		UE_LOG(LogTemp, Warning, TEXT("当前播放帧 %d 小于新的起始帧 %d，重置到新起始帧。"), CurrentPlayFrame.Value, CurrentStartFrame);
		CurrentPlayFrame = CurrentStartFrame;
	}
	else if (CurrentPlayFrame > CurrentEndFrame)
	{
		UE_LOG(LogTemp, Warning, TEXT("当前播放帧 %d 大于新的结束帧 %d，重置到起始帧 %d。"), CurrentPlayFrame.Value, CurrentEndFrame,
		       CurrentStartFrame);
		CurrentPlayFrame = CurrentStartFrame;
	}
}

TSharedRef<SWidget> SInteractButtonsUtils::CreateFramePlayRangeButton()
{
	return SNew(SButton)
	                    .Content()
	                    [
		                    SNew(STextBlock)
		                                    .Justification(ETextJustify::Center) // 显式居中
		                                    .Text(this, &SInteractButtonsUtils::GetPlayRangeButtonText) // 动态文本
	                    ]
	                    .IsEnabled(this, &SInteractButtonsUtils::IsPlayRangeButtonEnabled)
	                    .ToolTipText(this, &SInteractButtonsUtils::GetPlayRangeButtonToolTip)
	                    .ButtonColorAndOpacity_Lambda([this]()
	                    {
		                    return bIsPlayingRange ? FLinearColor::Red : FLinearColor::Green; // 暂停红，播放绿
	                    })
	                    .ButtonStyle(FAppStyle::Get(),
	                                 bIsPlayingRange ? "FlatButton.Danger" : "FlatButton.Success") // 动态按钮样式（可选，增强视觉效果）
	                    .OnClicked_Lambda([this]()
	                    {
		                    bIsPlayingRange ? StopPlayRange() : StartPlayRange();
		                    return FReply::Handled();
	                    });
}

TSharedRef<SWidget> SInteractButtonsUtils::CreateMenuEntryWidget(
    FName& Name,
    const FString& Label,
    const FString& FilterString,
    FName* CameraSelectedKey,
    TMap<FName, ECheckBoxState>* FolderCheckStates
)
{
    const bool bIsCamera = (CameraSelectedKey != nullptr);
	
	// 提取高亮部分
    auto ExtractHighlightParts = [](const FString& Text, const FString& Filter) {
        FString Before = Text, Match, After;
        if (!Filter.IsEmpty()) {
            if (const int32 Index = Text.ToLower().Find(Filter.ToLower()); Index != INDEX_NONE) {
                Before = Text.Left(Index);
                Match = Text.Mid(Index, Filter.Len());
                After = Text.Mid(Index + Filter.Len());
            }
        }
        return std::make_tuple(Before, Match, After);
    };
    auto [Before, Match, After] = ExtractHighlightParts(Label, FilterString);

    // 检查是否有匹配的高亮部分
    auto IsCheckedFunc = [Name, CameraSelectedKey, FolderCheckStates, bIsCamera]() {
        if (bIsCamera) {
            return (*CameraSelectedKey == Name) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
        }
        if (FolderCheckStates) {
	        const ECheckBoxState* State = FolderCheckStates->Find(Name);
	        return State ? *State : ECheckBoxState::Unchecked;
        }
        return ECheckBoxState::Unchecked;
    };

	// 更新单选框/复选框状态
    auto ChangeCheckState = [this, Name, Label, CameraSelectedKey, FolderCheckStates, bIsCamera](ECheckBoxState NewState) {
        if (bIsCamera) {
             if (NewState == ECheckBoxState::Checked) {
                *CameraSelectedKey = Name;
            	UE_LOG(LogTemp, Log, TEXT("勾选: %s"), *Label);
                // 清除其他 Camera 勾选
                for (auto& Pair : CameraCheckBoxMap) {
                    if (Pair.Key != Name && Pair.Value.IsValid()) {
                        Pair.Value->SetIsChecked(ECheckBoxState::Unchecked);
                    }
                }
            }else if (*CameraSelectedKey == Name && NewState == ECheckBoxState::Unchecked) {
            	*CameraSelectedKey = FName();
                UE_LOG(LogTemp, Warning, TEXT("取消: %s"), *Label);
             }
        } else if (FolderCheckStates) {
        	if (NewState == ECheckBoxState::Checked)
        	{
        		FolderCheckStates->Add(Name, NewState);
        		UE_LOG(LogTemp, Log, TEXT("勾选: %s"), *Label);
        	}
        	else
        	{
        		FolderCheckStates->Remove(Name);
        		UE_LOG(LogTemp, Warning, TEXT("取消: %s"), *Label);
        	}
        }
    };

    const TSharedRef<SHorizontalBox> RowBox = SNew(SHorizontalBox);
    TSharedPtr<SCheckBox> CheckBoxPtr;
	
	// 添加 CheckBox（单选/多选分支）
    RowBox->AddSlot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 8, 0)
    [
        SAssignNew(CheckBoxPtr, SCheckBox)
        .Style(FAppStyle::Get(), bIsCamera ? "Menu.RadioButton" : "Menu.CheckBox")
        .IsChecked_Lambda(IsCheckedFunc)
        .OnCheckStateChanged_Lambda(ChangeCheckState)
    ];

    // 注册 Camera CheckBox
    if (bIsCamera && CheckBoxPtr.IsValid()) {
        CameraCheckBoxMap.Add(Name, CheckBoxPtr);
    }

    // 文本高亮部分
    RowBox->AddSlot()
        .AutoWidth()
        .VAlign(VAlign_Center)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock).Text(FText::FromString(Before)).ColorAndOpacity(FSlateColor::UseForeground())
        ]
        + SHorizontalBox::Slot().AutoWidth()[
            SNew(SWidgetSwitcher)
            .WidgetIndex(Match.IsEmpty() ? 0 : 1)
            + SWidgetSwitcher::Slot()[ SNew(SSpacer) ]
            + SWidgetSwitcher::Slot()[
                SNew(SBorder)
                .Padding(FMargin(1, 0))
                .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                .BorderBackgroundColor(FLinearColor(1.f, 1.f, 0.3f, 1.f))
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(Match))
                    .ColorAndOpacity(FLinearColor::Black)
                ]
            ]
        ]
        + SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock).Text(FText::FromString(After)).ColorAndOpacity(FSlateColor::UseForeground())
        ]
    ];

    // 返回按钮
    return SNew(SButton)
        .ContentPadding(FMargin(20, 2))
        .ButtonStyle(FAppStyle::Get(), "Menu.Button")
        .OnClicked_Lambda([ChangeCheckState, CheckBoxPtr]() {
            if (!CheckBoxPtr.IsValid()) {
                UE_LOG(LogTemp, Error, TEXT("CheckBoxPtr 无效!"));
                return FReply::Handled();
            }
            const bool bIsChecked = CheckBoxPtr->IsChecked();
            const ECheckBoxState CurrentState = bIsChecked ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
            const ECheckBoxState NewState = (CurrentState == ECheckBoxState::Checked)
                                        ? ECheckBoxState::Unchecked : ECheckBoxState::Checked;
            CheckBoxPtr->SetIsChecked(NewState);
            ChangeCheckState(NewState);
            return FReply::Handled();
        })
        [
            RowBox
        ];
}

void SInteractButtonsUtils::UpdateMenuEntries(
	TArray<FMenuItem>* MenuItems,  
	FName* CameraSelectedKey,                    
	TMap<FName, ECheckBoxState>* FolderCheckStates, 
	const FText& FilterText,
	const TSharedPtr<SVerticalBox>& MenuEntriesBox
)
{
	if (!MenuEntriesBox.IsValid() || !MenuItems) return;
	MenuEntriesBox->ClearChildren();
	const FString FilterString = FilterText.ToString();
	if (CameraSelectedKey != nullptr) CameraCheckBoxMap.Empty();

	for (auto& Item : *MenuItems)
	{
		if (FilterText.IsEmpty() || Item.Label.Contains(FilterString, ESearchCase::IgnoreCase))
		{
			MenuEntriesBox->AddSlot()
				.AutoHeight()
				.Padding(0)
				[
					CreateMenuEntryWidget(
						Item.Name,
						Item.Label,
						FilterString,
						CameraSelectedKey,
						FolderCheckStates
					)
				];
		}
	}
}

TSharedRef<SWidget> SInteractButtonsUtils::GenerateDropdownMenu(
	TArray<FMenuItem>* MenuItems,
	FName* CameraSelectedKey,
	TMap<FName, ECheckBoxState>* FolderCheckStates
)
{
	// 搜索框内容、菜单项区域
	FText& FilterText = CameraSelectedKey ? CameraFilterText : FolderFilterText;
	TSharedPtr<SVerticalBox>& MenuEntriesBox = CameraSelectedKey ? CameraMenuEntriesBox : FolderMenuEntriesBox;
	
	// 重置过滤与菜单项容器
	FilterText = FText::GetEmpty();
	MenuEntriesBox = SNew(SVerticalBox);

	// 搜索框内容变化构建菜单
	TSharedRef<SVerticalBox> FullMenu = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(10.0f))
		[
			SNew(SSearchBox)
			.HintText(LOCTEXT("SearchHintStartTyping", "Search..."))
			.SelectAllTextWhenFocused(false)
			.OnTextChanged_Lambda([&, MenuItems, CameraSelectedKey, FolderCheckStates](const FText& InFilterText)
			{
				FilterText = InFilterText;
				if (MenuItems)
				{
					UpdateMenuEntries(
						MenuItems,
						CameraSelectedKey,
						FolderCheckStates,
						FilterText,
						MenuEntriesBox
					);
				}
			})
		]
		+ SVerticalBox::Slot()
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 10.0f))
		[
			MenuEntriesBox.ToSharedRef()
		];

	// 初始构建菜单项
	if (MenuItems)
	{
		UpdateMenuEntries(
			MenuItems,
			CameraSelectedKey,
			FolderCheckStates,
			FilterText,
			MenuEntriesBox
		);
	}
	
	return FullMenu;
}

void SInteractButtonsUtils::GetLevelAllMenuItems(const EToolbarType MenuType, TMap<FName, FMenuItem>& OutMenuItemMap)
{
	OutMenuItemMap.Empty();

	const UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World) return;

	// 临时存储菜单项，用于排序
	TArray<FMenuItem> TempMenuItems;

	if (MenuType == EToolbarType::Camera)
	{
		for (TActorIterator<ACineCameraActor> It(World); It; ++It)
		{
			if (const ACineCameraActor* Camera = *It; Camera && !Camera->IsPendingKillPending()) // Camera 的定义和赋值写在 if 中
			{
				const FName UniqueName = FName(*Camera->GetName()); // 对象有 GetName() 方法
				const FString Label = Camera->GetActorLabel();
				TempMenuItems.Add(FMenuItem(UniqueName, Label)); // 按顺序填写
			}
		}
	}
	else if (MenuType == EToolbarType::Label)
	{
		TSet<FName> FolderPaths;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (const AActor* Actor = *It; Actor && !Actor->IsPendingKillPending())
			{
				if (const FName FolderPath = Actor->GetFolderPath(); !FolderPath.IsNone()) // 文件夹路径没有 GetName() 方法
				{
					FolderPaths.Add(FolderPath);
				}
			}
		}
		for (const FName& FolderPath : FolderPaths)
		{
			FString PathStr = FolderPath.ToString();
			FString Label;
			if (int32 SlashIdx; PathStr.FindLastChar('/', SlashIdx)) // 提取最后一级目录
				Label = PathStr.Mid(SlashIdx + 1);
			else
				Label = PathStr.IsEmpty() ? TEXT("(Root)") : PathStr;

			const FName UniqueName = FName(*PathStr);
			TempMenuItems.Add(FMenuItem(UniqueName, Label));
		}
	}

	// 按 Label 字母序排序
	TempMenuItems.Sort([](const FMenuItem& A, const FMenuItem& B)
	{
		return A.Label < B.Label;
	});

	// 依次加入 Map
	for (const FMenuItem& Item : TempMenuItems)
	{
		OutMenuItemMap.Add(Item.Name, Item);
	}

	// 打印出 OutMenuItemMap 内容
	// UE_LOG(LogTemp, Warning, TEXT("=== OutMenuItemMap 内容如下 ==="));
	// for (const auto& Pair : OutMenuItemMap)
	// {
	// 	const FName& Key = Pair.Key;
	// 	const auto& [Name, Label] = Pair.Value;
	// 	UE_LOG(LogTemp, Log, TEXT("Key: %s, FMenuItem.Name: %s, FMenuItem.Label: %s"), *Key.ToString(),
	// 	       *Name.ToString(), *Label);
	// }
}

TSharedRef<SWidget> SInteractButtonsUtils::BuildToolbar(EToolbarType ToolbarType)
{
	// 定义工具栏构建器和相关变量
	FUIAction ToggleButtonAction;
	FString ToolbarLabel;
	FSlateIcon ToolbarIcon;
	TArray<FMenuItem>* MenuItems;
	FName* CameraSelectedKey = nullptr;  // 单选框
	TMap<FName, ECheckBoxState>* FolderCheckStates = nullptr;  // 复选框

	if (ToolbarType == EToolbarType::Camera)
	{
		// 获取切换按钮的状态
		ToggleButtonAction.GetActionCheckState = FGetActionCheckState::CreateLambda([this]()
		{
			return bIsCameraToggled ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
		});
		ToggleButtonAction.ExecuteAction = FExecuteAction::CreateLambda([this]()
		{
			bIsCameraToggled = !bIsCameraToggled;
		});
		// 获取 CineCameraActor 列表
		GetLevelAllMenuItems(EToolbarType::Camera, CameraMenuItemMap);
		// 设置工具栏标签和图标
		ToolbarLabel = TEXT("Camera");
		ToolbarIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.box-perspective");
		// 载入菜单项、单选框
		for (const auto& Pair : CameraMenuItemMap)
		{
			CameraMenuItems.Add(FMenuItem(Pair.Value.Name, Pair.Value.Label));
		}
		CameraMenuRadioButton = FName();
		// 设置菜单项、单选指针
		MenuItems = &CameraMenuItems;
		CameraSelectedKey = &CameraMenuRadioButton;
	}
	else // Folder
	{
		ToggleButtonAction.GetActionCheckState = FGetActionCheckState::CreateLambda([this]()
		{
			return bIsLabelToggled ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
		});
		ToggleButtonAction.ExecuteAction = FExecuteAction::CreateLambda([this]()
		{
			bIsLabelToggled = !bIsLabelToggled;
		});
		GetLevelAllMenuItems(EToolbarType::Label, FolderMenuItemMap);
		ToolbarLabel = TEXT("Image Label");
		ToolbarIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.pyramid");
		for (const auto& Pair : FolderMenuItemMap)
		{
			FolderMenuItems.Add(FMenuItem(Pair.Value.Name, Pair.Value.Label));
			FolderMenuCheckBox.Add(Pair.Key, ECheckBoxState::Unchecked);
		}
		MenuItems = &FolderMenuItems;
		FolderCheckStates = &FolderMenuCheckBox;
	}

	// Build toolbar
	FSlimHorizontalToolBarBuilder Toolbar(nullptr, FMultiBoxCustomization::None);

	// 切换按钮
	Toolbar.AddToolBarButton(
		ToggleButtonAction,
		NAME_None,
		FText::FromString(ToolbarLabel),
		FText::GetEmpty(),
		ToolbarIcon,
		EUserInterfaceActionType::ToggleButton
	);

	// 下拉按钮
	Toolbar.AddComboButton(
		FUIAction(),
		FOnGetContent::CreateLambda([this, MenuItems, CameraSelectedKey, FolderCheckStates, ToolbarType]()
		{
			return GenerateDropdownMenu(
				MenuItems,
				ToolbarType == EToolbarType::Camera ? CameraSelectedKey : nullptr,
				ToolbarType == EToolbarType::Camera ? nullptr : FolderCheckStates
			);
		}),
		FText::FromString("Dropdown"),
		FText::GetEmpty(),
		ToolbarIcon,
		true
	);
	
	return Toolbar.MakeWidget();
}

TOptional<float> SInteractButtonsUtils::GetLabelingProgress() const
{
	return ProgressPercent;
}

TSharedRef<SWidget> SInteractButtonsUtils::CreateProgressBar()
{
	return SNew(SProgressBar)
		.Percent(this, &SInteractButtonsUtils::GetLabelingProgress);
}

TSharedRef<SWidget> SInteractButtonsUtils::CreateStartLabelingButton()
{
	return SNew(SButton)
		.Content()
		[
			SNew(STextBlock)
			.Justification(ETextJustify::Center)
			.Text(FText::FromString(TEXT("Start Labeling")))
		]
		.ButtonStyle(FAppStyle::Get(), "PrimaryButton")
		.OnClicked(FOnClicked::CreateSP(this, &SInteractButtonsUtils::OnStartLabelingClicked));
}

FReply SInteractButtonsUtils::OnStartLabelingClicked()
{
	// 1. 检查前置条件
	SourcePath = *FileSettingObject->SourceDirectory.Path;
	SavePath = *FileSettingObject->SaveDirectory.Path;
	if (!FPaths::DirectoryExists(SourcePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("请选择有效的源目录路径: %s"), *SourcePath);
		return FReply::Handled();
	}
	if (!FPaths::DirectoryExists(SavePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("请选择有效的保存目录路径:%s"), *SavePath);
		return FReply::Handled();
	}
	
	if (!bIsCameraToggled || !bIsLabelToggled)
	{
		const FString ErrorMsg = FString::Printf(
			TEXT("无法开始标注！需要满足以下条件：\n")
			TEXT("1. Camera切换状态: %s\n")
			TEXT("2. Label切换状态: %s"),
			bIsCameraToggled ? TEXT("已激活") : TEXT("未激活"),
			bIsLabelToggled ? TEXT("已激活") : TEXT("未激活")
		);
		UE_LOG(LogTemp, Warning, TEXT("%s"), *ErrorMsg);
		return FReply::Handled();
	}
	
	if (CameraMenuRadioButton.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("请选择一个 Camera！"));
		return FReply::Handled();
	}
	
	CheckedFolders.Empty();
	for (const auto& Elem : FolderMenuCheckBox)
	{
		if (Elem.Value == ECheckBoxState::Checked)
		{
			CheckedFolders.Add(Elem.Key);
		}
	}
	if (CheckedFolders.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("请至少勾选一个 Folder！"));
		return FReply::Handled();
	}
	
	if (!IsPlayRangeButtonEnabled())
	{
		UE_LOG(LogTemp, Warning, TEXT("Play Range 按钮未启用，无法开始标注！"));
		return FReply::Handled();
	}

	// 2. 初始化标注状态
	CurrentFrame = CurrentStartFrame;
	ProgressPercent = 0.0f;

	// 3. 启动定时器，每隔0.05秒处理一帧
	if (GWorld)
	{
		GWorld->GetTimerManager().SetTimer(
			LabelingTimerHandle,
			FTimerDelegate::CreateRaw(this, &SInteractButtonsUtils::DoLabelingStep),
			0.05f,
			true
		);
	}
	return FReply::Handled();
}

void SInteractButtonsUtils::DoLabelingStep()
{
	if (CurrentFrame > CurrentEndFrame)
	{
		// 结束，清除定时器，进度条满
		if (GWorld)
		{
			GWorld->GetTimerManager().ClearTimer(LabelingTimerHandle);
		}
		ProgressPercent = 1.0f;
		UE_LOG(LogTemp, Warning, TEXT("标注完成! 总帧数: %d"), CurrentEndFrame - CurrentStartFrame + 1);
		return;
	}

	// 1. 设置 Sequencer 到当前帧
	const FFrameTime FrameTime(CurrentFrame);
	const FFrameNumber InternalFrame = FFrameRate::TransformTime(FrameTime, DisplayRate, TickResolution).FloorToFrame();
	if (const TSharedPtr<ISequencer> Sequencer = GetActiveSequencer())
	{
		Sequencer->SetLocalTime(InternalFrame.Value);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Sequencer无效，无法设置帧！"));
		++CurrentFrame;
		return;
	}

	// 2. 标注逻辑
	UE_LOG(LogTemp, Warning, TEXT("==== Frame: %d ===="), CurrentFrame);
	FSequenceAlgorithmUtils AlgorithmUtils;
	FCineCameraData CameraData;
	TArray<FStaticMeshData> MeshArray;
	AlgorithmUtils.CollectCheckedCineCameraData(CameraMenuRadioButton, CameraData);
	AlgorithmUtils.CollectCheckedStaticMeshData(CheckedFolders, MeshArray);
	AlgorithmUtils.ProjectMeshVerticesToCameraPlane(CameraData, MeshArray);
	AlgorithmUtils.CollectCameraSequenceFrameMetadata(SourcePath, SavePath);

	// 3. 更新进度百分比
	ProgressPercent = static_cast<float>(CurrentFrame - CurrentStartFrame + 1) / static_cast<float>(CurrentEndFrame - CurrentStartFrame + 1);

	// 4. 下一帧
	++CurrentFrame;
}


