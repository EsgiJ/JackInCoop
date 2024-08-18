// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ZombieAIController.generated.h"

class ABuildable;
class AZombieWayPoint;
class AShooterCharacter;
enum class EZombieBehavior : uint8;
class UBehaviorTreeComponent;
/**
 * 
 */
UCLASS()
class JACKINCOOP_API AZombieAIController : public AAIController
{
	GENERATED_BODY()

	AZombieAIController(const class FObjectInitializer& ObjectInitializer);
	
	virtual void OnPossess(APawn* InPawn) override;

	virtual void OnUnPossess() override;

	UBehaviorTreeComponent* BehaviorTreeComp;

	UBlackboardComponent* BlackboardComp;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName TargetEnemyKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName PatrolLocationKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName CurrentWaypointKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName ZombieBehaviorTypeKeyName;
	
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName SecondsToWaitKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName TargetWallKeyName;
public:

	AActor* GetWaypoint() const;

	AShooterCharacter* GetTargetEnemy() const;

	void SetWaypoint(AActor* NewWaypoint);

	void SetTargetEnemy(APawn* NewWallTarget);

	void SetTargetWall(ABuildable* NewTarget);

	void SetBlackboardZombieBehaviorType(EZombieBehavior NewType);

	void SetSecondsToWait(float SecondsToWait);

	/** Returns BehaviorComp subobject **/
	FORCEINLINE UBehaviorTreeComponent* GetBehaviorComp() const { return BehaviorTreeComp; }

	FORCEINLINE UBlackboardComponent* GetBlackboardComp() const { return BlackboardComp; }
};
