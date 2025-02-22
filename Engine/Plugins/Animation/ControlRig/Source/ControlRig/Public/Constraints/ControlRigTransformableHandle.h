﻿// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "TransformableHandle.h"

#include "ControlRigTransformableHandle.generated.h"

class UControlRig;
class USkeletalMeshComponent;
struct FRigControlElement;

/**
 * UTransformableControlHandle
 */

UCLASS()
class CONTROLRIG_API UTransformableControlHandle : public UTransformableHandle 
{
	GENERATED_BODY()
	
public:
	
	virtual ~UTransformableControlHandle();

	/** @todo document */
	virtual bool IsValid() const override;

	/** @todo document */
	virtual void SetGlobalTransform(const FTransform& InGlobal) const override;
	/** @todo document */
	virtual void SetLocalTransform(const FTransform& InLocal) const override;
	/** @todo document */
	virtual FTransform GetGlobalTransform() const  override;
	/** @todo document */
	virtual FTransform GetLocalTransform() const  override;

	/** @todo document */
	virtual UObject* GetPrerequisiteObject() const override;
	/** @todo document */
	virtual FTickFunction* GetTickFunction() const override;

	/** @todo document */
	virtual uint32 GetHash() const override;

#if WITH_EDITOR
	/** @todo document */
	virtual FName GetName() const override;
#endif

	/** @todo document */
	UPROPERTY()
	TWeakObjectPtr<UControlRig> ControlRig;
	
	/** @todo document */
	UPROPERTY()
	FName ControlName;
	
private:

	/** @todo document */
	USkeletalMeshComponent* GetSkeletalMesh() const;

	/** @todo document */
	FRigControlElement* GetControlElement() const;
};