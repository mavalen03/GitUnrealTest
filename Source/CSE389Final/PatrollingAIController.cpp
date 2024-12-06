// Fill out your copyright notice in the Description page of Project Settings.


#include "PatrollingAIController.h"
#include "NavigationSystem.h"

void APatrollingAIController::BeginPlay() {

	Super::BeginPlay();

	NavArea = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);

	RandomPatrol();
}

void APatrollingAIController::RandomPatrol() {

	//check if the nav area was succesfully created
	if (NavArea) {
		NavArea->K2_GetRandomReachablePointInRadius(GetWorld(), GetPawn()->GetActorLocation(), RandomLocation, 15000.0f);

		MoveToLocation(RandomLocation);
	}
}

