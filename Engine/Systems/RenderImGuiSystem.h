#pragma once

#include "ECS/ECS.h"

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>

class RenderImGuiSystem : public System
{
public:
	RenderImGuiSystem()
	{
		// No components required. Renders ImGui widgets
	}

	void Update(SDL_Renderer& renderer)
	{
		// Start ImGui frame
		ImGui_ImplSDLRenderer3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		// Form widgets
		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize;
		if (ImGui::Begin("Spawn Enemies", NULL, windowFlags))
		{
			ImGui::Text("This is window should be with no decoration and with auto resize");
		}
		ImGui::End();

		// Render ImGui
		ImGui::Render();
		ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), &renderer);
	}
};