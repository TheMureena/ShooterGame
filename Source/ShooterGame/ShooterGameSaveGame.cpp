
#include "ShooterGameSaveGame.h"
#include "ShooterGame.h"

UShooterGameSaveGame::UShooterGameSaveGame()
{
	SaveSlotName = TEXT("TestSaveSlot");
	UserIndex = 0;
}

void UShooterGameSaveGame::SaveGame()
{
	if (UShooterGameSaveGame* SaveGameInstance = Cast<UShooterGameSaveGame>(UGameplayStatics::CreateSaveGameObject(UShooterGameSaveGame::StaticClass())))
	{
		// set the data on the savegame object
		SaveGameInstance->PlayerName = TEXT("DefaultPlayer");

		// save the data immediately
		if (UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveSlotName, 0))
		{
			// save succeeded
			UE_LOG(LogTemp, Warning, TEXT("Saved Game: %s"), *SaveGameInstance->PlayerName);
		}

	}
}

void UShooterGameSaveGame::LoadGame()
{
	// Retrieve and cast the USaveGame object to UShooterGameSaveGame.
	if (UShooterGameSaveGame* LoadedGame = Cast<UShooterGameSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0)))
	{
		// The operation was successful, so LoadedGame now contains the data we saved earlier.
		UE_LOG(LogTemp, Warning, TEXT("LOADED: %s"), *LoadedGame->PlayerName);
	}

}
