#include "FogOfWarManager.h"
#include "Public/RHI.h"
#include "AFogOfWarWorker.h"
#include "CustomThirdPerson.h"

AFogOfWarManager::AFogOfWarManager(const FObjectInitializer &FOI) : Super(FOI)
{
	PrimaryActorTick.bCanEverTick = true;
	textureRegions = new FUpdateTextureRegion2D(0, 0, 0, 0, TextureSize, TextureSize);

	//15 Gaussian samples. Sigma is 2.0.
	//CONSIDER: Calculate the kernel instead, more flexibility...
	blurKernel.Init(0.0f, blurKernelSize);
	blurKernel[0] = 0.000489f;
	blurKernel[1] = 0.002403f;
	blurKernel[2] = 0.009246f;
	blurKernel[3] = 0.02784f;
	blurKernel[4] = 0.065602f;
	blurKernel[5] = 0.120999f;
	blurKernel[6] = 0.174697f;
	blurKernel[7] = 0.197448f;
	blurKernel[8] = 0.174697f;
	blurKernel[9] = 0.120999f;
	blurKernel[10] = 0.065602f;
	blurKernel[11] = 0.02784f;
	blurKernel[12] = 0.009246f;
	blurKernel[13] = 0.002403f;
	blurKernel[14] = 0.000489f;
}

AFogOfWarManager::~AFogOfWarManager()
{
	if (FowThread) {
		FowThread->ShutDown();
	}
}

void AFogOfWarManager::BeginPlay()
{
	Super::BeginPlay();
	bIsDoneBlending = true;
	AFogOfWarManager::StartFOWTextureUpdate();
}

void AFogOfWarManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (FOWTexture && LastFOWTexture && bHasFOWTextureUpdate && bIsDoneBlending) {
		LastFOWTexture->UpdateResource();
		UpdateTextureRegions(LastFOWTexture, (int32)0, (uint32)1, textureRegions, (uint32)(4 * TextureSize), (uint32)4, (uint8*)LastFrameTextureData.GetData(), false);
		FOWTexture->UpdateResource();
		UpdateTextureRegions(FOWTexture, (int32)0, (uint32)1, textureRegions, (uint32)(4 * TextureSize), (uint32)4, (uint8*)TextureData.GetData(), false);
		bHasFOWTextureUpdate = false;
		bIsDoneBlending = false;
		//Trigger the blueprint update
		OnFowTextureUpdated(FOWTexture, LastFOWTexture);
	}
}

void AFogOfWarManager::StartFOWTextureUpdate()
{
	if (!FOWTexture) {
		FOWTexture = UTexture2D::CreateTransient(TextureSize, TextureSize);
		LastFOWTexture = UTexture2D::CreateTransient(TextureSize, TextureSize);
		int arraySize = TextureSize * TextureSize;
		TextureData.Init(FColor(FowMaskColor, FowMaskColor, FowMaskColor, 255), arraySize);
		LastFrameTextureData.Init(FColor(FowMaskColor, FowMaskColor, FowMaskColor, 255), arraySize);
		HorizontalBlurData.Init(0, arraySize);
		UnfoggedData.Init(true , arraySize);
		CurrentlyInSight.Init(false, arraySize);
		FowThread = new AFogOfWarWorker(this);
	}


	//
	if (GetIsTextureFileEnabled()) {

		if (!TextureInFile) {

			UE_LOG(LogTemp, Error, TEXT("Missing texture in LevelInfo, please load the mask!"));
			return;
		}
		if (TextureInFile != nullptr) {

			int TextureInFileSize = TextureInFile->GetSizeX();
			TextureInFileData.Init(FColor(1, 1, 1, 255), TextureInFileSize * TextureInFileSize);
			//init TArray
			//TextureInFileData.Init(FColor(0, 0, 0, 255), arraySize * arraySize);//init TArray, use this in unified version
			//The operation from Texture File ->to-> TArray of FColors

			UE_LOG(LogTemp, Warning, TEXT("Texture in file is :  %d  pixels wide"), TextureInFileSize);

			//Force texture compression to vectorDispl , https://wiki.unrealengine.com/Procedural_Materials

			//TODO here you need to add a halt or a warning to prevent the loading of textures that don´t meet the criteria
			//no mipmaps, compression to vector Displacement
			//i could check the compression settings, but they are in enum form

			//template<class TEnum>
			//class TEnumAsByte TextureCompressionStatus = TextureInFile->CompressionSettings;



			FTexture2DMipMap& Mip = TextureInFile->PlatformData->Mips[0];
			void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
			uint8* raw = NULL;
			raw = (uint8*)Data;
			FColor pixel = FColor(0, 0, 0, 255);//used for spliting the data stored in raw form

			for (int y = 0; y < TextureInFileSize; y++) {
				//data in the raw var is serialized i think ;)
				//so a pixel is four consecutive numbers e.g 0,0,0,255
				//and the following code split the values in single components and store them in a FColor
				for (int x = 0; x < TextureInFileSize; x++) {
					pixel.B = raw[4 * (TextureInFileSize * y + x) + 0];
					pixel.G = raw[4 * (TextureInFileSize * y + x) + 1];
					pixel.R = raw[4 * (TextureInFileSize * y + x) + 2];
					TextureInFileData[x + y * TextureInFileSize] = FColor((uint8)pixel.R, (uint8)pixel.G, (uint8)pixel.B, 255);

					//ToDo: IMPORTANT, YOU NEED TO THINK WHAT HAPPENS IF THE TEXTURE IN FILE HAS A DIFFERRENT SIZE THAN BOOL UNFOGGEDDATA, THIS IS COULD
					//CAUSE AN OUT OF BOUNDS ARRAY, OR SOMETHING

					//Here we are writing to the UnfoggedData Array the values that are already unveiled, from the texture file
					if (pixel.B >= 100) {
						UnfoggedData[x + y * TextureInFileSize] = true;
					}

				}
				//FColor Colorcito = TextureInFileData[y ];
				//UE_LOG(LogTemp, Warning, TEXT("TArray in y:  %d is :  %s"), y , *Colorcito.ToString());
			}

			//FColor Colorcito = TextureInFileData[524288];
			//UE_LOG(LogTemp, Warning, TEXT("TArray in 524288 is :  %s"), *Colorcito.ToString());

			Mip.BulkData.Unlock();
			TextureInFile->UpdateResource();
		}


		if (FOWTexture) {

			TextureData = TextureInFileData;
			LastFrameTextureData = TextureInFileData;
			//Missing the TArray<bool> for unfogged data, do this
		}

	}
}

void AFogOfWarManager::OnFowTextureUpdated_Implementation(UTexture2D* currentTexture, UTexture2D* lastTexture) {
	//Handle in blueprint
}

void AFogOfWarManager::RegisterFowActor(AActor* Actor) {
	FowActors.Add(Actor);

	ACustomThirdPerson* Unit = Cast<ACustomThirdPerson>(Actor);
	if (Unit) 
	{
		Unit->UnitDeadDelegate.AddUniqueDynamic(this ,&AFogOfWarManager::UnregisterFowActor);
	}
}

void AFogOfWarManager::UnregisterFowActor(ACustomThirdPerson* Actor) 
{
	FowActors.Remove(Actor);
}

bool AFogOfWarManager::GetIsBlurEnabled() {
	return bIsBlurEnabled;
}

bool AFogOfWarManager::GetIsTextureFileEnabled() {
	return bUseTextureFile;
}


void AFogOfWarManager::UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData)
{
	if (Texture && Texture->Resource)
	{
		struct FUpdateTextureRegionsData
		{
			FTexture2DResource* Texture2DResource;
			int32 MipIndex;
			uint32 NumRegions;
			FUpdateTextureRegion2D* Regions;
			uint32 SrcPitch;
			uint32 SrcBpp;
			uint8* SrcData;
		};

		FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

		RegionData->Texture2DResource = (FTexture2DResource*)Texture->Resource;
		RegionData->MipIndex = MipIndex;
		RegionData->NumRegions = NumRegions;
		RegionData->Regions = Regions;
		RegionData->SrcPitch = SrcPitch;
		RegionData->SrcBpp = SrcBpp;
		RegionData->SrcData = SrcData;

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			UpdateTextureRegionsData,
			FUpdateTextureRegionsData*, RegionData, RegionData,
			bool, bFreeData, bFreeData,
			{
				for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex)
				{
					int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
					if (RegionData->MipIndex >= CurrentFirstMip)
					{
						RHIUpdateTexture2D(
							RegionData->Texture2DResource->GetTexture2DRHI(),
							RegionData->MipIndex - CurrentFirstMip,
							RegionData->Regions[RegionIndex],
							RegionData->SrcPitch,
							RegionData->SrcData
							+ RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch
							+ RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp
						);
					}
				}
		if (bFreeData)
		{
			FMemory::Free(RegionData->Regions);
			FMemory::Free(RegionData->SrcData);
		}
		delete RegionData;
			}
		);
	}
}

bool AFogOfWarManager::IsInCognitionArea(FVector Location) 
{
	if (FowThread) 
	{
		return FowThread->IsInIncogArea(Location);
	}
	else 
	{
		UE_LOG(LogTemp,Warning, L"½ÇÆÐ¾Ö")
		return false;
	}

}
