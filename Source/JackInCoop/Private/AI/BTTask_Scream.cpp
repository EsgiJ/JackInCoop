// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_Scream.h"

#include "AI/Zombie.h"
#include "AI/ZombieAIController.h"

EBTNodeResult::Type UBTTask_Scream::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AZombieAIController* MyController = Cast<AZombieAIController>(OwnerComp.GetAIOwner());
	if (MyController == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	AZombie* ZombieCharacter = Cast<AZombie>(MyController->GetPawn());
	if (ZombieCharacter)
	{
		ZombieCharacter->Scream();
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}
