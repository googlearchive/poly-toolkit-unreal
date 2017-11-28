// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "PolyAsset.h"
#include "PolyToolkit.h"

#include "HttpDownload.generated.h"

UCLASS()
class UHttpDownload : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Download a PolyFile and store it the the game's content folder. Calls
	 * PolyToolkit OnDownloadResourceComplete when the download is completed.
	 */
	void Download(const FPolyFile& File, const FString& AssetName, UPolyToolkit* PolyToolkit);
private:
	void OnDownloadResourceResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	FPolyFile File;
	FString AssetName;
	FHttpModule* HttpModule;
	UPolyToolkit* PolyToolkit;
};

