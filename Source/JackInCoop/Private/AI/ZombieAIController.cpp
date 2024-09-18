// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/ZombieAIController.h"
#include "Buildable.h"
#include "ShooterCharacter.h"
#include "AI/Zombie.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

AZombieAIController::AZombieAIController(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	BehaviorTreeComp = ObjectInitializer.CreateDefaultSubobject<UBehaviorTreeComponent>(this, TEXT("BehaviorTreeComponent"));
	BlackboardComp = ObjectInitializer.CreateDefaultSubobject<UBlackboardComponent>(this, TEXT("BlackboardComponent"));

	/* Match with the AI/ZombieBlackboard */
	PatrolLocationKeyName = "PatrolLocation";
	CurrentWaypointKeyName = "CurrentWaypoint";
	ZombieBehaviorTypeKeyName = "ZombieBehaviorType";
	TargetEnemyKeyName = "TargetEnemy";
	SecondsToWaitKeyName = "SecondsToWait";
	TargetWallKeyName = "TargetWall";

	/* Initializes PlayerState so we can assign a team index to AI */
	bWantsPlayerState = true;
}

void AZombieAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AZombie* ZombieCharacter = Cast<AZombie>(InPawn);
	if (ZombieCharacter)
	{
		if (ensure(ZombieCharacter->BehaviorTree->BlackboardAsset))
		{
			BlackboardComp->InitializeBlackboard(*ZombieCharacter->BehaviorTree->BlackboardAsset);
		}
		BehaviorTreeComp->StartTree(*ZombieCharacter->BehaviorTree);
		/* Make sure the Blackboard has the type of bot we possessed */
		SetBlackboardZombieBehaviorType(ZombieCharacter->ZombieBehaviorType);
	}
}

void AZombieAIController::OnUnPossess()
{
	Super::OnUnPossess();
	
	/* Stop any behavior running as we no longer have a pawn to control */
	BehaviorTreeComp->StopTree();
}

void AZombieAIController::SetWaypoint(AActor* NewWaypoint)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsObject(CurrentWaypointKeyName, NewWaypoint);
	}
}


void AZombieAIController::SetTargetEnemy(APawn* NewTarget)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsObject(TargetEnemyKeyName, NewTarget);
	}
}

void AZombieAIController::SetTargetWall(ABuildable* NewWallTarget)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsObject(TargetWallKeyName, NewWallTarget);
	}
}


AActor* AZombieAIController::GetWaypoint() const
{
	if (BlackboardComp)
	{
		return Cast<AActor>(BlackboardComp->GetValueAsObject(CurrentWaypointKeyName));
	}

	return nullptr;
}


AShooterCharacter* AZombieAIController::GetTargetEnemy() const
{
	if (BlackboardComp)
	{
		return Cast<AShooterCharacter>(BlackboardComp->GetValueAsObject(TargetEnemyKeyName));
	}

	return nullptr;
}


void AZombieAIController::SetBlackboardZombieBehaviorType(EZombieBehavior NewType)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsEnum(ZombieBehaviorTypeKeyName, (uint8)NewType);
	}
}

void AZombieAIController::SetSecondsToWait(float SecondsToWait)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsFloat(SecondsToWaitKeyName, SecondsToWait);
	}
}
