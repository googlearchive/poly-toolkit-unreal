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
#include "PolyAsset.generated.h"

// All the fields in this file are lowercase to match Poly Http Rest API.

/**
 * Represents a file in Poly, which can be a root, resource, or thumbnail file.
 */
USTRUCT(BlueprintType)
struct FPolyFile
{
	GENERATED_USTRUCT_BODY()

	/** The path of the resource file relative to the root file. For root or thumbnail files, this is just the filename. */
	UPROPERTY(BlueprintReadWrite)
	FString relativePath;

	/** The URL where the file data can be retrieved. */
	UPROPERTY(BlueprintReadWrite)
	FString url;

	/** The MIME content-type, such as image/png. For more information, see MIME types.*/
	UPROPERTY(BlueprintReadWrite)
	FString contentType;

};

/**
 * Information on the complexity of this Format.
 */
USTRUCT(BlueprintType)
struct FPolyFormatComplexity
{
	GENERATED_USTRUCT_BODY()

	/** The estimated number of triangles. */
	UPROPERTY(BlueprintReadWrite)
	FString triangleCount;

	/** A non-negative integer that represents the level of detail (LOD) of this format relative to other formats of the same asset with the same formatType. This hint allows you to sort formats from the most-detailed (0) to least-detailed (integers greater than 0). */
	UPROPERTY(BlueprintReadWrite)
	int32 lod_hint;
};

/**
 * The same asset can be represented in different formats, for example, a
 * WaveFront .obj file with its corresponding .mtl file or a Khronos glTF file
 * with its corresponding .glb binary data. A format refers to a specific
 * representation of an asset and contains all information needed to retrieve
 * and describe this representation.
 */
USTRUCT(BlueprintType)
struct FPolyFormat
{
	GENERATED_USTRUCT_BODY()

	/**
	 * The root of the file hierarchy. This will always be populated. For
	 * some [format_types][Format.format_types] - such as TILT, which are
	 * self-contained - this is all of the data.
	 *
	 * Other types - such as OBJ - often reference other data elements.
	 * These are contained in the resources field.
	 */
	UPROPERTY(BlueprintReadWrite)
	FPolyFile root;

	/** A list of dependencies of the root element. May include, but is not limited to, materials, textures, and shader programs. */
	UPROPERTY(BlueprintReadWrite)
	TArray<FPolyFile> resources;

	/** Complexity stats about this representation of the asset. */
	UPROPERTY(BlueprintReadWrite)
	FPolyFormatComplexity format_complexity;

	/** A short string that identifies the format type of this representation. Possible values are: FBX, GLTF, GLTF2, OBJ, and TILT. */
	UPROPERTY(BlueprintReadWrite)
	FString formatType;
};

/**
 * Represents and describes an asset in the Poly library. An Asset is a 3D model or scene created
 * using Tilt Brush, Blocks, or any 3D program that produces a file that can be uploaded to Poly.
 * The core of any response from the Poly API is information on one or more Assets.
 */
USTRUCT(BlueprintType)
struct FPolyAsset
{
	GENERATED_USTRUCT_BODY()

	/** The unique identifier for the Asset in the form assets/{ASSET_ID}. */
	UPROPERTY(BlueprintReadWrite)
	FString name;

	/** The human-readable name, set by the asset's author. */
	UPROPERTY(BlueprintReadWrite)
	FString displayName;

	/**
	 * The author's publicly visible name. Use this name when giving credit
	 * to the author. For more information, see Licensing.
	 */
	UPROPERTY(BlueprintReadWrite)
	FString authorName;

	/** The human-readable description, set by the asset's author. */
	UPROPERTY(BlueprintReadWrite)
	FString description;

	/**
	 * For published assets, the time when the asset was published. For
	 * unpublished assets, the time when the asset was created.
	 *
	 * A timestamp in RFC3339 UTC "Zulu" format, accurate to nanoseconds.
	 * Example: "2014-10-02T15:01:23.045123456Z".
	 */
	UPROPERTY(BlueprintReadWrite)
	FString createTime;

	/**
	 * The time when the asset was last modified. For published assets,
	 * whose contents are immutable, the update time changes only when
	 * metadata properties, such as visibility, are updated.
	 *
	 * A timestamp in RFC3339 UTC "Zulu" format, accurate to nanoseconds.
	 * Example: "2014-10-02T15:01:23.045123456Z".
	 */
	UPROPERTY(BlueprintReadWrite)
	FString updateTime;

	/** A list of Formats where each format describes one representation of the asset. */
	UPROPERTY(BlueprintReadWrite)
	TArray<FPolyFormat> formats;

	/** The thumbnail image for the asset. */
	UPROPERTY(BlueprintReadWrite)
	FPolyFile thumbnail;

	/** Specifies the license under which the author has made the asset available for use, if any. */
	UPROPERTY(BlueprintReadWrite)
	FString license;

	/** Specifies the visibility of the asset and who can access it. */
	UPROPERTY(BlueprintReadWrite)
	FString visibility;

	/** Specifies whether this asset has been curated by the Poly team. */
	UPROPERTY(BlueprintReadWrite)
	bool is_curated;
};
