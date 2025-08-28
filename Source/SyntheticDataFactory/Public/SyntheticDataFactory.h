// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MoviePipelineQueue.h"
#include "Modules/ModuleManager.h"
#include "Editor/Sequencer/Public/ISequencerModule.h"
/**
 * 插件的主模块类，继承自 IModuleInterface，负责插件的初始化和生命周期管理。
 */

class FSyntheticDataFactoryModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();

	// TSharedPtr<FEditorInterfaceWithMoviePipeline> EditorInterfaceWithMoviePipelineWidget;

private:
	void RegisterMenus();

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

private:
	TSharedPtr<class FUICommandList> PluginCommands;

private:
	UPROPERTY() // 宏，标记反射属性
	UMoviePipelineExecutorJob* CurrentJob; // 当前 Sequencer 下的 Job

	int32 StartFrame = 0;
	int32 EndFrame = 0;
	FFrameRate DisplayRate; // 自动初始化为1/1（1fps）
	FFrameRate TickResolution; // 自动初始化为1/1（1fps）

	UPROPERTY()
	UMoviePipelineQueue* PipelineQueue; // 队列

	// 初始化、更新
	void InitializePipelineQueue();
	void UpdateCurrentJob(); // 获取 Level Sequence、Map、Playpack Rang

	// 保存当前 Sequencer 指针
	TWeakPtr<ISequencer> LastSequencer;
	FDelegateHandle SequencerCreatedHandle;

	// 监听 Sequencer 创建事件
	void OnSequencerCreated(TSharedRef<ISequencer> InSequencer);
};
