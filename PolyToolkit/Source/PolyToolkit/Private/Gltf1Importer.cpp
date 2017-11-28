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
#include "Gltf1Importer.h"

#define TINYGLTF_LOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include "tiny_gltf_loader.h"
#include "ConstructorHelpers.h"
#include "ProceduralMeshComponent.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include <cstring>

UGltf1Importer::UGltf1Importer(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	static ConstructorHelpers::FObjectFinder<UMaterial> PaperMaterialFinder(TEXT("Material'/PolyToolkit/PaperMaterial.PaperMaterial'"));
	if(PaperMaterialFinder.Succeeded())
	{
		PaperMaterial = PaperMaterialFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> GlassMaterialFinder(TEXT("Material'/PolyToolkit/GlassMaterial.GlassMaterial'"));
	if(GlassMaterialFinder.Succeeded())
	{
		GlassMaterial = GlassMaterialFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> GemMaterialFinder(TEXT("Material'/PolyToolkit/GemMaterial.GemMaterial'"));
	if(GemMaterialFinder.Succeeded())
	{
		GemMaterial = GemMaterialFinder.Object;
	}
}

void UGltf1Importer::ImportModel(const FPolyFormat& File, const FString& AssetName, AActor* PolyActor)
{
#if PLATFORM_ANDROID
	FString BasePath = "/sdcard/UE4Game/HelloPolyToolkit/HelloPolyToolkit/Content/";
#else
	FString BasePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
#endif
	AssetPath = FPaths::Combine(BasePath, AssetName);
	FString RootFilePath = FPaths::Combine(AssetPath, File.root.relativePath);

	UE_LOG(LogTemp, Warning, TEXT("GLTF1 File Path: %s"), *RootFilePath);
	tinygltf::TinyGLTFLoader Loader;
	std::string Err;
	bool Ret = false;
	Ret = Loader.LoadASCIIFromFile(&Scene, &Err, TCHAR_TO_ANSI(*RootFilePath));

	if(!Err.empty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Error parsing glTF: %s"), UTF8_TO_TCHAR(Err.c_str()));
	}

	if(!Ret){
		UE_LOG(LogTemp, Warning, TEXT("Failed to parse glTF file"));
		return;
	}

	if(!Scene.defaultScene.empty())
	{
		LoadScene(Scene.scenes[Scene.defaultScene], PolyActor);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No default scene"));
	}
}

void UGltf1Importer::LoadScene(const std::vector<std::string>& SceneNodes, AActor* PolyActor)
{
	// Create root component.
	UProceduralMeshComponent* Mesh = NewObject<UProceduralMeshComponent>(PolyActor);
	PolyActor->SetRootComponent(Mesh);
	Mesh->RegisterComponent();
	// Iterate through Nodes.
	for(auto& Node : SceneNodes)
	{
		LoadNode(Scene.nodes[Node], Mesh);
	}
}

void UGltf1Importer::LoadNode(const tinygltf::Node& Node, USceneComponent* Parent)
{
	// Create Node component.
	UProceduralMeshComponent* NodeComponent = NewObject<UProceduralMeshComponent>(Parent);
	NodeComponent->SetupAttachment(Parent);
	NodeComponent->RegisterComponent();
	// Apply transform from TRS.
	FVector scale;
	if(Node.rotation.size() == 3 && Node.rotation.size() == 4 && Node.translation.size() == 3)
	{
		FVector Scale(Node.scale[0], Node.scale[1], Node.scale[2]);
		FQuat Rotation(Node.rotation[0], Node.rotation[1], Node.rotation[2], Node.rotation[3]);
		FVector Translation(Node.translation[0], Node.translation[1], Node.translation[2]);
		FTransform Transform(Rotation, Translation, Scale);
		NodeComponent->SetRelativeTransform(Transform);
	}

	// Load meshes of component.
	for(auto& Mesh : Node.meshes)
	{
		LoadMesh(Scene.meshes[Mesh], NodeComponent);
	}

	// Load children of this Node.
	for (auto& Child : Node.children)
	{
		LoadNode(Scene.nodes[Child], NodeComponent);
	}
}

void UGltf1Importer::LoadMesh(const tinygltf::Mesh& Mesh, USceneComponent* Parent)
{
	// Load all the primitives of this mesh.
	for(auto& Primitive : Mesh.primitives)
	{
		LoadPrimitive(Primitive, Parent);
	}
}

void UGltf1Importer::LoadPrimitive(const tinygltf::Primitive& Primitive, USceneComponent* Parent)
{
	if(Primitive.mode != TINYGLTF_MODE_TRIANGLES)
	{
		UE_LOG(LogTemp, Warning, TEXT("Mode is not triangles, cannot be loaded."));
		return;
	}

	TArray<int32> Triangles;
	if(!Primitive.indices.empty())
	{
		Triangles = LoadAttribute<int32,int32>(Scene.accessors[Primitive.indices]);
	}

	TArray<FVector> Vertices;
	auto It = Primitive.attributes.find("POSITION");
	if (It != Primitive.attributes.end())
	{
		Vertices = LoadAttribute<FVector, float>(Scene.accessors[It->second]);
	}

	TArray<FVector> Normals;
	It = Primitive.attributes.find("NORMAL");
	if (It != Primitive.attributes.end())
	{
		Normals = LoadAttribute<FVector, float>(Scene.accessors[It->second]);
	}

	TArray<FVector2D> TextCoords;
	It = Primitive.attributes.find("TEXCOORD_0");
	if (It != Primitive.attributes.end())
	{
		TextCoords = LoadAttribute<FVector2D, float>(Scene.accessors[It->second]);
	}

	TArray<FColor> VertexColors;
	It = Primitive.attributes.find("COLOR");
	if(It != Primitive.attributes.end())
	{
		VertexColors = LoadAttribute<FColor, float>(Scene.accessors[It->second]);
	}

	//Create procedural mesh component for this primitive
	UProceduralMeshComponent* Mesh = NewObject<UProceduralMeshComponent>(Parent);
	Mesh->CreateMeshSection(0, Vertices, Triangles, Normals, TextCoords, VertexColors, TArray<FProcMeshTangent>(), false);
	Mesh->SetupAttachment(Parent);
	Mesh->RegisterComponent();

	UMaterialInstanceDynamic* MaterialInstance = NULL;
	if (!Primitive.material.empty())
	{
		MaterialInstance = LoadMaterial(Scene.materials[Primitive.material], Mesh);
	}

	Mesh->SetMaterial(0, MaterialInstance);
}

UMaterialInstanceDynamic* UGltf1Importer::LoadMaterial(const tinygltf::Material& Material, USceneComponent* Parent)
{
	UMaterialInstanceDynamic* MaterialInstance = NULL;
	tinygltf::Technique& Technique = Scene.techniques[Material.technique];

	const std::string& gvrss = Technique.extras.Get("gvrss").Get<std::string>();
	if(gvrss == "https://vr.google.com/shaders/w/gvrss/paper.json")
	{
		MaterialInstance = UMaterialInstanceDynamic::Create(PaperMaterial, Parent);
	}
	else if(gvrss == "https://vr.google.com/shaders/w/gvrss/glass.json")
	{
		MaterialInstance = UMaterialInstanceDynamic::Create(GlassMaterial, Parent);
	}
	else if(gvrss == "https://vr.google.com/shaders/w/gvrss/gem.json")
	{
		// TODO(pmanzi) Gem = Glass until Gem is properly implemented.
		MaterialInstance = UMaterialInstanceDynamic::Create(GlassMaterial, Parent);
	}

	return MaterialInstance;
}


	template<typename T, typename U>
TArray<T> UGltf1Importer::LoadAttribute(const tinygltf::Accessor& Accessor)
{
	TArray<T> Elements;

	int ElementSize = 0;
	int BytesPerComponent = CalculateBytesPerComponent(Accessor.componentType);
	int NumComponents = CalculateNumComponents(Accessor.type);
	ElementSize = BytesPerComponent * NumComponents;

	tinygltf::BufferView& BufferView = Scene.bufferViews[Accessor.bufferView];
	tinygltf::Buffer& Buffer = Scene.buffers[BufferView.buffer];

	U* Values = new U[NumComponents];
	for (unsigned int i = 0; i < Accessor.count; i++)
	{
		memset(Values, 0, sizeof(U) * NumComponents);
		int Stride = Accessor.byteStride ? Accessor.byteStride : ElementSize;

		for (int j = 0; j < NumComponents; j++)
		{
			char Component[4];
			memset(Component, 0, 4);
			memcpy(Component, Buffer.data.data() + Accessor.byteOffset + BufferView.byteOffset + Stride * i + BytesPerComponent * j, BytesPerComponent);
			Values[j] = *(reinterpret_cast<U*>(Component));
		}
		T* Element;
		if (std::is_same<T, FVector>::value)
		{
			FVector Vector(-Values[2] *100, Values[0]*100, Values[1]*100);
			Element = reinterpret_cast<T*> (&Vector);
			Elements.Add(*Element);
		}
		else if(std::is_same<T, FVector2D>::value)
		{
			FVector2D Vector2D(Values[0], Values[1]);
			Element = reinterpret_cast<T*> (&Vector2D);
			Elements.Add(*Element);
		}
		else if(std::is_same<T, int32>::value)
		{
			int32 Val = Values[0];
			Element = reinterpret_cast<T*> (&Val);
			Elements.Add(*Element);
		}
		else if(std::is_same<T, FColor>::value)
		{
			FLinearColor LinearColor(Values[0], Values[1], Values[2], Values[3]);
			FColor Color = LinearColor.ToFColor(false);
			Element = reinterpret_cast<T*> (&Color);
			Elements.Add(*Element);
		}
	}
	delete[] Values;
	return Elements;
}

template TArray<int32> UGltf1Importer::LoadAttribute<int32, int32>(const tinygltf::Accessor& Accessor);
template TArray<FVector> UGltf1Importer::LoadAttribute<FVector, float>(const tinygltf::Accessor& Accessor);
template TArray<FVector2D> UGltf1Importer::LoadAttribute<FVector2D, float>(const tinygltf::Accessor& Accessor);
template TArray<FColor> UGltf1Importer::LoadAttribute<FColor, float>(const tinygltf::Accessor& Accessor);

int UGltf1Importer::CalculateBytesPerComponent(int ComponentType)
{
	switch(ComponentType)
	{
		case TINYGLTF_COMPONENT_TYPE_BYTE:
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
			return 1;
		case TINYGLTF_COMPONENT_TYPE_SHORT:
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
			return 2;
		case TINYGLTF_COMPONENT_TYPE_INT:
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
		case TINYGLTF_COMPONENT_TYPE_FLOAT:
			return 4;
		default:
			return 0;
	}
}

int UGltf1Importer::CalculateNumComponents(int Type)
{
	switch(Type)
	{
		case TINYGLTF_TYPE_SCALAR:
			return 1;
		case TINYGLTF_TYPE_VEC2:
			return 2;
		case TINYGLTF_TYPE_VEC3:
			return 3;
		case TINYGLTF_TYPE_VEC4:
		case TINYGLTF_TYPE_MAT2:
			return 4;
		case TINYGLTF_TYPE_MAT3:
			return 9;
		case TINYGLTF_TYPE_MAT4:
			return 16;
		default:
			return 0;
	}
}

