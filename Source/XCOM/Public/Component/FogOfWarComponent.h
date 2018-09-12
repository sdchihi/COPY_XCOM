// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FogOfWarComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XCOM_API UFogOfWarComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFogOfWarComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	

	/*Is the actor able to influence unfogged texels*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FogOfWar)
	bool WriteUnFog = true;

	/*Is the actor able to influence fog of war texels*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FogOfWar)
	bool WriteFow = true;

	/*Is the actor able to influence terra incognita texels*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FogOfWar)
	bool WriteTerraIncog = true;

	/*Check if the actor is in terra incognita*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FogOfWar)
	bool bCheckActorTerraIncog = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = FogOfWar)
	bool isActorInTerraIncog = false;
		
	/**
	* 안개안에 위치하고있으면 Unit을 게임에서 감춥니다.
	* @param bCanCognize 안개안에 위치하고있는지 여부
	*/
	void SetActorInTerraInCog(bool bCanCognize);
};
