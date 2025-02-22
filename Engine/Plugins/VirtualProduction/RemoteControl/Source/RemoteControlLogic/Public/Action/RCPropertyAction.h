﻿// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "RCAction.h"
#include "RCPropertyAction.generated.h"

class URCVirtualPropertySelfContainer;

/**
 * Property Action which is set property value from the action to exposed property
 */
UCLASS()
class REMOTECONTROLLOGIC_API URCPropertyAction : public URCAction
{
	GENERATED_BODY()

public:
	URCPropertyAction();
	
	//~ Begin URCAction interface
	virtual void Execute() const override;
	//~ End URCAction interface

public:
	/** Virtual Property Container */
	UPROPERTY()
	TObjectPtr<URCVirtualPropertySelfContainer> PropertySelfContainer = nullptr;
};
