// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_FindClosestDefense.h"

#include "AI/Zombie.h"
#include "AI/ZombieAIController.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"

EBTNodeResult::Type UBTTask_FindClosestDefense::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AZombieAIController* MyController = Cast<AZombieAIController>(OwnerComp.GetAIOwner());
	if (MyController == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	AZombie* ZombieCharacter = Cast<AZombie>(MyController->GetPawn());
	if (ZombieCharacter)
	{
		if(ZombieCharacter->FindNearbyWall())
		{
			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}
