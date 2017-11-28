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

namespace UnrealBuildTool.Rules
{
	public class PolyToolkit : ModuleRules
	{
		public PolyToolkit(ReadOnlyTargetRules Target) : base(Target)
		{
			PrivateIncludePaths.AddRange(
				new string[] {
					"PolyToolkit/Private",
					"PolyToolkit/ThirdParty/gltf2-loader/include",
					"PolyToolkit/ThirdParty/gltf2-loader/ext",
					"PolyToolkit/ThirdParty/tinygltfloader",
					}
			);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					"HTTP",
					"Json",
					"JsonUtilities",
					"ProceduralMeshComponent",
				}
			);

		}
	}
}
