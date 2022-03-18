#include "Gameplay/Components/LightBehaviour.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"
#include "Gameplay/InputEngine.h"

void LightBehaviour::Awake()
{
	_renderer = GetComponent<RenderComponent>();



	warpDiffuseOn = false;
	warpSpecOn = false;
}

void LightBehaviour::RenderImGui() {
	//LABEL_LEFT(ImGui::DragFloat, "Light", &light, 13.6f);
}

nlohmann::json LightBehaviour::ToJson() const {
	return {
		{ "light", light }
	};
}

LightBehaviour::LightBehaviour() :
	IComponent(),
	_renderer(nullptr),
	light(10.0f)
{ }

LightBehaviour::~LightBehaviour() = default;

LightBehaviour::Sptr LightBehaviour::FromJson(const nlohmann::json & blob) {
	LightBehaviour::Sptr result = std::make_shared<LightBehaviour>();
	result->light = blob["light"];
	return result;
}

void LightBehaviour::Update(float deltaTime)
{
	///////// TOGGLES ////////////

	if (InputEngine::GetKeyState(GLFW_KEY_1) == ButtonState::Pressed) { //Toggle Lights
		lightOn = !lightOn;
	}

	if (lightOn) { //set lights to default ranges from DefaultSceneLayer
		GetGameObject()->GetScene()->Lights[0].Range = 200.0f;
		GetGameObject()->GetScene()->SetupShaderAndLights();
	}
	else { //set light ranges to 0
		GetGameObject()->GetScene()->Lights[0].Range = 0.0f;
		GetGameObject()->GetScene()->SetupShaderAndLights();
	}

	if (InputEngine::GetKeyState(GLFW_KEY_2) == ButtonState::Pressed) { //Ambient Lighting Only
		_renderer->GetMaterial()->Set("u_Material.Toggle", 1);
	}

	if (InputEngine::GetKeyState(GLFW_KEY_3) == ButtonState::Pressed) { //Specular Lighting Only
		_renderer->GetMaterial()->Set("u_Material.Toggle", 2);
	}

	if (InputEngine::GetKeyState(GLFW_KEY_4) == ButtonState::Pressed) { //Ambient + Specular
		_renderer->GetMaterial()->Set("u_Material.Toggle", 3);
	}

	if (InputEngine::GetKeyState(GLFW_KEY_5) == ButtonState::Pressed) { //Ambient + Specular CUSTOM SHADER
		_renderer->GetMaterial()->Set("u_Material.Toggle", 4);
	}

	if (InputEngine::GetKeyState(GLFW_KEY_6) == ButtonState::Pressed) { //Diffuse warp/ramp
		_renderer->GetMaterial()->Set("u_Material.Toggle", 5);
		warpDiffuseOn = !warpDiffuseOn;

		if (warpDiffuseOn)
			_renderer->GetMaterial()->Set("u_Material.ToggleOn", true);
		else
			_renderer->GetMaterial()->Set("u_Material.ToggleOn", false);
	}

	if (InputEngine::GetKeyState(GLFW_KEY_7) == ButtonState::Pressed) { //Spec warp/ramp
		_renderer->GetMaterial()->Set("u_Material.Toggle", 6);
		warpSpecOn = !warpSpecOn;

		if (warpSpecOn)
			_renderer->GetMaterial()->Set("u_Material.ToggleOn", true);
		else
			_renderer->GetMaterial()->Set("u_Material.ToggleOn", false);
	}

	//make diffuse warp false when another button is pressed
	if (InputEngine::GetKeyState(GLFW_KEY_0) == ButtonState::Pressed ||
		InputEngine::GetKeyState(GLFW_KEY_1) == ButtonState::Pressed ||
		InputEngine::GetKeyState(GLFW_KEY_2) == ButtonState::Pressed ||
		InputEngine::GetKeyState(GLFW_KEY_3) == ButtonState::Pressed ||
		InputEngine::GetKeyState(GLFW_KEY_4) == ButtonState::Pressed ||
		InputEngine::GetKeyState(GLFW_KEY_5) == ButtonState::Pressed ||
		InputEngine::GetKeyState(GLFW_KEY_7) == ButtonState::Pressed ||
		InputEngine::GetKeyState(GLFW_KEY_8) == ButtonState::Pressed ||
		InputEngine::GetKeyState(GLFW_KEY_9) == ButtonState::Pressed) {
		warpDiffuseOn = false;
	}

	//make spec warp false when another button is pressed
	if (InputEngine::GetKeyState(GLFW_KEY_0) == ButtonState::Pressed ||
		InputEngine::GetKeyState(GLFW_KEY_1) == ButtonState::Pressed ||
		InputEngine::GetKeyState(GLFW_KEY_2) == ButtonState::Pressed ||
		InputEngine::GetKeyState(GLFW_KEY_3) == ButtonState::Pressed ||
		InputEngine::GetKeyState(GLFW_KEY_4) == ButtonState::Pressed ||
		InputEngine::GetKeyState(GLFW_KEY_5) == ButtonState::Pressed ||
		InputEngine::GetKeyState(GLFW_KEY_6) == ButtonState::Pressed ||
		InputEngine::GetKeyState(GLFW_KEY_8) == ButtonState::Pressed ||
		InputEngine::GetKeyState(GLFW_KEY_9) == ButtonState::Pressed) {
		warpSpecOn = false;
	}

}

