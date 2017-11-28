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

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gltf2Importer.h"
#include "JsonObjectConverter.h"
#include "Regex.h"
#include "PolyAssetResponse.h"
#include "PolyToolkit.h"

UPolyToolkit* UPolyToolkit::PolyToolkitInstance = NULL;

UPolyToolkit::UPolyToolkit(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	HttpModule = &FHttpModule::Get();
	PendingDownloads = 0;
}

UPolyToolkit* UPolyToolkit::GetPolyToolkitInstance()
{
	if(PolyToolkitInstance == NULL || !PolyToolkitInstance->IsValidLowLevel())
	{
		PolyToolkitInstance = NewObject<UPolyToolkit>();
		PolyToolkitInstance->AddToRoot();
	}
	return PolyToolkitInstance;
}

void UPolyToolkit::GetAsset(const FString& ApiKey, const FString& AssetName, const FOnGetAssetComplete& OnGetAssetCallback)
{
	UPolyToolkit* PolyToolkit = GetPolyToolkitInstance();
	PolyToolkit->OnGetAssetComplete = OnGetAssetCallback;
	TSharedRef<IHttpRequest> Request = PolyToolkit->HttpModule->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(PolyToolkit, &UPolyToolkit::OnGetAssetResponseReceived);
	Request->SetURL("https://poly.googleapis.com/v1/" + AssetName + "?key="+ ApiKey);
	Request->SetVerb("GET");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->ProcessRequest();
}

void UPolyToolkit::OnGetAssetResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FPolyAssetResponse AssetResponse;
	if (Response.IsValid() && bWasSuccessful)
	{
		if(Response->GetResponseCode() == HTTP_RESPONSE_OK)
		{
			FPolyAsset Asset;
			if (FJsonObjectConverter::JsonObjectStringToUStruct(Response->GetContentAsString(), &Asset, 0, 0))
			{
				AssetResponse.PolyAsset = Asset;
				AssetResponse.Success = true;
				OnGetAssetComplete.ExecuteIfBound(AssetResponse);
				return;
			}
		}
	}
	AssetResponse.ErrorMessage = Response->GetContentAsString();
	AssetResponse.Success = false;
	OnGetAssetComplete.ExecuteIfBound(AssetResponse);
}

void UPolyToolkit::ListAssets(const FString& ApiKey, const FString& Keywords, bool Curated, EPolyCategory Category, EPolyComplexity MaxComplexity, const EPolyFormat Format, int32 PageSize, EPolyOrder OrderBy, const FString& PageToken, const FOnListAssetsComplete& OnListAssetsCallback)
{
	UPolyToolkit* PolyToolkit = GetPolyToolkitInstance();
	PolyToolkit->OnListAssetsComplete = OnListAssetsCallback;
	TSharedRef<IHttpRequest> Request = PolyToolkit->HttpModule->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(PolyToolkit, &UPolyToolkit::OnListAssetsResponseReceived);
	FString Params;
	if(!Keywords.IsEmpty())
	{
		Params += "&keywords="+Keywords.Replace(TEXT(" "), TEXT("%20"));
	}
	if(Curated)
	{
		Params += "&curated=true";
	}
	if(Category != EPolyCategory::Any)
	{
		Params += "&category="+EPolyCategoryToString(Category);
	}
	if(MaxComplexity != EPolyComplexity::Complex)
	{
		Params += "&max_complexity="+ EPolyComplexityToString(MaxComplexity);
	}
	if(Format != EPolyFormat::Any)
	{
		Params += "&format="+EPolyFormatToString(Format);
	}
	if(PageSize > 0)
	{
		Params += "&page_size="+FString::FromInt(PageSize);
	}
	if(OrderBy != EPolyOrder::Best)
	{
		Params += "&order_by="+EPolyOrderToString(OrderBy);
	}
	if(!PageToken.IsEmpty())
	{
		Params += "&page_token="+PageToken;
	}
	FString Url = "https://poly.googleapis.com/v1/assets?key="+ ApiKey + Params;
	Request->SetURL(Url);
	Request->SetVerb("GET");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->ProcessRequest();
}

void UPolyToolkit::OnListAssetsResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FPolyAssetListResponse AssetListResponse;
	if (Response.IsValid() && bWasSuccessful)
	{
		if(Response->GetResponseCode() == HTTP_RESPONSE_OK)
		{
			FPolyAssetList AssetList;
			if (FJsonObjectConverter::JsonObjectStringToUStruct(Response->GetContentAsString(), &AssetList, 0, 0))
			{
				AssetListResponse.PolyAssetList = AssetList;
				AssetListResponse.Success = true;
				OnListAssetsComplete.ExecuteIfBound(AssetListResponse);
				return;
			}
		}
	}
	AssetListResponse.ErrorMessage = Response->GetContentAsString();
	AssetListResponse.Success = false;
	OnListAssetsComplete.ExecuteIfBound(AssetListResponse);
}

void UPolyToolkit::ImportAsset(UObject* WorldContextObject, const FPolyAsset& Asset, const FOnImportAssetComplete& OnImportAssetCompleteCallback)
{
	UPolyToolkit* PolyToolkit = GetPolyToolkitInstance();
	PolyToolkit->OnImportAssetComplete = OnImportAssetCompleteCallback;
	PolyToolkit->WorldContextObject = WorldContextObject;

	bool SupportedFormatFound = false;
	for(auto& PolyFormat : Asset.formats)
	{
		if(PolyFormat.formatType == "GLTF2" || PolyFormat.formatType == "GLTF")
		{
			PolyToolkit->ImportedAsset = Asset;
			PolyToolkit->PendingDownloads = PolyFormat.resources.Num() + 1; // The root plus all the resources.
			PolyToolkit->DownloadResource(PolyFormat.root, Asset.name);
			for(auto& Resource : PolyFormat.resources)
			{
				PolyToolkit->DownloadResource(Resource, Asset.name);
			}
			SupportedFormatFound = true;
			break;
		}
	}

	if(!SupportedFormatFound)
	{
		FPolyActorResponse ActorResponse;
		ActorResponse.ErrorMessage = "No supported format was found. Currently only GLTF and GLTF2 formats are supported.";
		ActorResponse.Success = false;
		PolyToolkit->OnImportAssetComplete.ExecuteIfBound(ActorResponse);
	}
}

void UPolyToolkit::DownloadResource(const FPolyFile& File, const FString& AssetName)
{
	UHttpDownload* ResourceDownload = NewObject<UHttpDownload>();
	ResourceDownload->Download(File, AssetName, GetPolyToolkitInstance());
}

void UPolyToolkit::OnDownloadResourceComplete(bool Status)
{
	PendingDownloads--;
	if(PendingDownloads == 0)
	{
		ImportModel();
	}
}

void UPolyToolkit::ImportModel()
{
	FPolyActorResponse ActorResponse;

	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	AActor* PolyActor = World->SpawnActor<AActor>(AActor::StaticClass());

	bool Loaded = false;
	for (auto& FileFormat : ImportedAsset.formats)
	{
		if (FileFormat.formatType == "GLTF2")
		{
			UGltf2Importer* Gltf2Importer = NewObject<UGltf2Importer>();
			Gltf2Importer->ImportModel(FileFormat, ImportedAsset.name, PolyActor);
			Loaded = true;
			break;
		}
		else if(FileFormat.formatType == "GLTF")
		{
			UGltf1Importer* Gltf1Importer = NewObject<UGltf1Importer>();
			Gltf1Importer->ImportModel(FileFormat, ImportedAsset.name, PolyActor);
			Loaded = true;
			break;
		}
	}
	if(Loaded)
	{
		ActorResponse.Actor = PolyActor;
		ActorResponse.Success = true;
	}
	else
	{
		ActorResponse.ErrorMessage = "Model could not be imported";
		ActorResponse.Success = false;
	}
	OnImportAssetComplete.ExecuteIfBound(ActorResponse);
}

