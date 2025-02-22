﻿// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "OptimusNode_ComputeKernelBase.h"
#include "OptimusBindingTypes.h"

#include "OptimusNode_ComputeKernelFunction.generated.h"


UCLASS()
class UOptimusNode_ComputeKernelFunctionGeneratorClass :
	public UClass
{
	GENERATED_BODY()
public:
	static UClass *CreateNodeClass(
		UObject* InPackage,
		FName InCategory,
		const FString& InKernelName,
		FIntVector InGroupSize,
		const TArray<FOptimusParameterBinding>& InInputBindings,
		const TArray<FOptimusParameterBinding>& InOutputBindings,
		const FString& InShaderSource
		);

	// UClass overrides
	void InitPropertiesFromCustomList(uint8* InObjectPtr, const uint8* InCDOPtr) override;
	void Link(FArchive& Ar, bool bRelinkExistingProperties) override;
	
	UPROPERTY()
	FName Category;

	UPROPERTY()
	FString KernelName;

	UPROPERTY()
	FIntVector GroupSize;

	UPROPERTY()
	TArray<FOptimusParameterBinding> InputBindings;

	UPROPERTY()
	TArray<FOptimusParameterBinding> OutputBindings;

	UPROPERTY()
	FString ShaderSource;
};


/**
 * 
 */
UCLASS(Hidden)
class OPTIMUSCORE_API UOptimusNode_ComputeKernelFunction :
	public UOptimusNode_ComputeKernelBase
{
	GENERATED_BODY()

public:
	UOptimusNode_ComputeKernelFunction();

	
	// UOptimusNode overrides
	FText GetDisplayName() const override;
	FName GetNodeCategory() const override; 

	// UOptimusNode_ComputeKernelBase overrides
	FString GetKernelName() const override;
	FIntVector GetGroupSize() const override;
	FString GetKernelSourceText() const override;

	void ConstructNode() override;

	// IOptiusComputeKernelProvider overrides
	void SetCompilationDiagnostics(
		const TArray<FOptimusCompilerDiagnostic>& InDiagnostics
		) override;
	
private:
	UOptimusNode_ComputeKernelFunctionGeneratorClass *GetGeneratorClass() const;
};
