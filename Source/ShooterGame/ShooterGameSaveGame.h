
#pragma once 

#include "GameFramework/SaveGame.h"
#include "ShooterGameSaveGame.generated.h"

class AGun;

USTRUCT(BlueprintType)
struct FPlayerSave
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
    ULevel* CurrentLevel = nullptr;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<AGunBase*> CurrentGuns;
};

UCLASS()
class SHOOTERGAME_API UShooterGameSaveGame : public USaveGame
{

	GENERATED_BODY()

public:

    UPROPERTY(VisibleAnywhere, Category = Basic)
    FString PlayerName;

    UPROPERTY(VisibleAnywhere, Category = Basic)
    FString SaveSlotName;

    UPROPERTY(VisibleAnywhere, Category = Basic)
    uint32 UserIndex;

    //UPROPERTY(VisibleAnywhere, Category = Variables)
    //ULevel* CurrentLevel;

    //UPROPERTY(VisibleAnywhere, Category = Variables)
    //TArray<AGun*> CurrentGuns;

    UShooterGameSaveGame();

    void SaveGame();

    void LoadGame();

};

