#pragma once

struct HealthComponent
{
	int healthPercentage;
	int maxHealthPercentage;

	HealthComponent(int healthPercentage = 100, int maxHealthPercentage = 100) : healthPercentage(healthPercentage), maxHealthPercentage(maxHealthPercentage)
	{
		maxHealthPercentage = std::max(healthPercentage, maxHealthPercentage);
	}
};