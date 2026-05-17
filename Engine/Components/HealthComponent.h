#pragma once

struct HealthComponent
{
	int healthPercentage;
	int maxHealthPercentage;

	HealthComponent(int healthPercentage = 100) : healthPercentage(healthPercentage), maxHealthPercentage(healthPercentage){}
};