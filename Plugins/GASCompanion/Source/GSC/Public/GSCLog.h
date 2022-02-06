// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Intended categories:
//	Log - This happened. What gameplay programmers may care about to debug
//	Verbose - This is why this happened. What you may turn on to debug the ability system code.
//

GASCOMPANION_API DECLARE_LOG_CATEGORY_EXTERN(LogAbilitySystemCompanion, Display, All);
GASCOMPANION_API DECLARE_LOG_CATEGORY_EXTERN(VLogAbilitySystemCompanion, Display, All);
GASCOMPANION_API DECLARE_LOG_CATEGORY_EXTERN(LogAbilitySystemCompanionUI, Display, All);
GASCOMPANION_API DECLARE_LOG_CATEGORY_EXTERN(VLogAbilitySystemCompanionUI, Display, All);
GASCOMPANION_API DECLARE_LOG_CATEGORY_EXTERN(LogAbilitySystemCompanionAI, Display, All);
GASCOMPANION_API DECLARE_LOG_CATEGORY_EXTERN(VLogAbilitySystemCompanionAI, Display, All);

#if NO_LOGGING || !PLATFORM_DESKTOP

// Without logging enabled we pass ability system through to UE_LOG which only handles Fatal verbosity in NO_LOGGING
#define GSC_LOG(Verbosity, Format, ...) \
{ \
    UE_LOG(LogAbilitySystemCompanion, Verbosity, Format, ##__VA_ARGS__); \
}

#define GSC_VLOG(Actor, Verbosity, Format, ...) \
{ \
    UE_LOG(VLogAbilitySystemCompanion, Verbosity, Format, ##__VA_ARGS__); \
    UE_VLOG(Actor, VLogAbilitySystemCompanion, Verbosity, Format, ##__VA_ARGS__); \
}

#define GSC_UI_LOG(Verbosity, Format, ...) \
{ \
    UE_LOG(LogAbilitySystemCompanionUI, Verbosity, Format, ##__VA_ARGS__); \
}

#define GSC_UI_VLOG(Actor, Verbosity, Format, ...) \
{ \
    UE_LOG(VLogAbilitySystemCompanionUI, Verbosity, Format, ##__VA_ARGS__); \
    UE_VLOG(Actor, VLogAbilitySystemCompanionUI, Verbosity, Format, ##__VA_ARGS__); \
}

#define GSC_AI_LOG(Verbosity, Format, ...) \
{ \
    UE_LOG(LogAbilitySystemCompanionAI, Verbosity, Format, ##__VA_ARGS__); \
}

#define GSC_AI_VLOG(Actor, Verbosity, Format, ...) \
{ \
    UE_LOG(VLogAbilitySystemCompanionAI, Verbosity, Format, ##__VA_ARGS__); \
    UE_VLOG(Actor, VLogAbilitySystemCompanionAI, Verbosity, Format, ##__VA_ARGS__); \
}

#else

#define GSC_LOG(Verbosity, Format, ...) \
{ \
    UE_LOG(LogAbilitySystemCompanion, Verbosity, Format, ##__VA_ARGS__); \
}

#define GSC_VLOG(Actor, Verbosity, Format, ...) \
{ \
    UE_LOG(VLogAbilitySystemCompanion, Verbosity, Format, ##__VA_ARGS__); \
    UE_VLOG(Actor, VLogAbilitySystemCompanion, Verbosity, Format, ##__VA_ARGS__); \
}

#define GSC_UI_LOG(Verbosity, Format, ...) \
{ \
    UE_LOG(LogAbilitySystemCompanionUI, Verbosity, Format, ##__VA_ARGS__); \
}

#define GSC_UI_VLOG(Actor, Verbosity, Format, ...) \
{ \
    UE_LOG(VLogAbilitySystemCompanionUI, Verbosity, Format, ##__VA_ARGS__); \
    UE_VLOG(Actor, VLogAbilitySystemCompanionUI, Verbosity, Format, ##__VA_ARGS__); \
}

#define GSC_AI_LOG(Verbosity, Format, ...) \
{ \
    UE_LOG(LogAbilitySystemCompanionAI, Verbosity, Format, ##__VA_ARGS__); \
}

#define GSC_AI_VLOG(Actor, Verbosity, Format, ...) \
{ \
    UE_LOG(VLogAbilitySystemCompanionAI, Verbosity, Format, ##__VA_ARGS__); \
    UE_VLOG(Actor, VLogAbilitySystemCompanionAI, Verbosity, Format, ##__VA_ARGS__); \
}

#endif //NO_LOGGING
