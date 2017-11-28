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
#include "PolyAsset.h"
#include "PolyAssetList.generated.h"

// All the fields in PolyAssetList struct are lowercase to match Poly Http Rest API.

/// @defgroup PolyToolkit Poly Toolkit

/**
 * @ingroup PolyToolkit
 * An enum that describes the complexity to filter assets.
 */
UENUM(BlueprintType)
enum class EPolyComplexity : uint8
{
	/** Returns all results, regardless of their complexity. */
	Complex,
	/** Returns only Assets of simple complexity, and medium complexity. */
	Medium,
	/** Returns only simple  assets. */
	Simple
};

inline FString EPolyComplexityToString(const EPolyComplexity& Complexity)
{
	switch(Complexity)
	{
		case EPolyComplexity::Complex:
			return TEXT("COMPLEX");
		case EPolyComplexity::Medium:
			return TEXT("MEDIUM");
		case EPolyComplexity::Simple:
			return TEXT("SIMPLE");
		default:
			return TEXT("COMPLEX");
	}
}

/**
 * @ingroup PolyToolkit
 * An enum to specify an ordering for the Assets returned.
 */
UENUM(BlueprintType)
enum class EPolyOrder : uint8
{
	/** Orders Assets by an internal ranking, based on a combination of popularity and other features. */
	Best,
	/** Orders Assets by the time they were created, with newest results first. */
	Newest,
	/** Orders Assets by the time they were created, with oldest results first. */
	Oldest
};

inline FString EPolyOrderToString(const EPolyOrder& Order)
{
	switch(Order)
	{
		case EPolyOrder::Best:
			return TEXT("BEST");
		case EPolyOrder::Newest:
			return TEXT("NEWEST");
		case EPolyOrder::Oldest:
			return TEXT("OLDEST");
		default:
			return TEXT("BEST");
	}
}

/**
 * @ingroup PolyToolkit
 * An enum to restrict results to only Assets that contain the given format.
 */
UENUM(BlueprintType)
enum class EPolyFormat : uint8
{
	/** OBJ. */
	Obj,
	/** GLTF 1. */
	Gltf,
	/** GLTF 2. */
	Gltf2,
	/** Tilt Brush. */
	Tilt,
	/** Blocks. */
	Blocks,
	/** Any format. */
	Any
};

inline FString EPolyFormatToString(const EPolyFormat& Format){
	switch(Format)
	{
		case EPolyFormat::Obj:
			return TEXT("OBJ");
		case EPolyFormat::Gltf:
			return TEXT("GLTF");
		case EPolyFormat::Gltf2:
			return TEXT("GLTF2");
		case EPolyFormat::Tilt:
			return TEXT("TILT");
		case EPolyFormat::Blocks:
			return TEXT("BLOCKS");
		default:
			return TEXT("GLTF");
	}
}

/**
 * @ingroup PolyToolkit
 * An enum to restrict result to only Assets from a specific category.
 */
UENUM(BlueprintType)
enum class EPolyCategory : uint8
{
	/** */
	Animals,
	/** */
	Architecture,
	/** */
	Art,
	/** */
	Food,
	/** */
	Nature,
	/** */
	Objects,
	/** */
	People,
	/** */
	Scenes,
	/** */
	Technology,
	/** */
	Transport,
	/** */
	Any
};

inline FString EPolyCategoryToString(const EPolyCategory& Category)
{
	switch(Category)
	{
		case EPolyCategory::Animals:
			return TEXT("animals");
		case EPolyCategory::Architecture:
			return TEXT("architecture");
		case EPolyCategory::Art:
			return TEXT("art");
		case EPolyCategory::Food:
			return TEXT("food");
		case EPolyCategory::Nature:
			return TEXT("nature");
		case EPolyCategory::Objects:
			return TEXT("objects");
		case EPolyCategory::People:
			return TEXT("people");
		case EPolyCategory::Scenes:
			return TEXT("scenes");
		case EPolyCategory::Technology:
			return TEXT("technology");
		case EPolyCategory::Transport:
			return TEXT("transport");
		default:
			return TEXT("");
	}
}

/**
 * A struct that describes a list of Assets returned by ListAssets function.
 */
USTRUCT(BlueprintType)
struct FPolyAssetList
{
	GENERATED_USTRUCT_BODY()

	/** An array of Assets, where each entry represents one Asset matching the specified criteria. */
	UPROPERTY(BlueprintReadWrite)
	TArray<FPolyAsset> assets;

	/** A token to retrieve the next page of results. If empty, there are no further pages available. */
	UPROPERTY(BlueprintReadWrite)
	FString nextPageToken;

	/** The total number of results generated in response to this request. Note that this figure is independent of pagination, and will be identical for each page retrieved. */
	UPROPERTY(BlueprintReadWrite)
	int32 totalSize;
};
