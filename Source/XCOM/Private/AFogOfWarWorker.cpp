
#include "AFogOfWarWorker.h"
#include "HAL/RunnableThread.h"
#include "FogOfWarManager.h"
#include "FogOfWarComponent.h"
#include "Kismet/KismetSystemLibrary.h"

AFogOfWarWorker::AFogOfWarWorker()
{

}

AFogOfWarWorker::AFogOfWarWorker(AFogOfWarManager* manager) {
	Manager = manager;
	Thread = FRunnableThread::Create(this, TEXT("AFogOfWarWorker"), 0U, TPri_BelowNormal);
}

AFogOfWarWorker::~AFogOfWarWorker() {
	delete Thread;
	Thread = NULL;
}

void AFogOfWarWorker::ShutDown() {
	Stop();
	Thread->WaitForCompletion();
}

bool AFogOfWarWorker::Init() {
	if (Manager)
	{
		Manager->GetWorld()->GetFirstPlayerController()->ClientMessage("Fog of War worker thread started");
		return true;
	}
	return false;
}

uint32 AFogOfWarWorker::Run() {
	FPlatformProcess::Sleep(0.03f);
	while (StopTaskCounter.GetValue() == 0) {
		float time;
		if (Manager && Manager->GetWorld()) {
			time = Manager->GetWorld()->TimeSeconds;
		}
		if (!Manager->bHasFOWTextureUpdate) {
			UpdateFowTexture();
			if (Manager && Manager->GetWorld()) {
				Manager->fowUpdateTime = Manager->GetWorld()->TimeSince(time);
			}
		}
		FPlatformProcess::Sleep(0.1f);
	}
	return 0;
}

void AFogOfWarWorker::UpdateFowTexture() {
	Manager->LastFrameTextureData = TArray<FColor>(Manager->TextureData);
	uint32 halfTextureSize = Manager->TextureSize / 2;
	int signedSize = (int)Manager->TextureSize; //For convenience....
	TSet<FVector2D> texelsToBlur;
	int sightTexels = Manager->SightRange * Manager->SamplesPerMeter;
	float dividend = 100.0f / Manager->SamplesPerMeter;

	for (auto Itr(Manager->FowActors.CreateIterator()); Itr; Itr++) {
		//Find actor position
		if (!*Itr) return;
		FVector position = (*Itr)->GetActorLocation();

		//We divide by 100.0 because 1 texel equals 1 meter of visibility-data.
		int posX = (int)(position.X / dividend) + halfTextureSize;
		int posY = (int)(position.Y / dividend) + halfTextureSize;
		double integerX, integerY;

		FVector2D fractions = FVector2D(modf(position.X / 50.0f, &integerX), modf(position.Y / 50.0f, &integerY));
		FVector2D textureSpacePos = FVector2D(posX, posY);
		int size = (int)Manager->TextureSize;

		int halfKernelSize = (Manager->blurKernelSize - 1) / 2;

		//Store the positions we want to blur
		for (int y = posY - sightTexels - halfKernelSize; y <= posY + sightTexels + halfKernelSize; y++) {
			for (int x = posX - sightTexels - halfKernelSize; x <= posX + sightTexels + halfKernelSize; x++) {
				if (x > 0 && x < size && y > 0 && y < size) {
					texelsToBlur.Add(FIntPoint(x, y));
				}
			}
		}

		isWriteUnFog = (*Itr)->FindComponentByClass<UFogOfWarComponent>()->WriteUnFog;
		isWriteFow = (*Itr)->FindComponentByClass<UFogOfWarComponent>()->WriteFow;
		isWriteTerraIncog = (*Itr)->FindComponentByClass<UFogOfWarComponent>()->WriteTerraIncog;

		if (isWriteUnFog)
		{
			//Unveil the positions our actors are currently looking at
			for (int y = posY - sightTexels; y <= posY + sightTexels; y++) {
				for (int x = posX - sightTexels; x <= posX + sightTexels; x++) {
					//Kernel for radial sight
					if (x > 0 && x < size && y > 0 && y < size) {
						FVector2D currentTextureSpacePos = FVector2D(x, y);
						int length = (int)(textureSpacePos - currentTextureSpacePos).Size();
						if (length <= sightTexels) {
							FVector currentWorldSpacePos = FVector(
								((x - (int)halfTextureSize)) * dividend,
								((y - (int)halfTextureSize)) * dividend,
								position.Z);

							//CONSIDER: This is NOT the most efficient way to do conditional unfogging. With long view distances and/or a lot of actors affecting the FOW-data
							//it would be preferrable to not trace against all the boundary points and internal texels/positions of the circle, but create and cache "rasterizations" of
							//viewing circles (using Bresenham's midpoint circle algorithm) for the needed sightranges, shift the circles to the actor's location
							//and just trace against the boundaries. 
							//We would then use Manager->GetWorld()->LineTraceSingle() and find the first collision texel. Having found the nearest collision
							//for every ray we would unveil all the points between the collision and origo using Bresenham's Line-drawing algorithm.
							//However, the tracing doesn't seem like it takes much time at all (~0.02ms with four actors tracing circles of 18 texels each),
							//it's the blurring that chews CPU..
							Manager->UnfoggedData[x + y * Manager->TextureSize] = true;
							//Store the positions we are currently seeing.

							Manager->CurrentlyInSight[x + y * Manager->TextureSize] = true;

						}
					}
					else
					{
						Manager->CurrentlyInSight[x + y * Manager->TextureSize] = false;
					}
				}
			}
		}
		//currentlyInSight.Contains(FVector2D(textureSpacePos.X  , textureSpacePos.Y))
		//Is the current actor marked for checking if is in terra incognita
		bCheckActorInTerraIncog = (*Itr)->FindComponentByClass<UFogOfWarComponent>()->bCheckActorTerraIncog;
		if (bCheckActorInTerraIncog)
		{
			bool CurrentCog = (*Itr)->FindComponentByClass<UFogOfWarComponent>()->isActorInTerraIncog;
			if (Manager->CurrentlyInSight[textureSpacePos.X + textureSpacePos.Y * Manager->TextureSize])
			{
				FVector currentWorldSpacePos = FVector(
					((textureSpacePos.X - (int)halfTextureSize)) * dividend,
					((textureSpacePos.Y - (int)halfTextureSize)) * dividend,
					position.Z);

				(*Itr)->FindComponentByClass<UFogOfWarComponent>()->SetActorInTerraInCog(true);
				/*UE_LOG(LogTemp, Warning, L"È£ÃâµÊ");
				if (!CurrentCog)
				{
					(*Itr)->FindComponentByClass<UFogOfWarComponent>()->SetActorInTerraInCog(true);
					UE_LOG(LogTemp, Warning, L"È£ÃâµÊ");

				}*/

			}
			//If this is a previously discovered position that we're not currently looking at, put it into a "shroud of darkness".
			else
			{
				FVector currentWorldSpacePos = FVector(
					((textureSpacePos.X - (int)halfTextureSize)) * dividend,
					((textureSpacePos.Y - (int)halfTextureSize)) * dividend,
					position.Z);


				(*Itr)->FindComponentByClass<UFogOfWarComponent>()->SetActorInTerraInCog(false);

				/*if (CurrentCog)
				{
					(*Itr)->FindComponentByClass<UFogOfWarComponent>()->SetActorInTerraInCog(false);
					UE_LOG(LogTemp, Warning, L"È£ÃâµÊ2");

				}*/

			}
		}
	}


	//  ÀÌ ¹ØÀ¸·Î´Â Blur
	if (Manager->GetIsBlurEnabled()) {
		//Horizontal blur pass
		int offset = floorf(Manager->blurKernelSize / 2.0f);
		for (auto Itr(texelsToBlur.CreateIterator()); Itr; ++Itr) {
			int x = (Itr)->IntPoint().X;
			int y = (Itr)->IntPoint().Y;
			float sum = 0;
			for (int i = 0; i < Manager->blurKernelSize; i++) {
				int shiftedIndex = i - offset;
				if (x + shiftedIndex >= 0 && x + shiftedIndex <= signedSize - 1) {
					if (Manager->UnfoggedData[x + shiftedIndex + (y * signedSize)]) {
						//If we are currently looking at a position, unveil it completely
						if (Manager->CurrentlyInSight[x + shiftedIndex + y* Manager->TextureSize]) {
							sum += (Manager->blurKernel[i] * 255);
						}
						//If this is a previously discovered position that we're not currently looking at, put it into a "shroud of darkness".							
						else {
							sum += (Manager->blurKernel[i] * 100);
						}
					}
				}
			}
			Manager->HorizontalBlurData[x + y * signedSize] = (uint8)sum;
		}


		//Vertical blur pass
		for (auto Itr(texelsToBlur.CreateIterator()); Itr; ++Itr) {
			int x = (Itr)->IntPoint().X;
			int y = (Itr)->IntPoint().Y;
			float sum = 0;
			for (int i = 0; i < Manager->blurKernelSize; i++) {
				int shiftedIndex = i - offset;
				if (y + shiftedIndex >= 0 && y + shiftedIndex <= signedSize - 1) {
					sum += (Manager->blurKernel[i] * Manager->HorizontalBlurData[x + (y + shiftedIndex) * signedSize]);
				}
			}
			Manager->TextureData[x + y * signedSize] = FColor((uint8)sum, (uint8)sum, (uint8)sum, 255);
		}
	}
	else {
		for (int y = 0; y < signedSize; y++) {
			for (int x = 0; x < signedSize; x++) {

				if (Manager->UnfoggedData[x + (y * signedSize)]) {
					//If we are currently looking at a position, unveil it completely
					//if the vectors are inside de Tset
					if (Manager->CurrentlyInSight[x + (y * signedSize)]) {
						Manager->TextureData[x + y * signedSize] = FColor(Manager->UnfogColor, Manager->UnfogColor, Manager->UnfogColor, 255);
					}
					//If this is a previously discovered position that we're not currently looking at, put it into a "shroud of darkness".
					else {
						Manager->TextureData[x + y * signedSize] = FColor(Manager->FowMaskColor, Manager->FowMaskColor, Manager->FowMaskColor, 255);

						//RED mask?, no , this needs to work in tandem with the material
						//Manager->TextureData[x + y * signedSize] = FColor(Manager->FowMaskColor, (uint8)0, (uint8)0, 255);
					}
				}
			}
		}
	}
	Manager->bHasFOWTextureUpdate = true;
}


void AFogOfWarWorker::Stop() {
	StopTaskCounter.Increment();
}

bool AFogOfWarWorker::IsInIncogArea(FVector Location) 
{
	uint32 halfTextureSize = Manager->TextureSize / 2;
	float dividend = 100.0f / Manager->SamplesPerMeter;

	FVector position = Location;

	int posX = (int)(position.X / dividend) + halfTextureSize;
	int posY = (int)(position.Y / dividend) + halfTextureSize;

	FVector2D textureSpacePos = FVector2D(posX, posY);

	if (Manager->CurrentlyInSight[textureSpacePos.X + textureSpacePos.Y * Manager->TextureSize])
	{
		return true;
	}
	else
	{
		return false;
	}
}