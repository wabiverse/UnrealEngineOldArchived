﻿// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"

#include "ConstraintsActor.generated.h"

class UConstraintsManager;

UCLASS(NotPlaceable)
class CONSTRAINTS_API AConstraintsActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AConstraintsActor(const FObjectInitializer& ObjectInitializer);
	virtual ~AConstraintsActor();

	virtual void BeginDestroy() override;
	virtual void Destroyed() override;

	virtual void PostRegisterAllComponents() override;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY()
	TObjectPtr<UConstraintsManager> ConstraintsManager = nullptr;
	
private:
	void RegisterConstraintsTickFunctions() const;
};
