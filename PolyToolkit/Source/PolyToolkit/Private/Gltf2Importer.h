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
#include "Engine.h"
#include "GameFramework/Actor.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "PolyAsset.h"
#include "gltf2/glTF2.hpp"

#if PLATFORM_WINDOWS
#include <WindowsHWrapper.h>
#endif

#include "Gltf2Importer.generated.h"

UCLASS()
class UGltf2Importer : public UObject
{
	GENERATED_UCLASS_BODY()
public:
	/**
	 * Imports a glTF2 file generating meshes and materials. The result is
	 * attached to PolyActor as the root component.
	 */
	void ImportModel(const FPolyFormat& Format, const FString& AssetName, AActor* PolyActor);

private:
	void LoadScene(const gltf2::Scene& Scene, AActor* PolyActor);
	void LoadNode(const gltf2::Node& Node, USceneComponent* Parent);
	void LoadMesh(const gltf2::Mesh& Mesh, USceneComponent* Parent);
	void LoadPrimitive(const gltf2::Primitive& Primitive, USceneComponent* Parent);
	UMaterialInstanceDynamic* LoadMaterial(const gltf2::Material& Material, USceneComponent* Parent);

	int CalculateBytesPerComponent(gltf2::Accessor::ComponentType ComponentType);
	int CalculateNumComponents(gltf2::Accessor::Type Type);
	UTexture2D* LoadTexture2DFromFile(const FString& FullFilePath, EImageFormat ImageFormat);

	template<typename T, typename U>
	TArray<T> LoadAttribute(const gltf2::Accessor& accessor);

	// gltf2-loader Asset
	gltf2::Asset Asset;

	// Full path to asset folder.
	FString AssetPath;

	// Opaque material.
	UMaterial* PbrMaterial;
	// Blend Material.
	UMaterial* PbrMaterialTranslucent;
};
