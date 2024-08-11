// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_Attack.h"
#include "BehaviorTree/BlackboardComponent.h"
/* This contains includes all key types like UBlackboardKeyType_Vector used below. */
#include "AI/Zombie.h"
#include "AI/ZombieAIController.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AZombieAIController* MyController = Cast<AZombieAIController>(OwnerComp.GetAIOwner());
	if (MyController == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	AZombie* ZombieCharacter = Cast<AZombie>(MyController->GetPawn());
	if (ZombieCharacter)
	{
		ZombieCharacter->AttackTarget();
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}
