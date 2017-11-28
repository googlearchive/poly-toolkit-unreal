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
#include "PolyAssetList.h"
#include "PolyAssetResponse.h"
#include "PolyAssetListResponse.h"
#include "PolyActorResponse.h"

#include "PolyToolkit.generated.h"

#define HTTP_RESPONSE_OK 200

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGetAssetComplete, FPolyAssetResponse, PolyAssetResponse);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnListAssetsComplete, FPolyAssetListResponse, PolyAssetListResponse);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnImportAssetComplete, FPolyActorResponse, PolyActorResponse);

/**
 * A UObject that encapsulates the PolyToolkit API.
 * PolyToolkit is a singleton.
 */
UCLASS(BlueprintType, Blueprintable)
class POLYTOOLKIT_API UPolyToolkit : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/**
	 * Get the PolyToolkit instance.
	 */
        static UPolyToolkit* GetPolyToolkitInstance();

	/**
	 * Request full information on a single Asset given an Asset id.
	 *
	 * @param AssetId	The id of Asset to retrieve.
	 * @param OnGetAssetCallback	A callback to be executed if the Asset is succesfully retrieved.
	 */
	UFUNCTION(BlueprintCallable, Category="PolyToolkit")
	static void GetAsset(const FString& ApiKey, const FString& AssetName, const FOnGetAssetComplete& OnGetAssetCallback);

	/**
         * List, search and filter public Assets given a criteria.
         *
         * @param Keywords      One or more search terms to be matched against all text that Poly has indexed for assets, including title, description and tags. Multiple keywords should be separated by spaces.
	 * @param Curated	Restricts to only results that have been ‘curated’ by the Poly team.
	 * @param Category	Restricts results to only Assets in the given category.
	 * @param MaxComplexity	Restricts results to only Assets of at most the given model complexity.
	 * @param Format	Restricts results to only Assets that contain the given format.
	 * @param PageSize	Specifies the number of results to return in the first page. This defaults to 100 and has a maximum value of 1000.
	 * @param OrderBy	Specifies an ordering for the Assets returned.
	 * @param PageToken	Specifies a continuation token for a previous search whose results were split into multiple pages.
         * @param OnListAssetsCallback    A callback to be executed if the list of Assets is succesfully retrieved.
         */
	UFUNCTION(BlueprintCallable, meta = (HidePin = "PageSize|OrderBy|PageToken"), Category="PolyToolkit")
	static void ListAssets(const FString& ApiKey, const FString& Keywords, bool Curated, EPolyCategory Category, EPolyComplexity MaxComplexity, EPolyFormat Format, int32 PageSize, EPolyOrder OrderBy, const FString& PageToken, const FOnListAssetsComplete& OnListAssetsCallback);

	/**
	 * Imports an Asset at runtime.
	 *
	 * @param Asset	The Asset to be loaded. This should be returned by GetAsset or ListAssets.
	 * @param OnImportCompleteCallback	A callback to be executed after loading the model.
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = WorldContextObject), Category="PolyToolkit")
	static void ImportAsset(UObject* WorldContextObject, const FPolyAsset& Asset, const FOnImportAssetComplete& OnImportCompleteCallback);

private:
	void ImportModel();

	void OnGetAssetResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnListAssetsResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

public:
	/** @private */
	void OnDownloadResourceComplete(bool Status);

public:
	// Callback delegates.
	FOnGetAssetComplete OnGetAssetComplete;
	FOnListAssetsComplete OnListAssetsComplete;
	FOnImportAssetComplete OnImportAssetComplete;

private:
	void DownloadResource(const FPolyFile& File, const FString& AssetName);

	// Singleton instance.
	static UPolyToolkit* PolyToolkitInstance;

	UObject* WorldContextObject;
	FHttpModule* HttpModule;

	// Fields for importing  asset and downloading resources.
	int32 PendingDownloads;
	FPolyAsset ImportedAsset;
};

