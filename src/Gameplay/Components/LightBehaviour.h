#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Components/RenderComponent.h"
#include "Graphics/Textures/Texture3D.h"

#pragma once
class LightBehaviour : public Gameplay::IComponent
{
public:
	typedef std::shared_ptr<LightBehaviour> Sptr;

	std::weak_ptr<Gameplay::IComponent> Panel;

	LightBehaviour();
	virtual ~LightBehaviour();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(LightBehaviour);
	virtual nlohmann::json ToJson() const override;
	static LightBehaviour::Sptr FromJson(const nlohmann::json& blob);

protected:
	float light;
	RenderComponent::Sptr _renderer;
	bool lightOn = true, ambientOn, warpDiffuseOn, warpSpecOn;
	//Texture3D::Sptr lutWarm;
	//Texture3D::Sptr lutCool;
	//Texture3D::Sptr lutFilm;
};


