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

	void Update(SDL_Renderer& renderer, Registry& registry, SDL_FRect& camera)
	{
		// Start ImGui frame
		ImGui_ImplSDLRenderer3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		// Form widgets
		if (ImGui::Begin("Spawn Enemy"))
		{
			///////////////// Transform component
			static int enemyXPos = 0;
			static int enemyYPos = 0;
			static float enemyXScale = 1.0f;
			static float enemyYScale = 1.0f;
			static float enemyRotationDegrees = 0.0f;

			if (ImGui::CollapsingHeader("Transform Component", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Indent();

				// Position
				ImGui::InputInt("X Position", &enemyXPos);
				ImGui::InputInt("Y Position", &enemyYPos);

				// Scale
				ImGui::InputFloat("X Scale", &enemyXScale);
				ImGui::InputFloat("Y Scale", &enemyYScale);

				enemyXScale = std::max(0.0f, enemyXScale);
				enemyYScale = std::max(0.0f, enemyYScale);

				// Rotation
				ImGui::SliderFloat("Rotation", &enemyRotationDegrees, 0.0f, 360.0f, "%.1f deg");

				ImGui::Unindent();
			}

			///////////////// Rigidbody component
			static int enemyXVelocity = 0;
			static int enemyYVelocity = 0;

			if (ImGui::CollapsingHeader("RigidBody Component", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Indent();

				// Velocity
				ImGui::InputInt("X Velocity", &enemyXVelocity);
				ImGui::InputInt("Y Velocity", &enemyYVelocity);

				ImGui::Unindent();
			}

			///////////////// Sprite component
			const std::vector<std::string>& textureIds = AssetStore::Get().GetTextureIds();
			static int selectedId = 0; 

			static int spriteZIndex = 0;

			static bool isAutoFullTexture = true;

			static int spriteSourceRectWidth = 0;
			static int spriteSourceRectHeight = 0;

			static int spriteSourceRectX = 0;
			static int spriteSourceRectY = 0;

			if (ImGui::CollapsingHeader("Sprite Component", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Indent();

				// Ensure the index stays safe if elements are deleted elsewhere in your code
				if (selectedId >= textureIds.size()) 
				{
					selectedId = textureIds.empty() ? -1 : 0;
				}

				// Get the preview text safely
				const char* previewValue = (selectedId >= 0 && selectedId < textureIds.size())
					? textureIds[selectedId].c_str()
					: "Select an item...";

				if (ImGui::BeginCombo("Texture Ids List", previewValue)) 
				{
					for (int i = 0; i < textureIds.size(); ++i) 
					{
						const bool isSelected = (selectedId == i);

						// Pass the raw C-string using .c_str()
						if (ImGui::Selectable(textureIds[i].c_str(), isSelected))
						{
							selectedId = i;
						}

						// Set the initial focus onto the currently selected item
						if (isSelected) 
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				// Set z-index for sprite
				ImGui::InputInt("Z index", &spriteZIndex);
				spriteZIndex = std::max(0, spriteZIndex);

				// Enable or disable auto sprite size calculation. It will take full sprite size
				ImGui::Checkbox("Enable Auto Full Sprite Size Texture", &isAutoFullTexture);

				if (!isAutoFullTexture)
				{
					// Sprite source rectangle size
					ImGui::InputInt("Sprite Source Rect Width", &spriteSourceRectWidth);
					ImGui::InputInt("Sprite Source Rect Height", &spriteSourceRectHeight);
					spriteSourceRectWidth = std::max(0, spriteSourceRectWidth);
					spriteSourceRectHeight = std::max(0, spriteSourceRectHeight);

					// Sprite source rectangle position
					ImGui::InputInt("Sprite Source Rect X", &spriteSourceRectX);
					ImGui::InputInt("Sprite Source Rect Y", &spriteSourceRectY);
					spriteSourceRectX = std::max(0, spriteSourceRectX);
					spriteSourceRectY = std::max(0, spriteSourceRectY);
				}

				ImGui::Unindent();
			}

			///////////////// Projectile Emit Component
			static float projectileEmitAngle = 0.0f;
			static float projectilXDirection = 0.0f;
			static float projectileYDirection = 0.0f;

			static float projectileSpeed = 1.0f;
			static float projectileEmitFrequencyInSeconds = 1.0f;
			static float projectileDurationInSeconds = 1.0f;

			static int projectileHitPercentDamage = 10;

			if (ImGui::CollapsingHeader("Projectile Emit Component", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Indent();

				// Set intitial direction for projectil emit
				float initialXDirection = 0.0f;
				float initialYDirection = -1.0f;

				// Set rotation in degrees
				ImGui::SliderFloat("Projectile Emit Direction", &projectileEmitAngle, 0.0f, 360.0f, "%.1f deg");
				
				// Reverse projectile emit angle, so it will go clock-wise
				float angle = projectileEmitAngle * (3.14f / 180.0f);

				projectilXDirection = initialXDirection * cos(angle) - initialYDirection * sin(angle);
				projectileYDirection = initialXDirection * sin(angle) + initialYDirection * cos(angle);

				// Set Projectile Speed
				ImGui::InputFloat("Projectile Speed", &projectileSpeed);
				projectileSpeed = std::max(0.0f, projectileSpeed);

				// Set Emit Frequency in seconds
				ImGui::InputFloat("Projectile Emit Frequency In Seconds", &projectileEmitFrequencyInSeconds);
				projectileEmitFrequencyInSeconds = std::max(0.0f, projectileEmitFrequencyInSeconds);

				// Set Projectile Duration
				ImGui::InputFloat("Projectile Duration In Seconds", &projectileDurationInSeconds);
				projectileDurationInSeconds = std::max(0.0f, projectileDurationInSeconds);

				// Set Projectile Damage
				ImGui::InputInt("Projectile Damage", &projectileHitPercentDamage);
				projectileHitPercentDamage = std::max(0, projectileHitPercentDamage);

				ImGui::Unindent();
			}

			///////////////// Health and HealthBar Components
			static int healthPercentage = 100;

			static float healthBarTextOffsetX = 70.0f;
			static float healthBarTextOffsetY = 0.0f;

			static float healthBarSizeX = 30.0f;
			static float healthBarSizeY = 10.0f;

			static float healthBarOffsetX = 70.0f;
			static float healthBarOffsetY = 20.0f;

			if (ImGui::CollapsingHeader("Health and Health Bar Components", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Indent();

				// Set Health 
				ImGui::InputInt("Health", &healthPercentage);
				healthPercentage = std::max(0, healthPercentage);

				// Set Health Text Offset
				ImGui::InputFloat("Health Bar Text Offset X", &healthBarTextOffsetX);
				ImGui::InputFloat("Health Bar Text Offset Y", &healthBarTextOffsetY);

				// Set Health Bar Size
				ImGui::InputFloat("Health Bar Size X", &healthBarSizeX);
				healthBarSizeX = std::max(0.0f, healthBarSizeX);
				ImGui::InputFloat("Health Bar Size Y", &healthBarSizeY);
				healthBarSizeY = std::max(0.0f, healthBarSizeY);

				// Set Health Bar Offset
				ImGui::InputFloat("Health Bar Offset X", &healthBarOffsetX);
				ImGui::InputFloat("Health Bar Offset Y", &healthBarOffsetY);

				ImGui::Unindent();
			}

			
			// Button to create an enemy
			if (ImGui::Button("Create New Enemy"))
			{
				Entity enemy = registry.CreateEntity();
				enemy.Group("enemies");
				enemy.AddComponent<TransformComponent>(glm::vec2(enemyXPos, enemyYPos), glm::vec2(enemyXScale, enemyYScale), enemyRotationDegrees);
				enemy.AddComponent<RigidBodyComponent>(glm::vec2(enemyXVelocity, enemyYVelocity));
				
				if (isAutoFullTexture)
				{
					enemy.AddComponent<SpriteComponent>(AssetStore::Get(), textureIds[selectedId], spriteZIndex);
				}
				else
				{
					enemy.AddComponent<SpriteComponent>(
						textureIds[selectedId],
						spriteSourceRectWidth,
						spriteSourceRectHeight,
						spriteZIndex,
						spriteSourceRectX,
						spriteSourceRectY
					);
				}

				enemy.AddComponent<BoxColliderComponent>(enemy.GetComponent<SpriteComponent>().width, enemy.GetComponent<SpriteComponent>().height);
				enemy.AddComponent<ProjectileEmitterComponent>(
				glm::vec2(projectilXDirection * projectileSpeed, projectileYDirection * projectileSpeed),
					projectileEmitFrequencyInSeconds * 1000.0f,
					projectileDurationInSeconds * 1000.0f,
					projectileHitPercentDamage,
				false,
				"bullet-image");

				enemy.AddComponent<HealthComponent>(healthPercentage);
				enemy.AddComponent<HealthBarComponent>("pico-font-10", glm::vec2(healthBarTextOffsetX, healthBarTextOffsetY), glm::vec2(healthBarSizeX, healthBarSizeY), glm::vec2(healthBarOffsetX, healthBarOffsetY));
			}
		}
		ImGui::End();

		// Display a small overlay window to display the map position using the mouse
		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiChildFlags_AlwaysAutoResize;
		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always, ImVec2(0, 0));
		ImGui::SetNextWindowBgAlpha(0.9f);
		if (ImGui::Begin("Map Coordinates", NULL, windowFlags))
		{
			ImGui::Text(
				"Map Coordinates (x=%.1f, y=%.1f)",
				ImGui::GetIO().MousePos.x + camera.x,
				ImGui::GetIO().MousePos.y + camera.y
			);
		}
		ImGui::End();

		// Render ImGui
		ImGui::Render();
		ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), &renderer);
	}
};