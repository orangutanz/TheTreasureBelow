// Copyright Yuhan Ma. All Rights Reserved.


#include "BaseGameInstance.h"
#include "Framework/Application/NavigationConfig.h"

void UTTBConfig::DisableTabNaviation()
{
	TSharedRef<FNavigationConfig> Navigation = MakeShared<FNavigationConfig>();
	//Navigation->bKeyNavigation = false;
	//Navigation->bAnalogNavigation = false;
	Navigation->bTabNavigation = false;
	FSlateApplication::Get().SetNavigationConfig(Navigation);
}
