// Copyright Epic Games, Inc. All Rights Reserved.

#include "VoxelWorldGameMode.h"
#include "VoxelWorldCharacter.h"
#include "UObject/ConstructorHelpers.h"

AVoxelWorldGameMode::AVoxelWorldGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
