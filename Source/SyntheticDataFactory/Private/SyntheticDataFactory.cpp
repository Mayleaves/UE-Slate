// Copyright Epic Games, Inc. All Rights Reserved.
/**
 * 插件的初始化、关闭、菜单注册和选项卡管理等核心功能。
 */

#include "SyntheticDataFactory.h"
#include "SyntheticDataFactoryStyle.h"
#include "SyntheticDataFactoryCommands.h"
#include "InteractButtonsUtils.h"

#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "Editor/Sequencer/Public/ISequencerModule.h"
#include "LevelSequence.h"  // 为 ULevelSequence
#include "MoviePipelineQueue.h" // 为 UMoviePipelineQueue
#include "ISequencer.h"
#include "MovieScene.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"

static const FName SyntheticDataFactoryTabName("SyntheticDataFactory");

#define LOCTEXT_NAMESPACE "FSyntheticDataFactoryModule"

void FSyntheticDataFactoryModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	// 初始化样式，类似 css
	FSyntheticDataFactoryStyle::Initialize();
	FSyntheticDataFactoryStyle::ReloadTextures();

	// 注册命令
	FSyntheticDataFactoryCommands::Register();

	// 映射按钮动作
	PluginCommands = MakeShareable(new FUICommandList);
	PluginCommands->MapAction(
		FSyntheticDataFactoryCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FSyntheticDataFactoryModule::PluginButtonClicked),
		FCanExecuteAction());

	// 注册菜单
	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FSyntheticDataFactoryModule::RegisterMenus));

	// 注册选项卡
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(SyntheticDataFactoryTabName,
	                                                  FOnSpawnTab::CreateRaw(
		                                                  this, &FSyntheticDataFactoryModule::OnSpawnPluginTab))
	                        .SetDisplayName(LOCTEXT("FSyntheticDataFactoryTabTitle", "Synthetic Data Factory"))
	                        .SetMenuType(ETabSpawnerMenuType::Hidden)
	                        // Set icons for tabs.
	                        .SetIcon(FSlateIcon(FSyntheticDataFactoryStyle::GetStyleSetName(),
	                                            "SyntheticDataFactory.OpenPluginWindow"));

	// todo 以下部分为新增内容
	// 初始化 PipelineQueue
	InitializePipelineQueue();

	// 注册监听 Sequencer 创建事件
	ISequencerModule& SequencerModule = FModuleManager::LoadModuleChecked<ISequencerModule>("Sequencer");
	SequencerCreatedHandle = SequencerModule.RegisterOnSequencerCreated(
		FOnSequencerCreated::FDelegate::CreateRaw(this, &FSyntheticDataFactoryModule::OnSequencerCreated)
	);
}

void FSyntheticDataFactoryModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FSyntheticDataFactoryStyle::Shutdown();

	FSyntheticDataFactoryCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(SyntheticDataFactoryTabName);

	// todo 以下部分为新增内容
	// 注销监听
	if (ISequencerModule* SequencerModule = FModuleManager::GetModulePtr<ISequencerModule>("Sequencer"))
	{
		SequencerModule->UnregisterOnSequencerCreated(SequencerCreatedHandle);
	}
}

void FSyntheticDataFactoryModule::InitializePipelineQueue()
{
	// 创建新的Queue对象
	PipelineQueue = NewObject<UMoviePipelineQueue>();
	check(PipelineQueue);
}

void FSyntheticDataFactoryModule::UpdateCurrentJob()
{
	// 确保有效的 Sequencer
	const TSharedPtr<ISequencer> Sequencer = LastSequencer.Pin();
	if (!Sequencer.IsValid())
	{
		CurrentJob = nullptr;
		return;
	}

	// 获取当前Sequence
	UObject* SequenceObj = Sequencer->GetRootMovieSceneSequence();
	const ULevelSequence* CurrentSequence = Cast<ULevelSequence>(SequenceObj);
	if (!CurrentSequence)
	{
		CurrentJob = nullptr;
		return;
	}

	// 设置 Job 属性
	if (!CurrentJob) CurrentJob = NewObject<UMoviePipelineExecutorJob>();

	CurrentJob->JobName = FPaths::GetBaseFilename(CurrentSequence->GetPathName());
	CurrentJob->Sequence = CurrentSequence;

	// 获取当前编辑器世界
	const UWorld* CurrentWorld = nullptr;

	// 通过 GEditor 获取编辑器世界
	if (GEditor) CurrentWorld = GEditor->GetEditorWorldContext().World();

	// 设置地图路径
	if (CurrentWorld)
	{
		const FString WorldPath = CurrentWorld->GetPathName();
		CurrentJob->Map = FSoftObjectPath(WorldPath);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("无法获取当前世界！"));
	}

	// 获取播放范围的显示帧数
	if (const UMovieScene* MovieScene = CurrentSequence->GetMovieScene())
	{
		DisplayRate = MovieScene->GetDisplayRate();
		TickResolution = MovieScene->GetTickResolution();
		const TRange<FFrameNumber> PlaybackRange = MovieScene->GetPlaybackRange();

		const FFrameNumber StartInternalFrame = PlaybackRange.GetLowerBoundValue();
		const FFrameNumber EndInternalFrame = PlaybackRange.GetUpperBoundValue();

		const FFrameNumber StartDisplayFrame = FFrameRate::TransformTime(FFrameTime(StartInternalFrame), TickResolution,
		                                                           DisplayRate).FloorToFrame();
		const FFrameNumber EndDisplayFrame = FFrameRate::TransformTime(FFrameTime(EndInternalFrame), TickResolution,
		                                                         DisplayRate).FloorToFrame();

		StartFrame = StartDisplayFrame.Value;
		EndFrame = EndDisplayFrame.Value;
	}
}

void FSyntheticDataFactoryModule::OnSequencerCreated(TSharedRef<ISequencer> InSequencer)
{
	LastSequencer = InSequencer;

	// Sequencer改变时更新CurrentJob
	UpdateCurrentJob();
}

TSharedRef<SDockTab> FSyntheticDataFactoryModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	/*return SNew(SDockTab)
		//.TabRole(ETabRole::NomadTab)
		//.ContentPadding(0)
		//.ForegroundColor(FSlateColor::UseStyle())
		//[

		//]
		;*/

	// 确保CurrentJob是最新的
	UpdateCurrentJob();
	return SNew(SDockTab)
		[
			SNew(SInteractButtonsUtils)
				.DefaultJob(CurrentJob) // 传递当前 Sequencer 中的 Job
				.CurrentStartFrame(StartFrame)
				.CurrentEndFrame(EndFrame)
				.DisplayRate(DisplayRate)
				.TickResolution(TickResolution)
				.CurrentSequencer(LastSequencer) // 传递当前活跃 Sequencer 指针
		];
}

void FSyntheticDataFactoryModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(SyntheticDataFactoryTabName);
}

/**
 * 在 Sequencer 工具的工具栏上添加了一个按钮，点击该按钮会触发 OpenPluginWindow 命令，打开插件的主窗口
 */
void FSyntheticDataFactoryModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	/*{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FSyntheticDataFactoryCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}*/

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("Sequencer.MainToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("SDG");
			{
				FToolMenuEntry& Entry = Section.AddEntry(
					FToolMenuEntry::InitToolBarButton(FSyntheticDataFactoryCommands::Get().OpenPluginWindow));
				// Hide the name of the button.
				Entry.StyleNameOverride = "SequencerToolbar";
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSyntheticDataFactoryModule, SyntheticDataFactory)
