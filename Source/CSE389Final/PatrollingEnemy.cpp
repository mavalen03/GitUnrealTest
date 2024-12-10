// Fill out your copyright notice in the Description page of Project Settings.


#include "PatrollingEnemy.h"
#include "ControllableCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "PatrollingAIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "AITypes.h"


// Sets default values
APatrollingEnemy::APatrollingEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PlayerCollisionDetection = CreateDefaultSubobject<USphereComponent>(TEXT("Player Collision Detection"));
	PlayerCollisionDetection->SetupAttachment(RootComponent);

	PlayerAttackDetection = CreateDefaultSubobject<UBoxComponent>(TEXT("Player Attack Detection"));
	PlayerAttackDetection->SetupAttachment(GetMesh(), TEXT("head"));

	// Initialize the audio component and attach it to the root component
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(RootComponent);
	AudioComponent->bAutoActivate = false; // Do not play sound automatically on start

	PlayerDetected = false;
	CanAttackPlayer = false;

}

// Called when the game starts or when spawned
void APatrollingEnemy::BeginPlay()
{
	Super::BeginPlay();

	//give our controller to our patrollingAIController class and save the reference
	AIController = Cast<APatrollingAIController>(GetController());
	//when we've completed any path following, call OnAIMoveCompleted
	AIController->GetPathFollowingComponent()->OnRequestFinished.AddUObject(this, &APatrollingEnemy::OnAIMoveCompleted);

	//Add links to functions so that our Enemy will react to the following events
	PlayerCollisionDetection->OnComponentBeginOverlap.AddDynamic(this, &APatrollingEnemy::OnPlayerDetectedOverlapBegin);
	PlayerCollisionDetection->OnComponentEndOverlap.AddDynamic(this, &APatrollingEnemy::OnPlayerDetectedOverlapEnd);
	PlayerAttackDetection->OnComponentBeginOverlap.AddDynamic(this, &APatrollingEnemy::OnPlayerAttackOverlapBegin);
	PlayerAttackDetection->OnComponentEndOverlap.AddDynamic(this, &APatrollingEnemy::OnPlayerAttackOverlapEnd);
	//DamageCollision->OnComponentBeginOverlap.AddDynamic(this, &APatrollingEnemy::OnDealDamageOverlapBegin);

	DamageSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Damage.Damage"));
	
}

// Called every frame
void APatrollingEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APatrollingEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void APatrollingEnemy::OnAIMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) {
	if (!PlayerDetected)
	{
		AIController->RandomPatrol();
	}
}

void APatrollingEnemy::MoveToPlayer() {
	if (Player)
		AIController->MoveToLocation(Player->GetActorLocation(), StoppingDistance, true);
}

void APatrollingEnemy::SeekPlayer() {
	//move to the player and continue to seek them out while the timer is running
	MoveToPlayer();
	GetWorld()->GetTimerManager().SetTimer(SeekPlayerTimerHandle, this,
		&APatrollingEnemy::SeekPlayer, 0.25f, true);
}

void APatrollingEnemy::StopSeekingPlayer() {
	//when we can't see the player and it's time to stop looking, clear the timer
	GetWorld()->GetTimerManager().ClearTimer(SeekPlayerTimerHandle);
}

void APatrollingEnemy::OnPlayerDetectedOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {

	Player = Cast<AControllableCharacter>(OtherActor);
	if (Player) {
		PlayerDetected = true;
		SeekPlayer();
	}

}

void APatrollingEnemy::OnPlayerDetectedOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex) {

	Player = Cast<AControllableCharacter>(OtherActor);
	if (Player) {
		PlayerDetected = false;
		StopSeekingPlayer();
		AIController->RandomPatrol();
	}
}

void APatrollingEnemy::OnPlayerAttackOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {

	Player = Cast<AControllableCharacter>(OtherActor);
	if (Player) {
		PlayerDetected = true;
		CanAttackPlayer = true;
		//deal damage to the player
		//UE_LOG(LogTemp, Warning, TEXT("Player Damaged"));

		// Launch the player backward
		if (Player->IsValidLowLevel())
		{
			// Define a fixed launch speed
			float LaunchSpeed = 4000.0f; // Adjust as needed

			// Get the backward direction relative to the enemy's forward vector
			FVector BackwardDirection = -Player->GetActorForwardVector().GetSafeNormal();

			// Scale the backward direction by the fixed launch speed
			FVector LaunchVelocity = BackwardDirection * LaunchSpeed;

			// Launch the player character
			Player->LaunchCharacter(LaunchVelocity, false, false);

			Player->DealDamage(10);

			// Set the sound to play and start it
			AudioComponent->SetSound(DamageSound);
			AudioComponent->Play();
		}
	}

}

void APatrollingEnemy::OnPlayerAttackOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex) {

	Player = Cast<AControllableCharacter>(OtherActor);
	if (Player) {
		CanAttackPlayer = false;
		SeekPlayer();
	}
}

bool APatrollingEnemy::GetCanAttackPlayer()
{
	return CanAttackPlayer;
}

