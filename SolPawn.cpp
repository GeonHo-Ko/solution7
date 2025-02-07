// Fill out your copyright notice in the Description page of Project Settings.


#include "SolPawn.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Controller.h"
#include "InputAction.h"
#include "InputMappingContext.h"

// Sets default values
ASolPawn::ASolPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	// 캡슐 컴포넌트 (충돌)
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	RootComponent = CapsuleComponent;
	CapsuleComponent->InitCapsuleSize(42.0f, 96.0f);

	// 캐릭터 메시
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CapsuleComponent);

	// 메시 및 머티리얼 적용
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Game/Resources/Characters/Meshes/SKM_Manny.SKM_Manny"));
	if (MeshAsset.Succeeded())
	{
		MeshComponent->SetSkeletalMesh(MeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInstance> Material1(TEXT("/Game/Resources/Characters/Materials/Instances/Manny/MI_Manny_01.MI_Manny_01"));
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> Material2(TEXT("/Game/Resources/Characters/Materials/Instances/Manny/MI_Manny_02.MI_Manny_02"));

	if (Material1.Succeeded() && Material2.Succeeded())
	{
		MeshComponent->SetMaterial(0, Material1.Object);
		MeshComponent->SetMaterial(1, Material2.Object);
	}

	// 물리 시뮬레이션 비활성화 (코드로 제어)
	MeshComponent->SetSimulatePhysics(false);

	// 스프링암 설정 (3인칭 카메라)
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(CapsuleComponent);
	SpringArm->TargetArmLength = 300.0f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bInheritPitch = true;
	SpringArm->bInheritYaw = true;
	SpringArm->bInheritRoll = false;

	// 카메라 설정
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> MappingContextObj(TEXT("/Game/Inputs/IMC_Move.IMC_Move"));
	static ConstructorHelpers::FObjectFinder<UInputAction> MoveForwardObj(TEXT("/Game/Inputs/MoveForward.MoveForward"));
	static ConstructorHelpers::FObjectFinder<UInputAction> MoveRightObj(TEXT("/Game/Inputs/MoveRight.MoveRight"));
	static ConstructorHelpers::FObjectFinder<UInputAction> LookObj(TEXT("/Game/Inputs/Look.Look"));

	if (MappingContextObj.Succeeded()) PawnMappingContext = MappingContextObj.Object;
	if (MoveForwardObj.Succeeded()) MoveForwardAction = MoveForwardObj.Object;
	if (MoveRightObj.Succeeded()) MoveRightAction = MoveRightObj.Object;
	if (LookObj.Succeeded()) LookAction = LookObj.Object;

}

void ASolPawn::BeginPlay()
{
	Super::BeginPlay();
	if (!PawnMappingContext) {
		UE_LOG(LogTemp, Warning, TEXT("PawnMappingContext is not loaded!"));
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("PawnMappingContext loaded successfully"));
	}

	// 플레이어 컨트롤러에 InputMappingContext 추가
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (InputSubsystem)
		{
			InputSubsystem->AddMappingContext(PawnMappingContext, 0);
		}
	}
}

void ASolPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASolPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInput)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnhancedInput is set"));
		EnhancedInput->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &ASolPawn::MoveForward);
		EnhancedInput->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &ASolPawn::MoveRight);
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASolPawn::Look);
	}
}

void ASolPawn::MoveForward(const FInputActionValue& Value)
{
	float InputValue = Value.Get<float>();
	UE_LOG(LogTemp, Warning, TEXT("MoveForward Input: %f"), InputValue);
	if (InputValue != 0.0f)
	{
		AddActorLocalOffset(FVector(InputValue * MoveSpeed * GetWorld()->GetDeltaSeconds(), 0.0f, 0.0f), true);
	}
}

void ASolPawn::MoveRight(const FInputActionValue& Value)
{
	float InputValue = Value.Get<float>();
	UE_LOG(LogTemp, Warning, TEXT("MoveRight Input: %f"), InputValue);
	if (InputValue != 0.0f)
	{
		AddActorLocalOffset(FVector(0.0f, InputValue * MoveSpeed * GetWorld()->GetDeltaSeconds(), 0.0f), true);
	}
}

void ASolPawn::Look(const FInputActionValue& Value)
{
	FVector2D InputValue = Value.Get<FVector2D>();
	if (!InputValue.IsNearlyZero())
	{
		APlayerController* PlayerController = Cast<APlayerController>(GetController());
		if (PlayerController)
		{
			PlayerController->AddYawInput(InputValue.X * TurnSpeed * GetWorld()->GetDeltaSeconds());
			PlayerController->AddPitchInput(-InputValue.Y * TurnSpeed * GetWorld()->GetDeltaSeconds());
		}
	}
}

FRotator ASolPawn::GetViewRotation() const
{
	if (Controller)
	{
		return Controller->GetControlRotation();
	}
	return Super::GetViewRotation();
}
