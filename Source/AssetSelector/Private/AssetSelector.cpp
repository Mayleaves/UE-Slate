// Private/AssetSelector.cpp

#include "AssetSelector.h"

void SAssetSelector::Construct(const FArguments& InArgs)
{
    OnGenerateButtonClicked = InArgs._OnGenerateButtonClicked;

    ChildSlot
    [
        SNew(SVerticalBox)
        
        + SVerticalBox::Slot()
        .Padding(10.0f)
        .AutoHeight()
        [
            SNew(SButton)
            .OnClicked(OnGenerateButtonClicked)
            .Text(FText::FromString("Generate Data"))
        ]
        
    ];
}

