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
#include "Gltf2Importer.h"

#include "ConstructorHelpers.h"
#include "ProceduralMeshComponent.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include <cstring>

UGltf2Importer::UGltf2Importer(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	static ConstructorHelpers::FObjectFinder<UMaterial> PbrMaterialFinder(TEXT("Material'/PolyToolkit/PbrMaterial.PbrMaterial'"));
	if(PbrMaterialFinder.Succeeded())
	{
		PbrMaterial = PbrMaterialFinder.Object;
	}
	static ConstructorHelpers::FObjectFinder<UMaterial> PbrMatTranslucentFinder(TEXT("Material'/PolyToolkit/PbrMaterialTranslucent.PbrMaterialTranslucent'"));
	if(PbrMatTranslucentFinder.Succeeded())
	{
		PbrMaterialTranslucent = PbrMatTranslucentFinder.Object;
	}
}


void UGltf2Importer::ImportModel(const FPolyFormat& File, const FString& AssetName, AActor* PolyActor)
{
#if PLATFORM_ANDROID
	FString BasePath = "/sdcard/UE4Game/HelloPolyToolkit/HelloPolyToolkit/Content/";
#else
	FString BasePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
#endif
	AssetPath = FPaths::Combine(BasePath, AssetName);
	FString RootFilePath = FPaths::Combine(AssetPath, File.root.relativePath);
	Asset = gltf2::load(TCHAR_TO_ANSI(*RootFilePath));

	if(Asset.metadata.version != "2.0")
	{
		UE_LOG(LogTemp, Warning, TEXT("Version %s not supported"), UTF8_TO_TCHAR(Asset.metadata.version.c_str()));
	}

	if(Asset.scene != -1)
	{
		LoadScene(Asset.scenes[Asset.scene], PolyActor);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No default scene"));
	}
}

void UGltf2Importer::LoadScene(const gltf2::Scene& Scene, AActor* PolyActor)
{
	// Create root component.
	UProceduralMeshComponent* Mesh = NewObject<UProceduralMeshComponent>(PolyActor);
	PolyActor->SetRootComponent(Mesh);
	Mesh->RegisterComponent();
	// Iterate through Nodes.
	for(int i = 0; i < Scene.nodes.size(); i++)
	{
		LoadNode(Asset.nodes[Scene.nodes[i]], Mesh);
	}
}

void UGltf2Importer::LoadNode(const gltf2::Node& Node, USceneComponent* Parent)
{
	// Create Node component.
	UProceduralMeshComponent* NodeComponent = NewObject<UProceduralMeshComponent>(Parent);
	NodeComponent->SetupAttachment(Parent);
	NodeComponent->RegisterComponent();
	// Apply transform from TRS.

	FVector Scale(Node.scale[0], Node.scale[1], Node.scale[2]);
	FQuat Rotation(Node.rotation[0], Node.rotation[1], Node.rotation[2], Node.rotation[3]);
	FVector Translation(Node.translation[0], Node.translation[1], Node.translation[2]);
	FTransform Transform(Rotation, Translation, Scale);
	NodeComponent->SetRelativeTransform(Transform);

	// Load mesh of component.
	if(Node.mesh != -1)
	{
		LoadMesh(Asset.meshes[Node.mesh], NodeComponent);
	}

	// Load children of this Node.
	for (int i = 0; i < Node.children.size(); i++)
	{
		LoadNode(Asset.nodes[Node.children[i]], NodeComponent);
	}
}

void UGltf2Importer::LoadMesh(const gltf2::Mesh& Mesh, USceneComponent* Parent)
{
	// Load all the primitives of this mesh.
	for(int i = 0; i < Mesh.primitives.size(); i++)
	{
		LoadPrimitive(Mesh.primitives[i], Parent);
	}
}

void UGltf2Importer::LoadPrimitive(const gltf2::Primitive& Primitive, USceneComponent* Parent)
{
	if(Primitive.mode != gltf2::Primitive::Mode::Triangles)
	{
		UE_LOG(LogTemp, Warning, TEXT("Mode is not triangles, cannot be loaded."));
		return;
	}

	TArray<int32> Triangles;
	if(Primitive.indices != -1)
	{
		Triangles = LoadAttribute<int32,int32>(Asset.accessors[Primitive.indices]);
	}

	TArray<FVector> Vertices;
	auto it = Primitive.attributes.find("POSITION");
	if(it!= Primitive.attributes.end())
	{
		Vertices = LoadAttribute<FVector, float>(Asset.accessors[it->second]);
	}

	TArray<FVector> Normals;
	it = Primitive.attributes.find("NORMAL");
	if(it!= Primitive.attributes.end())
	{
		Normals = LoadAttribute<FVector, float>(Asset.accessors[it->second]);
	}

	TArray<FVector2D> TextCoords;
	it = Primitive.attributes.find("TEXCOORD_0");
	if(it!= Primitive.attributes.end())
	{
		TextCoords = LoadAttribute<FVector2D, float>(Asset.accessors[it->second]);
	}

	//Create procedural mesh component for this primitive
	UProceduralMeshComponent* Mesh = NewObject<UProceduralMeshComponent>(Parent);
	Mesh->CreateMeshSection(0, Vertices, Triangles, Normals, TextCoords, TArray<FColor>(), TArray<FProcMeshTangent>(), false);
	Mesh->SetupAttachment(Parent);
	Mesh->RegisterComponent();

	UMaterialInstanceDynamic* MaterialInstance = NULL;
	if (Primitive.material != -1)
	{
		MaterialInstance = LoadMaterial(Asset.materials[Primitive.material], Mesh);
	}

	Mesh->SetMaterial(0, MaterialInstance);
}

UMaterialInstanceDynamic* UGltf2Importer::LoadMaterial(const gltf2::Material& Material, USceneComponent* Parent)
{
	UMaterialInstanceDynamic* MaterialInstance;
	if(Material.alphaMode == gltf2::Material::AlphaMode::Blend)
	{
		MaterialInstance = UMaterialInstanceDynamic::Create(PbrMaterialTranslucent, Parent);
	}
	else
	{
		MaterialInstance = UMaterialInstanceDynamic::Create(PbrMaterial, Parent);
	}

	MaterialInstance->SetScalarParameterValue(FName(TEXT("MetallicFactor")), Material.pbr.metallicFactor);
	MaterialInstance->SetScalarParameterValue(FName(TEXT("RoughnessFactor")), Material.pbr.roughnessFactor);
	FLinearColor BaseColorFactor(Material.pbr.baseColorFactor[0], Material.pbr.baseColorFactor[1], Material.pbr.baseColorFactor[2], Material.pbr.baseColorFactor[3]);
	MaterialInstance->SetVectorParameterValue(FName(TEXT("BaseColorFactor")), BaseColorFactor);
	MaterialInstance->TwoSided = Material.doubleSided;

	if (Material.pbr.baseColorTexture.index != -1)
	{
		gltf2::Texture& Texture = Asset.textures[Material.pbr.baseColorTexture.index];
		gltf2::Image& BaseColorImage = Asset.images[Texture.source];
		EImageFormat ImageFormat = EImageFormat::Invalid;
		if (BaseColorImage.mimeType == "image/png")
		{
			ImageFormat = EImageFormat::PNG;
		}
		else if (BaseColorImage.mimeType == "image/jpeg")
		{
			ImageFormat = EImageFormat::JPEG;
		}
#if PLATFORM_ANDROID
		FString TexturePath;
		const FRegexPattern Pattern(TEXT("^\\/sdcard\\/UE4Game\\/HelloPolyToolkit(\\/HelloPolyToolkit.*)$"));
		FRegexMatcher Matcher(Pattern, UTF8_TO_TCHAR(BaseColorImage.uri.c_str()));
		if(Matcher.FindNext())
		{
			TexturePath = Matcher.GetCaptureGroup(1);
		}
#else
		FString TexturePath = BaseColorImage.uri.c_str();
#endif
		UTexture2D* BaseColorTexture = LoadTexture2DFromFile(TexturePath, ImageFormat);
		MaterialInstance->SetTextureParameterValue(FName(TEXT("BaseColorTexture")), BaseColorTexture);
	}
	return MaterialInstance;
}

template<typename T, typename U>
TArray<T> UGltf2Importer::LoadAttribute(const gltf2::Accessor& Accessor)
{
	TArray<T> Elements;

	int ElementSize = 0;
	int BytesPerComponent = CalculateBytesPerComponent(Accessor.componentType);
	int NumComponents = CalculateNumComponents(Accessor.type);
	ElementSize = BytesPerComponent * NumComponents;

	gltf2::BufferView& BufferView = Asset.bufferViews[Accessor.bufferView];
	gltf2::Buffer& Buffer = Asset.buffers[BufferView.buffer];

	U* Values = new U[NumComponents];
	for (unsigned int i = 0; i < Accessor.count; i++)
	{
		memset(Values, 0, sizeof(U) * NumComponents);
		int Stride = BufferView.byteStride ? BufferView.byteStride : ElementSize;

		for (int j = 0; j < NumComponents; j++)
		{
			char Component[4];
			memset(Component, 0, 4);
			memcpy(Component, Buffer.data + Accessor.byteOffset + BufferView.byteOffset + Stride * i + BytesPerComponent * j, BytesPerComponent);
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
	}
	delete[] Values;
	return Elements;
}

template TArray<int32> UGltf2Importer::LoadAttribute<int32, int32>(const gltf2::Accessor& Accessor);
template TArray<FVector> UGltf2Importer::LoadAttribute<FVector, float>(const gltf2::Accessor& Accessor);
template TArray<FVector2D> UGltf2Importer::LoadAttribute<FVector2D, float>(const gltf2::Accessor& Accessor);

int UGltf2Importer::CalculateBytesPerComponent(gltf2::Accessor::ComponentType ComponentType)
{
	switch(ComponentType)
	{
		case gltf2::Accessor::ComponentType::Byte:
		case gltf2::Accessor::ComponentType::UnsignedByte:
			return 1;
		case gltf2::Accessor::ComponentType::Short:
		case gltf2::Accessor::ComponentType::UnsignedShort:
			return 2;
		case gltf2::Accessor::ComponentType::UnsignedInt:
		case gltf2::Accessor::ComponentType::Float:
			return 4;
		default:
			return 0;
	}
}

int UGltf2Importer::CalculateNumComponents(gltf2::Accessor::Type Type)
{
	switch(Type)
	{
		case gltf2::Accessor::Type::Scalar:
			return 1;
		case gltf2::Accessor::Type::Vec2:
			return 2;
		case gltf2::Accessor::Type::Vec3:
			return 3;
		case gltf2::Accessor::Type::Vec4:
		case gltf2::Accessor::Type::Mat2:
			return 4;
		case gltf2::Accessor::Type::Mat3:
			return 9;
		case gltf2::Accessor::Type::Mat4:
			return 16;
		default:
			return 0;
	}
}

UTexture2D* UGltf2Importer::LoadTexture2DFromFile(const FString& FullFilePath, EImageFormat ImageFormat)
{
	UTexture2D* LoadedT2D = NULL;

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);

	// Load File
	TArray<uint8> RawFileData;
	if (!FFileHelper::LoadFileToArray(RawFileData, * FullFilePath))
	{
		return NULL;
	}

	// Create Texture
	if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
	{
		const TArray<uint8>* UncompressedBGRA = NULL;
		if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
		{
			LoadedT2D = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);
			if (!LoadedT2D)
			{
				return NULL;
			}

			// Copy Data
			void* TextureData = LoadedT2D->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(TextureData, UncompressedBGRA->GetData(), UncompressedBGRA->Num());
			LoadedT2D->PlatformData->Mips[0].BulkData.Unlock();
			LoadedT2D->UpdateResource();
		}
	}

	return LoadedT2D;
}

