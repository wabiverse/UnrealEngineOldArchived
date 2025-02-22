﻿// Copyright Epic Games, Inc. All Rights Reserved.

#include "Util/TakeSnapshotUtil.h"

#include "Data/LevelSnapshotsEditorData.h"
#include "LevelSnapshot.h"
#include "LevelSnapshotsEditorSettings.h"
#include "LevelSnapshotsEditorModule.h"
#include "LevelSnapshotsEditorStyle.h"
#include "LevelSnapshotsLog.h"
#include "Widgets/SLevelSnapshotsEditorCreationForm.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "AssetViewUtils.h"
#include "IAssetTools.h"
#include "Editor.h"
#include "Engine/World.h"
#include "ObjectTools.h"
#include "Framework/Notifications/NotificationManager.h"
#include "GameplayMediaEncoder/Private/GameplayMediaEncoderCommon.h"
#include "Misc/FeedbackContext.h"
#include "Misc/MessageDialog.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "LevelSnapshotEditorLibrary"

namespace UE::LevelSnapshots::Editor
{
	TAutoConsoleVariable<bool> CVarSaveAsync(TEXT("LevelSnapshots.SnapshotFormUsesSaveAsync"), true, TEXT("Set whether the Take Snapshot Form, opened by pressing the toolbar icon, saves snapshots using the SAVE_Async flag."));
	
	static void HandleFormReply(const FText& InDescription, bool bShouldUseOverrides)
	{
		UWorld* World = ULevelSnapshotsEditorData::GetEditorWorld();
		if (!ensure(World))
		{
			return;
		}
		
		ULevelSnapshotsEditorSettings* DataManagementSettings = ULevelSnapshotsEditorSettings::GetMutable();
		DataManagementSettings->ValidateRootLevelSnapshotSaveDirAsGameContentRelative();
		DataManagementSettings->SanitizeAllProjectSettingsPaths(true);

		const FText& NewSnapshotDir = ULevelSnapshotsEditorSettings::ParseLevelSnapshotsTokensInText(
			FText::FromString(FPaths::Combine(DataManagementSettings->RootLevelSnapshotSaveDir.Path, DataManagementSettings->LevelSnapshotSaveDir)),
			World->GetName()
			);
		const FString& DescriptionString = bShouldUseOverrides && DataManagementSettings->IsNameOverridden() ?	DataManagementSettings->GetNameOverride() : DataManagementSettings->DefaultLevelSnapshotName;
		const FText& NewSnapshotName = ULevelSnapshotsEditorSettings::ParseLevelSnapshotsTokensInText(
				FText::FromString(DescriptionString),
				World->GetName()
				);

		const FString& ValidatedName = FPaths::MakeValidFileName(NewSnapshotName.ToString());

		const bool bSaveAsync = CVarSaveAsync.GetValueOnAnyThread();
		SnapshotEditor::TakeLevelSnapshotAndSaveToDisk(World, ValidatedName, NewSnapshotDir.ToString(), InDescription.ToString(), false, bSaveAsync);
	}
	
	static void DestroySnapshot(ULevelSnapshot* SnapshotAsset)
	{
		SnapshotAsset->ClearFlags(RF_Public | RF_Standalone);
		SnapshotAsset->Rename(nullptr, GetTransientPackage());
		SnapshotAsset->MarkAsGarbage();
		SnapshotAsset->RemoveFromRoot();
	}
}

void SnapshotEditor::TakeSnapshotWithOptionalForm()
{
	FLevelSnapshotsEditorModule& Module = FLevelSnapshotsEditorModule::Get();
	if (ULevelSnapshotsEditorSettings::Get()->bUseCreationForm)
	{
		TSharedRef<SWidget> CreationForm = SLevelSnapshotsEditorCreationForm::MakeAndShowCreationWindow(
			FCloseCreationFormDelegate::CreateLambda([](const FText& Description)
			{
				UE::LevelSnapshots::Editor::HandleFormReply(Description, true);
			}));
	}
	else
	{
		UE::LevelSnapshots::Editor::HandleFormReply(FText::GetEmpty(), false);
	}
}

ULevelSnapshot* SnapshotEditor::TakeLevelSnapshotAndSaveToDisk(UWorld* World, const FString& FileName, const FString& FolderPath, const FString& Description, bool bShouldCreateUniqueFileName, bool bSaveAsync)
{
	SCOPED_SNAPSHOT_EDITOR_TRACE(CreateSnapshotPackage);
	FString AssetName = FileName;
    	
    FString BasePackageName = FPaths::Combine(FolderPath, FileName);

    BasePackageName.RemoveFromStart(TEXT("/"));
    BasePackageName.RemoveFromStart(TEXT("Content/"));
    BasePackageName.StartsWith(TEXT("Game/")) == true ? BasePackageName.InsertAt(0, TEXT("/")) : BasePackageName.InsertAt(0, TEXT("/Game/"));

    FString PackageName = BasePackageName;

    if (bShouldCreateUniqueFileName)
    {
    	IAssetTools& AssetTools = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
    	
    	AssetTools.CreateUniqueAssetName(BasePackageName, TEXT(""), PackageName, AssetName);
    }

    const FString& PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
    const bool bPathIsValid = FPaths::ValidatePath(PackageFileName);

    bool bProceedWithSave = bPathIsValid;

    if (bProceedWithSave && !bShouldCreateUniqueFileName)
    {
    	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    	FAssetData ExistingAsset = AssetRegistryModule.Get().GetAssetByObjectPath(FName(*(BasePackageName + "." + FileName)));
    	if ( ExistingAsset.IsValid() && ExistingAsset.AssetClassPath == ULevelSnapshot::StaticClass()->GetClassPathName() )
    	{
    		EAppReturnType::Type ShouldReplace = FMessageDialog::Open( EAppMsgType::YesNo, FText::Format(LOCTEXT("ReplaceAssetMessage", "{0} already exists. Do you want to replace it?"), FText::FromString(FileName)) );
    		bProceedWithSave = (ShouldReplace == EAppReturnType::Yes);
    	}
    }

    if (bProceedWithSave)
    {
    	// Show notification
    	FNotificationInfo Notification(NSLOCTEXT("LevelSnapshots", "NotificationFormatText_CreatingSnapshot", "Creating Level Snapshot"));
    	Notification.Image = FLevelSnapshotsEditorStyle::GetBrush(TEXT("LevelSnapshots.ToolbarButton"));
    	Notification.bUseThrobber = true;
    	Notification.bUseSuccessFailIcons = true;
    	Notification.ExpireDuration = 2.f;
    	Notification.bFireAndForget = false;
    	
    	TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Notification);
    	NotificationItem->SetCompletionState(SNotificationItem::CS_Pending);
    	ON_SCOPE_EXIT
    	{
    		NotificationItem->ExpireAndFadeout();
    	};

    	UPackage* SavePackage = CreatePackage(*PackageName);
    	const EObjectFlags AssetFlags = RF_Public | RF_Standalone;
    	ULevelSnapshot* SnapshotAsset = NewObject<ULevelSnapshot>(SavePackage, *AssetName, AssetFlags, nullptr);

    	check(SavePackage && SnapshotAsset);
    	
    	SnapshotAsset->SetSnapshotName(*FileName);
    	SnapshotAsset->SetSnapshotDescription(Description);

        const bool bSuccessful = SnapshotAsset->SnapshotWorld(World);
        if (!bSuccessful)
        {
            // Package and snapshot need to be destroyed again
            UE::LevelSnapshots::Editor:: DestroySnapshot(SnapshotAsset);
            ObjectTools::DeleteObjectsUnchecked({ SavePackage });
            
            NotificationItem->SetText(
                NSLOCTEXT("LevelSnapshots", "NotificationFormatText_NoSnapshotTaken", "No snapshot was taken")
                );
            NotificationItem->SetCompletionState(SNotificationItem::CS_Fail);
            return nullptr;
        }
    	SnapshotAsset->MarkPackageDirty();

    	FAssetRegistryModule::AssetCreated(SnapshotAsset);
    	GenerateThumbnailForSnapshotAsset(SnapshotAsset);

    	bool bSavingSuccessful;
    	{
    		SCOPED_SNAPSHOT_EDITOR_TRACE(SaveSnapshotPackage);
			FSavePackageArgs SaveArgs;
			SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
			SaveArgs.Error = GWarn;
			SaveArgs.bWarnOfLongFilename = false;
			SaveArgs.SaveFlags = bSaveAsync ? SAVE_Async | SAVE_NoError : SAVE_None;
			bSavingSuccessful = UPackage::SavePackage(SavePackage, SnapshotAsset, *PackageFileName, SaveArgs);
    	}
    	
    	// Notify the user of the outcome
    	if (bPathIsValid && bSavingSuccessful)
    	{
    		NotificationItem->SetText(
    			FText::Format(
    				NSLOCTEXT("LevelSnapshots", "NotificationFormatText_CreateSnapshotSuccess", "Successfully created Level Snapshot \"{0}\""), FText::FromString(FileName)));
    		NotificationItem->SetCompletionState(SNotificationItem::CS_Success);
    	}
    	else
    	{
    		NotificationItem->SetText(
    			FText::Format(
    				NSLOCTEXT("LevelSnapshots", "NotificationFormatText_CreateSnapshotFailure", "Failed to create Level Snapshot \"{0}\". Check the file name."), FText::FromString(FileName)));
    		NotificationItem->SetCompletionState(SNotificationItem::CS_Fail);
    	}

    	return SnapshotAsset;
    }

    return nullptr;
}

void SnapshotEditor::GenerateThumbnailForSnapshotAsset(ULevelSnapshot* Snapshot)
{
	if (!ensure(Snapshot))
	{
		return;
	}
	
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	TArray<FAssetData> SnapshotAssetData;
	AssetRegistry.GetAssetsByPackageName(FName(*Snapshot->GetPackage()->GetPathName()), SnapshotAssetData);
	
	if (ensureMsgf(SnapshotAssetData.Num(), TEXT("Failed to find asset data for asset we just saved. Investigate!")))
	{
		// Copied from FAssetFileContextMenu::ExecuteCaptureThumbnail
		FViewport* Viewport = GEditor->GetActiveViewport();

		if (ensure(GCurrentLevelEditingViewportClient) && ensure(Viewport))
		{
			//have to re-render the requested viewport
			FLevelEditorViewportClient* OldViewportClient = GCurrentLevelEditingViewportClient;
			//remove selection box around client during render
			GCurrentLevelEditingViewportClient = NULL;
			Viewport->Draw();

			AssetViewUtils::CaptureThumbnailFromViewport(Viewport, SnapshotAssetData);

			//redraw viewport to have the yellow highlight again
			GCurrentLevelEditingViewportClient = OldViewportClient;
			Viewport->Draw();
		}
	}
}

#undef LOCTEXT_NAMESPACE
