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
#include "tiny_gltf_loader.h"

#include "Gltf1Importer.generated.h"

UCLASS()
	class UGltf1Importer : public UObject
{
	GENERATED_UCLASS_BODY()
public:
	/**
	 * Imports a glTF file generating meshes and materials. The result is
	 * attached to PolyActor as the root component.
	 */
	void ImportModel(const FPolyFormat& Format, const FString& AssetName, AActor* PolyActor);

private:
	void LoadScene(const std::vector<std::string>& SceneNodes, AActor* PolyActor);
	void LoadNode(const tinygltf::Node& Node, USceneComponent* Parent);
	void LoadMesh(const tinygltf::Mesh& Mesh, USceneComponent* Parent);
	void LoadPrimitive(const tinygltf::Primitive& Primitive, USceneComponent* Parent);
	UMaterialInstanceDynamic* LoadMaterial(const tinygltf::Material& Material, USceneComponent* Parent);

	int CalculateBytesPerComponent(int ComponentType);
	int CalculateNumComponents(int Type);

	template<typename T, typename U>
	TArray<T> LoadAttribute(const tinygltf::Accessor& accessor);

	// tinygltfloader Scene.
	tinygltf::Scene Scene;

	// Full path to asset folder.
	FString AssetPath;

	// Blocks Paper Material.
	UMaterial* PaperMaterial;
	// Blocks Glass Material.
	UMaterial* GlassMaterial;
	// Blocks Gem Material.
	UMaterial* GemMaterial;
};

