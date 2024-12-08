// Fill out your copyright notice in the Description page of Project Settings.


#include "ControllableCharacter.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Components/CapsuleComponent.h"

// Sets default values
AControllableCharacter::AControllableCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize the audio component and attach it to the root component
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(RootComponent);
	AudioComponent->bAutoActivate = false; // Do not play sound automatically on start
}

// Called when the game starts or when spawned
void AControllableCharacter::BeginPlay()
{
	Super::BeginPlay();

	JumpSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Jump.Jump"));

	secondsLeft = 30;

	numEggs = 0;

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AControllableCharacter::countDown, 2.f, true);

	CollisionComp = FindComponentByClass<UCapsuleComponent>();

	if (CollisionComp) {
		CollisionComp->OnComponentHit.AddDynamic(this, &AControllableCharacter::OnHit);
	}
	
}

// Called every frame
void AControllableCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AControllableCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	APlayerController* PC = Cast<APlayerController>(GetController());

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());

	Subsystem->ClearAllMappings();
	Subsystem->AddMappingContext(InputMapping, 0);

	UEnhancedInputComponent* PEI = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	PEI->BindAction(InputMove, ETriggerEvent::Triggered, this, &AControllableCharacter::Move);
	PEI->BindAction(InputLook, ETriggerEvent::Triggered, this, &AControllableCharacter::Look);
	PEI->BindAction(InputJump, ETriggerEvent::Triggered, this, &AControllableCharacter::MyJump);

}

void AControllableCharacter::Move(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		const FVector2D MoveValue = Value.Get<FVector2D>();
		const FRotator MovementRotation(0, Controller->GetControlRotation().Yaw, 0);

		if (MoveValue.Y != 0.f)
		{
			const FVector Direction = MovementRotation.RotateVector(FVector::ForwardVector);
			AddMovementInput(Direction, MoveValue.Y);
		}

		if (MoveValue.X != 0.f)
		{
			const FVector Direction = MovementRotation.RotateVector(FVector::RightVector);
			AddMovementInput(Direction, MoveValue.X);
		}
	}
}

void AControllableCharacter::Look(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		const FVector2D LookValue = Value.Get<FVector2D>();

		if (LookValue.X != 0.f)
		{
			AddControllerYawInput(LookValue.X);
		}

		if (LookValue.Y != 0.f)
		{
			AddControllerPitchInput(-LookValue.Y);
		}
	}
}

void AControllableCharacter::MyJump() {

	if (JumpSound && !AudioComponent->IsPlaying()) // make sure the sound and component are valid, and sound is not already playing
	{
		// Set the sound to play and start it
		AudioComponent->SetSound(JumpSound);
		AudioComponent->Play();
	}
	ACharacter::Jump();
}

void AControllableCharacter::countDown()
{
	secondsLeft--;
}

int AControllableCharacter::GetSecondsLeft()
{
	return secondsLeft;
}

int AControllableCharacter::GetNumEggs()
{
	return numEggs;
}

void AControllableCharacter::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor)
	{
		// Get the class name of the other actor
		FString ClassName = OtherActor->GetClass()->GetName();

		if (ClassName.Contains("BP_Egg"))
		{
			//count the egg
			numEggs++;

			//get rid of the egg
			OtherActor->Destroy();
		}

		if (ClassName.Contains("BP_Exit"))
		{
			UE_LOG(LogTemp, Warning, TEXT("Found Exit"));
			if (numEggs == 3)
			{
				UE_LOG(LogTemp, Warning, TEXT("Can Exit"));
			}
		}
	}
}

