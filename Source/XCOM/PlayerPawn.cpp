// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerPawn.h"
#include "Engine.h"
#include "Classes/Camera/CameraComponent.h"
#include "Classes/GameFramework/SpringArmComponent.h"
#include "Classes/Components/SceneComponent.h"
#include "Classes/Components/InputComponent.h"
#include "Classes/Engine/GameViewportClient.h"
#include "Classes/Kismet/KismetMathLibrary.h"
#include "PlayerPawn.h"
#include "CustomThirdPerson.h"


// Sets default values
APlayerPawn::APlayerPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(RootSceneComponent);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(RootSceneComponent);
	SpringArm->TargetArmLength = 700.0f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Follow Camera"));
	FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
}

// Called when the game starts or when spawned
void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector2D mousePosition;
	FVector2D viewportSize;

	//Get mouse position and screen size
	UGameViewportClient* gameViewport = GEngine->GameViewport;
	
	//Make sure viewport exists
	check(gameViewport);
	gameViewport->GetViewportSize(viewportSize);

	if (gameViewport->IsFocused(gameViewport->Viewport) && gameViewport->GetMousePosition(mousePosition))
	{
		//Check if the mouse is at the left or right edge of the screen and move accordingly
		if (mousePosition.X < CameraScrollBoundary)
		{
			MoveCameraRight(-1.0f );
		}
		else if (viewportSize.X - mousePosition.X < CameraScrollBoundary)
		{
			MoveCameraRight(1.0f );
		}

		//Check if the mouse is at the top or bottom edge of the screen and move accordingly
		if (mousePosition.Y < CameraScrollBoundary)
		{
			MoveCameraForward(1.0f );
		}
		else if (viewportSize.Y - mousePosition.Y < CameraScrollBoundary)
		{
			MoveCameraForward(-1.0f);
		}
	}

}

// Called to bind functionality to input
void APlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	PlayerInputComponent->BindAction("ZoomIn", IE_Pressed, this, &APlayerPawn::CameraZoomIn);
	PlayerInputComponent->BindAction("ZoomOut", IE_Pressed, this, &APlayerPawn::CameraZoomOut);

	PlayerInputComponent->BindAction("HoverCharacter", IE_Pressed, this, &APlayerPawn::EnableHover);
	PlayerInputComponent->BindAction("HoverCharacter", IE_Released, this, &APlayerPawn::DisableHover);

	PlayerInputComponent->BindAxis("Hover", this, &APlayerPawn::HoverCamera);
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerPawn::MoveCameraForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerPawn::MoveCameraRight);
}



void APlayerPawn::CameraZoomIn() 
{
	float PlayerPawnSpringArmLength= FMath::Clamp(SpringArm->TargetArmLength - CameraZoomSpeed, 100.f, 700.f);
	//100 ~ 700
	SpringArm->TargetArmLength = PlayerPawnSpringArmLength;
	//150 ~ 200
	ControlDistanceToUIDelegate.Broadcast((PlayerPawnSpringArmLength - 100) / 4 + 150);
}

void APlayerPawn::CameraZoomOut() 
{
	float PlayerPawnSpringArmLength = FMath::Clamp(SpringArm->TargetArmLength + CameraZoomSpeed, 100.f, 700.f);
	//100 ~ 700
	SpringArm->TargetArmLength = PlayerPawnSpringArmLength;
	//150 ~ 200
	ControlDistanceToUIDelegate.Broadcast((PlayerPawnSpringArmLength - 100) / 4 + 150);
}

void APlayerPawn::MoveCameraForward(float Direction) 
{
	float movementValue= Direction* FApp::GetDeltaTime() * CameraMovementSpeed;;
	FVector DeltaMovement = movementValue * FRotator(0.0f, FollowCamera->ComponentToWorld.Rotator().Yaw, 0.0f).Vector();
	FVector newLocation = this->GetActorLocation() + DeltaMovement;

	//Set the new location of the pawn
	SetActorLocation(newLocation);
}

void APlayerPawn::MoveCameraRight(float Direction) 
{
	float movementValue = Direction* FApp::GetDeltaTime() * CameraMovementSpeed;;
	FVector DeltaMovement = movementValue * (FRotator(0.0f, 90.0f, 0.0f) + FRotator(0.0f, FollowCamera->ComponentToWorld.Rotator().Yaw, 0.0f) ).Vector();
	FVector newLocation = this->GetActorLocation() + DeltaMovement;

	//Set the new location of the pawn
	SetActorLocation(newLocation);
}


void APlayerPawn::EnableHover()
{
	bCanHover = true;
	bCanScroll = false;	
}

void APlayerPawn::DisableHover()
{
	bCanHover = false;
	bCanScroll = true;
}


void APlayerPawn::HoverCamera(float AxisValue) 
{
	if (bCanHover) 
	{
		FRotator newRotator = SpringArm->GetComponentRotation() + FRotator(0, AxisValue * CameraHoverSpeed, 0);
		SpringArm->SetWorldRotation(newRotator);
		RotateUIDelegate.Broadcast(AxisValue);
	}
}

void APlayerPawn::OnClick()
{
	FHitResult TraceResult;
	GetHitResultUnderCursor(ECollisionChannel::ECC_MAX, true, TraceResult);
	AActor* actor = TraceResult.GetActor();

	if (!ensure(TraceResult.GetActor())) { return; }
	else
	{
		ACustomThirdPerson* TargetCharacter = Cast<ACustomThirdPerson>(actor);
		ATile* TargetTile = Cast<ATile>(actor);
		if (TargetCharacter)
		{
			if (CheckClickedCharacterTeam(TargetCharacter))
			{
				SwitchCharacter(TargetCharacter);
			}
			else
			{	//�� Ŭ����			(�׽�Ʈ�� �ڵ�- ���Ŀ� �ű���̶� �Լ�ȭ ���� �ʴ´�.)
				PawnInAimingSituation->SetCameraPositionInAimingSituation(SelectedCharacter->GetActorLocation(), TargetCharacter->GetActorLocation());
				//Possess(PawnInAimingSituation);
				SetViewTargetWithBlend(PawnInAimingSituation, 0.5);
				SelectedCharacter->CheckAttackPotential(TargetCharacter);
			}
		}
		else if (TargetTile)
		{
			if (!IsValid(SelectedCharacter))
			{
				UE_LOG(LogTemp, Warning, L"SelectedCharacter invalid!");
				return;
			}

			if (TargetTile->GetTileVisibility() == false)
			{
				UE_LOG(LogTemp, Warning, L"Can not be moved there");
				return;
			}

			int32 TargetTileIndex = TileManager->ConvertVectorToIndex(TargetTile->GetActorLocation());
			MoveCharacterBasedOnState(TargetTileIndex);
		}
		else
		{
			//TODO Ÿ�ϰ� ĳ���Ͱ� �ƴ� obj Ŭ���� ó��
		}
	}
}


// �׽�Ʈ�� ���� �޼ҵ�
// ���Ŀ� ���� �Ǵ� ���� �ʿ�
bool APlayerPawn::CheckClickedCharacterTeam(ACustomThirdPerson* ClickedCharacter)
{
	if (!SelectedCharacter)
	{
		SelectedCharacter = ClickedCharacter;
		return true;
	}

	if (SelectedCharacter->GetTeamFlag() == ClickedCharacter->GetTeamFlag())
	{
		return true;
	}
	else if (!(SelectedCharacter->bCanAction))
	{
		return true;
	}

	return false;
}

/**
* ���� �� �ȿ��� ���� ���õǾ� �ִ� ĳ���Ͱ� �ƴ� �ٸ� ĳ���ͷ� ��ȯ�ɶ� ȣ��˴ϴ�. 
* @param TargetCharacter - ��ȯ�� ĳ����
*/
void APlayerPawn::SwitchCharacter(ACustomThirdPerson* TargetCharacter)
{
	if (TargetCharacter->NumberOfRemainingActivities > 0) {
		DisableInput(this);
	
		TArray<AActor*> OverlappedTile;
		TargetCharacter->GetOverlappingActors(OverlappedTile);
		if (OverlappedTile.Num() == 0)		//����ó�� 
		{ 
			EnableInput(this);
			return; 
		}
		FTimerHandle UnUsedHandle;
		FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &AXCOMPlayerController::SetTilesToUseSelectedChararacter, Cast<ATile>(OverlappedTile[0]), 4);
		GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 0.5f, false);

		//Ŭ���� Actor �̵� �ʿ�   ( ī�޶� �̵��� �ƴ� )
		SelectedCharacter = TargetCharacter;
	}
	else
	{
		//�ൿ Ƚ�� 0 �϶�
	}
}

/**
* �ٸ� ĳ���͸� Ŭ���� ��ȯ������ ȣ��˴ϴ�.
* @param OverlappedTile - ���õ� ĳ���Ͱ� �ö��ִ� Ÿ��
* @param MovingAbility - �̵� ������ ĭ ��
*/
void APlayerPawn::SetTilesToUseSelectedChararacter(ATile* OverlappedTile, const int32 MovingAbility) 
{
	EnableInput(this);
	TileManager->ClearAllTiles(true);

	TArray<ATile*> TilesInRange;
	TileManager->GetAvailableTiles(OverlappedTile, MovingAbility, TilesInRange);

	for (ATile* Tile : TilesInRange)
	{
		UStaticMeshComponent* TileMesh = Cast<UStaticMeshComponent>(Tile->GetRootComponent());
		TileMesh->SetVisibility(true);
	}
}

/**
* ��ǥ�� �ϴ� Ÿ�Ϸ� ĳ������ ���� ���¿� ���� ĳ���Ͱ� �ٸ��� �����Դϴ�.
* @param TargetTileIndex - �̵��� Ÿ���� �ε���
*/
void APlayerPawn::MoveCharacterBasedOnState(int32 TargetTileIndex) 
{
	DisableInput(this);

	SelectedCharacter->NumberOfRemainingActivities = NumberOfRemainingActivities->NumberOfRemainingActivities - 1;
	if (SelectedCharacter->bIsCovering)
	{
		SelectedCharacter->ClearCoverDirectionInfo();
		FTimerHandle UnUsedHandle;
		FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &AXCOMPlayerController::MovingStepByStep, TileManager->PathArr[TileIndex], TileManager->PathArr[TileIndex].OnTheWay.Num() - 1);
		GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 1.2, false);	// 0.5 Delay ����
	}
	else
	{
		MovingStepByStep(TileManager->PathArr[TileIndex], TileManager->PathArr[TileIndex].OnTheWay.Num() - 1);
	}
}