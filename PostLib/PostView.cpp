#include "stdafx.h"
#include "PostView.h"
#include <PostLib/FEElement.h>
#include <PostLib/ColorMap.h>

void Post::Initialize()
{
	FEElementLibrary::InitLibrary();
	ColorMapManager::Initialize();
}
