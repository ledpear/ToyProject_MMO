// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ConnectServerComponent.h"
#include "SelfPlayerConnectServerComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Network), meta = (BlueprintSpawnableComponent))
class PROJECTMMO_CLIENT_API USelfPlayerConnectServerComponent : public UConnectServerComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USelfPlayerConnectServerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	virtual void SyncLocation (const float DeltaTime) override;
	float prevDeltaTime = 0.f;
};
