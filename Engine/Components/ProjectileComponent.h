#pragma once

struct ProjectileComponent {
    bool isFriendly;
    int hitPercentDamage;

    ProjectileComponent(bool isFriendly = false, int hitPercentDamage = 0) 
       : isFriendly(isFriendly), hitPercentDamage(hitPercentDamage)
    {}
};