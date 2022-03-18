#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Components/RenderComponent.h"
#include "Graphics/Textures/Texture3D.h"

#pragma once
class CarBehaviour : public Gameplay::IComponent
{
public:
	typedef std::shared_ptr<CarBehaviour> Sptr;

	std::weak_ptr<Gameplay::IComponent> Panel;

	CarBehaviour();
	virtual ~CarBehaviour();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(CarBehaviour);
	virtual nlohmann::json ToJson() const override;
	static CarBehaviour::Sptr FromJson(const nlohmann::json& blob);

protected:
	float car;
	RenderComponent::Sptr _renderer;
	float yPos;
	bool lutWarmOn, lutCoolOn, lutFilmOn;
	Texture3D::Sptr lutDefault;
	Texture3D::Sptr lutWarm;
	Texture3D::Sptr lutCool;
	Texture3D::Sptr lutFilm;
};

