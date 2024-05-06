// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#define TEST_BIT(Bitmask, BitFlag) (Bitmask & static_cast<uint64>(BitFlag))
#define SET_BIT(Bitmask, BitFlag) (Bitmask |= static_cast<uint64>(BitFlag))
#define CLEAR_BIT(Bitmask, BitFlag) (Bitmask ^= static_cast<uint64>(BitFlag))

UENUM(BlueprintType)
enum class EDebugLevels : uint8
{
	NODEBUG,
	DEBUGTEXT,
	DEBUGTEXTANDSHAPES,
};