#pragma once

struct HealthComponent
{
	int healthPercentage;

	HealthComponent(int healthPercentage = 100) : healthPercentage(healthPercentage){}
};