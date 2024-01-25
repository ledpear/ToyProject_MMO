// Fill out your copyright notice in the Description page of Project Settings.


#include "SelfPlayerConnectServerComponent.h"
#include "IocpSocketController.h"

// Sets default values for this component's properties
USelfPlayerConnectServerComponent::USelfPlayerConnectServerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void USelfPlayerConnectServerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

}


// Called every frame
void USelfPlayerConnectServerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void USelfPlayerConnectServerComponent::SyncLocation(const float DeltaTime)
{
	//위치 표시
	const FVector& actorLocation = Super::GetOwner()->GetActorLocation();
	const FString logText = FString::Printf(TEXT("[Location] X : %.3f, Y : %.3f, Z : %.3f"), actorLocation.X, actorLocation.Y, actorLocation.Z);
	GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, logText);
	prevDeltaTime = DeltaTime;

	//위치 Send
	IocpSocketController iocpSocketController;
}