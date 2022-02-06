// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

MODULARGASCOMPANION_API DECLARE_LOG_CATEGORY_EXTERN(LogModularGASCompanion, Display, All);

class FMGCScreenLogger
{
public:

	static FColor GetOnScreenVerbosityColor(const ELogVerbosity::Type Verbosity)
	{
		return
			Verbosity == ELogVerbosity::Fatal || Verbosity == ELogVerbosity::Error ? FColor::Red :
			Verbosity == ELogVerbosity::Warning ? FColor::Yellow :
			Verbosity == ELogVerbosity::Display || Verbosity == ELogVerbosity::Log ? FColor::Cyan :
			Verbosity == ELogVerbosity::Verbose || Verbosity == ELogVerbosity::VeryVerbose ? FColor::Orange :
			FColor::Cyan;
	}

	static void AddOnScreenDebugMessage(const ELogVerbosity::Type Verbosity, const FString Message)
	{
		if (GEngine)
		{
			const FColor Color = GetOnScreenVerbosityColor(Verbosity);
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.f, Color, Message);
		}
	}
};

#define MGC_LOG(Verbosity, Format, ...) \
{ \
    UE_LOG(LogModularGASCompanion, Verbosity, Format, ##__VA_ARGS__); \
}

#define MGC_SLOG(Verbosity, Format, ...) \
{ \
    FMGCScreenLogger::AddOnScreenDebugMessage(ELogVerbosity::Verbosity, FString::Printf(Format, ##__VA_ARGS__)); \
    UE_LOG(LogModularGASCompanion, Verbosity, Format, ##__VA_ARGS__); \
}
