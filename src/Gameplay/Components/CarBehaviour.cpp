#include "Gameplay/Components/CarBehaviour.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"
#include "Gameplay/InputEngine.h"

void CarBehaviour::Awake()
{	
	_renderer = GetComponent<RenderComponent>();
		
	lutDefault = ResourceManager::CreateAsset<Texture3D>("luts/default.CUBE");
	lutWarm = ResourceManager::CreateAsset<Texture3D>("luts/warm.CUBE");
	lutCool = ResourceManager::CreateAsset<Texture3D>("luts/cool.CUBE");
	lutFilm = ResourceManager::CreateAsset<Texture3D>("luts/film.CUBE");

	lutWarmOn = false;
	lutCoolOn = false;
	lutFilmOn = false;

	yPos = 0.02;
}

void CarBehaviour::RenderImGui() {
	//LABEL_LEFT(ImGui::DragFloat, "Impulse", &car, 13.6f);
}

nlohmann::json CarBehaviour::ToJson() const {
	return {
		{ "car", car }
	};
}

CarBehaviour::CarBehaviour() :
	IComponent(),
	_renderer(nullptr),
	car(10.0f)
{ }

CarBehaviour::~CarBehaviour() = default;

CarBehaviour::Sptr CarBehaviour::FromJson(const nlohmann::json & blob) {
	CarBehaviour::Sptr result = std::make_shared<CarBehaviour>();
	result->car = blob["car"]; 
	return result;
}

void CarBehaviour::Update(float deltaTime)
{
	//Car Movement
	GetGameObject()->SetPostion(GetGameObject()->GetPosition() + glm::vec3(0.0f, yPos, 0.0f));
	if (GetGameObject()->GetPosition().y >= 8.4f) {
		GetGameObject()->SetPostion(glm::vec3(GetGameObject()->GetPosition().x, -21.0f, GetGameObject()->GetPosition().z));
	}


	///////// LUT TOGGLES ////////////

	if (InputEngine::GetKeyState(GLFW_KEY_8) == ButtonState::Pressed) { //Warm LUT
		lutWarmOn = !lutWarmOn;

		lutCoolOn = false;
		lutFilmOn = false;

		if (lutWarmOn)
			GetGameObject()->GetScene()->SetColorLUT(lutWarm);
		else
			GetGameObject()->GetScene()->SetColorLUT(lutDefault);
	}

	if (InputEngine::GetKeyState(GLFW_KEY_9) == ButtonState::Pressed) { //Cool LUT
		lutCoolOn = !lutCoolOn;

		lutWarmOn = false;
		lutFilmOn = false;

		if (lutCoolOn)
			GetGameObject()->GetScene()->SetColorLUT(lutCool);
		else
			GetGameObject()->GetScene()->SetColorLUT(lutDefault);
	}

	if (InputEngine::GetKeyState(GLFW_KEY_0) == ButtonState::Pressed) { //Film LUT
		lutFilmOn = !lutFilmOn;

		lutWarmOn = false;
		lutCoolOn = false;
		
		if (lutFilmOn)
			GetGameObject()->GetScene()->SetColorLUT(lutFilm);
		else
			GetGameObject()->GetScene()->SetColorLUT(lutDefault);
	}
}
