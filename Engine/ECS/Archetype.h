#pragma once

#include "Types.h"
#include "Entity.h"
#include "Column.h"

/////////////////////////////////////////////////////////////////////
// Archetype
/////////////////////////////////////////////////////////////////////
// An archetype represents a unique combination of component types.
//
// Example:
//
// Archetype A:
// [Transform, Velocity]
//
// Archetype B:
// [Transform, Sprite]
//
// Archetype C:
// [Transform, Velocity, Health]
//
// Every entity belongs to exactly ONE archetype at a time,
// depending on which components it currently owns.
//
// ------------------------------------------------------------------
// Why archetypes exist
// ------------------------------------------------------------------
//
// Archetypes store entities that have the same component layout
// together in tightly packed contiguous arrays.
//
// This allows systems to iterate cache-friendly blocks of memory:
//
// Instead of:
//
//   Position of entity 1 somewhere in memory
//   Position of entity 2 somewhere else
//   Position of entity 3 somewhere else
//
// we get:
//
//   [P1][P2][P3][P4][P5]
//
// packed together sequentially.
//
// This drastically improves:
// - CPU cache locality
// - iteration speed
// - SIMD/vectorization opportunities
// - large-scale ECS performance
// 
// ------------------------------------------------------------------
// Archetype layout
// ------------------------------------------------------------------
//
// Each archetype acts similarly to a database table:
//
// -----------------------------------------------------
// | Entity | Transform | Velocity | Health |
// -----------------------------------------------------
// |   1    |    ...    |    ...   |   ...  |
// |   2    |    ...    |    ...   |   ...  |
// -----------------------------------------------------
//
// Each component type is stored in its own dense column.
/////////////////////////////////////////////////////////////////////
struct Archetype
{
	// Component set
	Signature signature;

	// Rows
	std::vector<Entity> entities;

	// Component array
	// [componentId]->[componentArray]
	std::vector<std::unique_ptr<IColumn>> columns;
};

// Location of element in archetype
struct Location
{
    std::shared_ptr<Archetype> archetype = nullptr;
    uint32_t row = 0;
};