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
#include "HttpDownload.h"

void UHttpDownload::Download(const FPolyFile& File, const FString& AssetName, UPolyToolkit* PolyToolkit)
{
	HttpModule = &FHttpModule::Get();
	this->File = File;
	this->AssetName = AssetName;
	this->PolyToolkit = PolyToolkit;
	TSharedRef<IHttpRequest> Request = HttpModule->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UHttpDownload::OnDownloadResourceResponseReceived);
	Request->SetURL(File.url);
	Request->SetVerb("GET");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
	Request->ProcessRequest();
}

void UHttpDownload::OnDownloadResourceResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (Response.IsValid() && bWasSuccessful)
	{
		if(Response->GetResponseCode() == HTTP_RESPONSE_OK)
		{
#if PLATFORM_ANDROID
			FString base = "/HelloPolyToolkit/Content/";
#else
			FString base = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
#endif
			FString path = FPaths::Combine(base, AssetName, File.relativePath);
			FFileHelper::SaveArrayToFile(Response->GetContent(),*path);
			PolyToolkit->OnDownloadResourceComplete(false);
			return;
		}
	}
	PolyToolkit->OnDownloadResourceComplete(false);
}
