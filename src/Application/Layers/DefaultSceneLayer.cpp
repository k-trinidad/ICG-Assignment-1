#include "DefaultSceneLayer.h"

// GLM math library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <GLM/gtx/common.hpp> // for fmod (floating modulus)

#include <filesystem>

// Graphics
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/VertexArrayObject.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Textures/TextureCube.h"
#include "Graphics/VertexTypes.h"
#include "Graphics/Font.h"
#include "Graphics/GuiBatcher.h"
#include "Graphics/Framebuffer.h"

// Utilities
#include "Utils/MeshBuilder.h"
#include "Utils/MeshFactory.h"
#include "Utils/ObjLoader.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/FileHelpers.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/StringUtils.h"
#include "Utils/GlmDefines.h"

// Gameplay
#include "Gameplay/Material.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"

// Components
#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Components/Camera.h"
#include "Gameplay/Components/RotatingBehaviour.h"
#include "Gameplay/Components/JumpBehaviour.h"
#include "Gameplay/Components/RenderComponent.h"
#include "Gameplay/Components/MaterialSwapBehaviour.h"
#include "Gameplay/Components/TriggerVolumeEnterBehaviour.h"
#include "Gameplay/Components/SimpleCameraControl.h"
#include "Gameplay/Components/CarBehaviour.h"
#include "Gameplay/Components/LightBehaviour.h"

// Physics
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Physics/Colliders/BoxCollider.h"
#include "Gameplay/Physics/Colliders/PlaneCollider.h"
#include "Gameplay/Physics/Colliders/SphereCollider.h"
#include "Gameplay/Physics/Colliders/ConvexMeshCollider.h"
#include "Gameplay/Physics/Colliders/CylinderCollider.h"
#include "Gameplay/Physics/TriggerVolume.h"
#include "Graphics/DebugDraw.h"

// GUI
#include "Gameplay/Components/GUI/RectTransform.h"
#include "Gameplay/Components/GUI/GuiPanel.h"
#include "Gameplay/Components/GUI/GuiText.h"
#include "Gameplay/InputEngine.h"

#include "Application/Application.h"
#include "Gameplay/Components/ParticleSystem.h"
#include "Graphics/Textures/Texture3D.h"
#include "Graphics/Textures/Texture1D.h"

DefaultSceneLayer::DefaultSceneLayer() :
	ApplicationLayer()
{
	Name = "Default Scene";
	Overrides = AppLayerFunctions::OnAppLoad;
}

DefaultSceneLayer::~DefaultSceneLayer() = default;

void DefaultSceneLayer::OnAppLoad(const nlohmann::json& config) {
	_CreateScene();
}

void DefaultSceneLayer::_CreateScene()
{
	using namespace Gameplay;
	using namespace Gameplay::Physics;

	Application& app = Application::Get();

	bool loadScene = false;
	// For now we can use a toggle to generate our scene vs load from file
	if (loadScene && std::filesystem::exists("scene.json")) {
		app.LoadScene("scene.json");
	} else {
		// This time we'll have 2 different shaders, and share data between both of them using the UBO
		// This shader will handle reflective materials 
		ShaderProgram::Sptr reflectiveShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_environment_reflective.glsl" }
		});
		reflectiveShader->SetDebugName("Reflective");

		// This shader handles our basic materials without reflections (cause they expensive)
		ShaderProgram::Sptr basicShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_blinn_phong_textured.glsl" }
		});
		basicShader->SetDebugName("Blinn-phong");

		// This shader handles our basic materials without reflections (cause they expensive)
		ShaderProgram::Sptr specShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/textured_specular.glsl" }
		});
		specShader->SetDebugName("Textured-Specular");

		// This shader handles our foliage vertex shader example
		ShaderProgram::Sptr foliageShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/foliage.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/screendoor_transparency.glsl" }
		});
		foliageShader->SetDebugName("Foliage");

		// This shader handles our cel shading example
		ShaderProgram::Sptr toonShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/toon_shading.glsl" }  
		});
		toonShader->SetDebugName("Toon Shader");

		// This shader handles our displacement mapping example 
		ShaderProgram::Sptr displacementShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/displacement_mapping.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_tangentspace_normal_maps.glsl" }
		});
		displacementShader->SetDebugName("Displacement Mapping");

		// This shader handles our tangent space normal mapping
		ShaderProgram::Sptr tangentSpaceMapping = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_tangentspace_normal_maps.glsl" }
		});
		tangentSpaceMapping->SetDebugName("Tangent Space Mapping"); 

		// This shader handles our multitexturing example
		ShaderProgram::Sptr multiTextureShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/vert_multitextured.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_multitextured.glsl" }
		});
		multiTextureShader->SetDebugName("Multitexturing");   
		 
		// Load in the meshes
		MeshResource::Sptr monkeyMesh = ResourceManager::CreateAsset<MeshResource>("Monkey.obj");
		MeshResource::Sptr carMesh = ResourceManager::CreateAsset<MeshResource>("car.obj");
		MeshResource::Sptr decorationMesh = ResourceManager::CreateAsset<MeshResource>("decorations.obj");
		MeshResource::Sptr roadMesh = ResourceManager::CreateAsset<MeshResource>("road5.obj");
		MeshResource::Sptr groundMesh = ResourceManager::CreateAsset<MeshResource>("grass2.obj");
		MeshResource::Sptr sidewalkMesh = ResourceManager::CreateAsset<MeshResource>("sidewalk.obj");

		// Load in some textures
		Texture2D::Sptr    boxTexture   = ResourceManager::CreateAsset<Texture2D>("textures/box-diffuse.png");
		Texture2D::Sptr    boxSpec      = ResourceManager::CreateAsset<Texture2D>("textures/box-specular.png");
		Texture2D::Sptr    monkeyTex    = ResourceManager::CreateAsset<Texture2D>("textures/monkey-uvMap.png");
		Texture2D::Sptr    leafTex      = ResourceManager::CreateAsset<Texture2D>("textures/leaves.png");
		Texture2D::Sptr	   carTexture   = ResourceManager::CreateAsset<Texture2D>("textures/Assignment 1/car.png");
		Texture2D::Sptr	   roadTexture = ResourceManager::CreateAsset<Texture2D>("textures/Assignment 1/roadTile.png");
		Texture2D::Sptr	   autumnTexture = ResourceManager::CreateAsset<Texture2D>("textures/Assignment 1/autumn.png");
		Texture2D::Sptr	   groundTexture = ResourceManager::CreateAsset<Texture2D>("textures/Assignment 1/groundTile.png");
		Texture2D::Sptr	   skyTexture = ResourceManager::CreateAsset<Texture2D>("textures/Assignment 1/sky.png");
		leafTex->SetMinFilter(MinFilter::Nearest);
		leafTex->SetMagFilter(MagFilter::Nearest);
		  

		// Loading in a 1D LUT
		Texture1D::Sptr toonLut = ResourceManager::CreateAsset<Texture1D>("luts/toon-1D.png"); 
		toonLut->SetWrap(WrapMode::ClampToEdge);

		// Here we'll load in the cubemap, as well as a special shader to handle drawing the skybox
		TextureCube::Sptr testCubemap = ResourceManager::CreateAsset<TextureCube>("cubemaps/ocean/ocean.jpg");
		ShaderProgram::Sptr      skyboxShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/skybox_vert.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/skybox_frag.glsl" } 
		}); 

		// Create an empty scene 
		Scene::Sptr scene = std::make_shared<Scene>(); 

		// Setting up our enviroment map
		scene->SetSkyboxTexture(testCubemap); 
		scene->SetSkyboxShader(skyboxShader);
		// Since the skybox I used was for Y-up, we need to rotate it 90 deg around the X-axis to convert it to z-up 
		scene->SetSkyboxRotation(glm::rotate(MAT4_IDENTITY, glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f)));

		// Loading in a color lookup table
		Texture3D::Sptr lut = ResourceManager::CreateAsset<Texture3D>("luts/default.CUBE");

		// Configure the color correction LUT
		scene->SetColorLUT(lut);  

		// Create our materials
		 
		Material::Sptr carMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			carMaterial->Name = "Car";
			carMaterial->Set("u_Material.Diffuse", carTexture);
			carMaterial->Set("u_Material.Shininess", 0.1f);
			carMaterial->Set("u_Material.Toggle", 3);
			carMaterial->Set("u_Material.ToggleOn", false);
			carMaterial->Set("s_ToonTerm", toonLut);  
		}

		Material::Sptr roadMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			roadMaterial->Name = "Road";
			roadMaterial->Set("u_Material.Diffuse", roadTexture);
			roadMaterial->Set("u_Material.Shininess", 0.1f);
			roadMaterial->Set("u_Material.Toggle", 3);
			roadMaterial->Set("u_Material.ToggleOn", false);
			roadMaterial->Set("s_ToonTerm", toonLut);
		}

		Material::Sptr autumnMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			autumnMaterial->Name = "Autumn";
			autumnMaterial->Set("u_Material.Diffuse", autumnTexture);
			autumnMaterial->Set("u_Material.Shininess", 0.1f);
			autumnMaterial->Set("u_Material.Toggle", 3);
			autumnMaterial->Set("u_Material.ToggleOn", false);
			autumnMaterial->Set("s_ToonTerm", toonLut);
		}

		Material::Sptr groundMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			groundMaterial->Name = "Ground";
			groundMaterial->Set("u_Material.Diffuse", groundTexture);
			groundMaterial->Set("u_Material.Shininess", 0.1f);
			groundMaterial->Set("u_Material.Toggle", 3);
			groundMaterial->Set("u_Material.ToggleOn", false);
			groundMaterial->Set("s_ToonTerm", toonLut);
		}

		Material::Sptr skyMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			skyMaterial->Name = "Sky"; 
			skyMaterial->Set("u_Material.Diffuse", skyTexture);
			skyMaterial->Set("u_Material.Shininess", 0.1f);
			skyMaterial->Set("u_Material.Toggle", 3); 
			skyMaterial->Set("u_Material.ToggleOn", false);
			skyMaterial->Set("s_ToonTerm", toonLut);  
		}
		
		// This will be our box material, with no environment reflections
		Material::Sptr boxMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			boxMaterial->Name = "Box";
			boxMaterial->Set("u_Material.Diffuse", boxTexture);
			boxMaterial->Set("u_Material.Shininess", 0.1f);
		}

		// This will be the reflective material, we'll make the whole thing 90% reflective
		Material::Sptr monkeyMaterial = ResourceManager::CreateAsset<Material>(reflectiveShader);
		{
			monkeyMaterial->Name = "Monkey";
			monkeyMaterial->Set("u_Material.Diffuse", monkeyTex);
			monkeyMaterial->Set("u_Material.Shininess", 0.5f);
		}

		// This will be the reflective material, we'll make the whole thing 90% reflective
		Material::Sptr testMaterial = ResourceManager::CreateAsset<Material>(specShader);
		{
			testMaterial->Name = "Box-Specular";
			testMaterial->Set("u_Material.Diffuse", boxTexture);
			testMaterial->Set("u_Material.Specular", boxSpec);
		}

		// Our foliage vertex shader material 
		Material::Sptr foliageMaterial = ResourceManager::CreateAsset<Material>(foliageShader);
		{
			foliageMaterial->Name = "Foliage Shader";
			foliageMaterial->Set("u_Material.Diffuse", leafTex);
			foliageMaterial->Set("u_Material.Shininess", 0.1f);
			foliageMaterial->Set("u_Material.Threshold", 0.1f);

			foliageMaterial->Set("u_WindDirection", glm::vec3(1.0f, 1.0f, 0.0f));
			foliageMaterial->Set("u_WindStrength", 0.5f);
			foliageMaterial->Set("u_VerticalScale", 1.0f);
			foliageMaterial->Set("u_WindSpeed", 1.0f);
		}

		// Our toon shader material
		Material::Sptr toonMaterial = ResourceManager::CreateAsset<Material>(toonShader);
		{
			toonMaterial->Name = "Toon";
			toonMaterial->Set("u_Material.Diffuse", boxTexture);
			toonMaterial->Set("s_ToonTerm", toonLut);
			toonMaterial->Set("u_Material.Shininess", 0.1f);
			toonMaterial->Set("u_Material.Steps", 8);
		}


		Material::Sptr displacementTest = ResourceManager::CreateAsset<Material>(displacementShader);
		{
			Texture2D::Sptr displacementMap = ResourceManager::CreateAsset<Texture2D>("textures/displacement_map.png");
			Texture2D::Sptr normalMap       = ResourceManager::CreateAsset<Texture2D>("textures/normal_map.png");
			Texture2D::Sptr diffuseMap      = ResourceManager::CreateAsset<Texture2D>("textures/bricks_diffuse.png");

			displacementTest->Name = "Displacement Map";
			displacementTest->Set("u_Material.Diffuse", diffuseMap);
			displacementTest->Set("s_Heightmap", displacementMap);
			displacementTest->Set("s_NormalMap", normalMap);
			displacementTest->Set("u_Material.Shininess", 0.5f);
			displacementTest->Set("u_Scale", 0.1f);
		}

		Material::Sptr normalmapMat = ResourceManager::CreateAsset<Material>(tangentSpaceMapping);
		{
			Texture2D::Sptr normalMap       = ResourceManager::CreateAsset<Texture2D>("textures/normal_map.png");
			Texture2D::Sptr diffuseMap      = ResourceManager::CreateAsset<Texture2D>("textures/bricks_diffuse.png");

			normalmapMat->Name = "Tangent Space Normal Map";
			normalmapMat->Set("u_Material.Diffuse", diffuseMap);
			normalmapMat->Set("s_NormalMap", normalMap);
			normalmapMat->Set("u_Material.Shininess", 0.5f);
			normalmapMat->Set("u_Scale", 0.1f);
		}

		Material::Sptr multiTextureMat = ResourceManager::CreateAsset<Material>(multiTextureShader);
		{
			Texture2D::Sptr sand  = ResourceManager::CreateAsset<Texture2D>("textures/terrain/sand.png");
			Texture2D::Sptr grass = ResourceManager::CreateAsset<Texture2D>("textures/terrain/grass.png");

			multiTextureMat->Name = "Multitexturing";
			multiTextureMat->Set("u_Material.DiffuseA", sand);
			multiTextureMat->Set("u_Material.DiffuseB", grass);
			multiTextureMat->Set("u_Material.Shininess", 0.5f);
			multiTextureMat->Set("u_Scale", 0.1f);
		}

		// Create some lights for our scene
		scene->Lights.resize(1); 
		scene->Lights[0].Position = glm::vec3(0.0f, 4.0f, 14.0f); 
		scene->Lights[0].Color = glm::vec3(1.0f, 1.0f, 1.0f);
		scene->Lights[0].Range = 200.0f;

		// We'll create a mesh that is a simple plane that we can resize later
		MeshResource::Sptr planeMesh = ResourceManager::CreateAsset<MeshResource>();
		planeMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(1.0f)));
		planeMesh->GenerateMesh();

		MeshResource::Sptr sphere = ResourceManager::CreateAsset<MeshResource>();
		sphere->AddParam(MeshBuilderParam::CreateIcoSphere(ZERO, ONE, 5));
		sphere->GenerateMesh();

		// Set up the scene's camera
		GameObject::Sptr camera = scene->MainCamera->GetGameObject()->SelfRef();
		{
			camera->SetPostion({ -8.670, -6.110, 8.460 });
			camera->SetRotation(glm::vec3(63.400f, 0.0f, -112.0f));

			//camera->Add<SimpleCameraControl>();

			// This is now handled by scene itself!
			//Camera::Sptr cam = camera->Add<Camera>();
			// Make sure that the camera is set as the scene's main camera!
			//scene->MainCamera = cam;
		}

		// Set up all our sample objects

		GameObject::Sptr car = scene->CreateGameObject("Car");
		{
			// Set position in the scene
			car->SetPostion(glm::vec3(-2.7f, -15.0f, 0.0f));
			car->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			car->SetScale(glm::vec3(1.5f, 1.5f, 1.5f));

			car->Add<CarBehaviour>();
			car->Add<LightBehaviour>();

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = car->Add<RenderComponent>();
			renderer->SetMesh(carMesh);
			renderer->SetMaterial(carMaterial);
		}

		GameObject::Sptr ground = scene->CreateGameObject("Ground");
		{
			// Set position in the scene
			ground->SetPostion(glm::vec3(0.0f, -1.740f, 0.0f));
			ground->SetRotation(glm::vec3(90.0f, 0.0f, -180.0f));
			ground->SetScale(glm::vec3(1.0f, 1.0f, 1.17f));

			ground->Add<LightBehaviour>();

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = ground->Add<RenderComponent>();
			renderer->SetMesh(groundMesh);
			renderer->SetMaterial(groundMaterial);
		} 

		GameObject::Sptr sidewalk = scene->CreateGameObject("Sidewalk");
		{
			// Set position in the scene
			sidewalk->SetPostion(glm::vec3(0.0f, -1.740f, 0.0f));
			sidewalk->SetRotation(glm::vec3(90.0f, 0.0f, -180.0f));
			sidewalk->SetScale(glm::vec3(1.0f, 1.0f, 1.17f));

			sidewalk->Add<LightBehaviour>();

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = sidewalk->Add<RenderComponent>();
			renderer->SetMesh(sidewalkMesh);
			renderer->SetMaterial(autumnMaterial);
		}

		GameObject::Sptr decoration = scene->CreateGameObject("Decoration"); 
		{
			// Set position in the scene
			decoration->SetPostion(glm::vec3(1.22f, -3.57f, 0.0f));
			decoration->SetRotation(glm::vec3(90.0f, 0.0f, -180.0f));
			decoration->SetScale(glm::vec3(0.75f, 0.75f, 0.75f));

			decoration->Add<LightBehaviour>();

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = decoration->Add<RenderComponent>();
			renderer->SetMesh(decorationMesh);
			renderer->SetMaterial(autumnMaterial);
		}

		GameObject::Sptr road = scene->CreateGameObject("Road");
		{
			// Set position in the scene
			road->SetPostion(glm::vec3(-1.85f, -12.350f, -0.20f));
			road->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
			road->SetScale(glm::vec3(21.5f, 28.310f, 21.5f));

			road->Add<LightBehaviour>();

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = road->Add<RenderComponent>();
			renderer->SetMesh(planeMesh);
			renderer->SetMaterial(roadMaterial);

		}

		GameObject::Sptr group = scene->CreateGameObject("Group");
		group->AddChild(car);
		group->AddChild(road);
		group->AddChild(ground);
		group->AddChild(sidewalk);
		group->AddChild(decoration);
		
		

		/////////////////////////// UI //////////////////////////////
		/*
		GameObject::Sptr canvas = scene->CreateGameObject("UI Canvas");
		{
			RectTransform::Sptr transform = canvas->Add<RectTransform>();
			transform->SetMin({ 16, 16 });
			transform->SetMax({ 256, 256 });

			GuiPanel::Sptr canPanel = canvas->Add<GuiPanel>();


			GameObject::Sptr subPanel = scene->CreateGameObject("Sub Item");
			{
				RectTransform::Sptr transform = subPanel->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 128, 128 });

				GuiPanel::Sptr panel = subPanel->Add<GuiPanel>();
				panel->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

				panel->SetTexture(ResourceManager::CreateAsset<Texture2D>("textures/upArrow.png"));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 16.0f);
				font->Bake();

				GuiText::Sptr text = subPanel->Add<GuiText>();
				text->SetText("Hello world!");
				text->SetFont(font);

				monkey1->Get<JumpBehaviour>()->Panel = text;
			}

			canvas->AddChild(subPanel);
		}
		*/

		GuiBatcher::SetDefaultTexture(ResourceManager::CreateAsset<Texture2D>("textures/ui-sprite.png"));
		GuiBatcher::SetDefaultBorderRadius(8);

		// Save the asset manifest for all the resources we just loaded
		ResourceManager::SaveManifest("scene-manifest.json");
		// Save the scene to a JSON file
		scene->Save("scene.json");

		// Send the scene to the application
		app.LoadScene(scene);
	}

	int main(); {

	}
}
