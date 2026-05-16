#pragma once

#include "ECS/ECS.h"

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>

#include "Components/TransformComponent.h"
#include "Components/RigidBodyComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/BoxColliderComponent.h"
#include "Components/ProjectileEmitterComponent.h"
#include "Components/HealthComponent.h"
#include "Components/HealthBarComponent.h"

class RenderImGuiSystem : public System
{
public:
	RenderImGuiSystem()
	{
		// No components required. Renders ImGui widgets
	}

	void Update(SDL_Renderer& renderer, Registry& registry)
	{
		// Start ImGui frame
		ImGui_ImplSDLRenderer3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		// Form widgets
		if (ImGui::Begin("Spawn Enemy"))
		{
			// Input for the enemy X position
			static int enemyXPos = 0;
			static int enemyYPos = 0;
			ImGui::InputInt("X position", &enemyXPos);
			ImGui::InputInt("Y position", &enemyYPos);

			if (ImGui::Button("Create new enemy"))
			{
				Entity enemy = registry.CreateEntity();
				enemy.Group("enemies");
				enemy.AddComponent<TransformComponent>(glm::vec2(enemyXPos, enemyYPos), glm::vec2(1.0, 1.0), 0.0);
				enemy.AddComponent<RigidBodyComponent>(glm::vec2(0, 0));
				enemy.AddComponent<SpriteComponent>(AssetStore::Get(), "truck-image", 1);
				enemy.AddComponent<BoxColliderComponent>(enemy.GetComponent<SpriteComponent>().width, enemy.GetComponent<SpriteComponent>().height);
				enemy.AddComponent<ProjectileEmitterComponent>(glm::vec2(0.0, -100.0), 1000, 5000, 10, false, "bullet-image");
				enemy.AddComponent<HealthComponent>(100);
				enemy.AddComponent<HealthBarComponent>("pico-font-10", glm::vec2(70, 0), glm::vec2(30, 10), glm::vec2(70, 20));
			}
		}
		ImGui::End();

		// Render ImGui
		ImGui::Render();
		ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), &renderer);
	}
};