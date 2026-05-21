template<typename TComponent>
void Registry::EnsureComponentRegistered()
{
	static bool registered = false;
	if (registered)
	{
		return;
	}

	int id = Component<TComponent>::GetId();
	_componentColumnCreators[id] = []() {
		return std::make_unique<Column<TComponent>>();
		};
	registered = true;

	LOG_INFO("ECS system registered new component for column creation: %i id", id);
}

template<typename TComponent, typename ...TArgs>
void Registry::AddComponent(Entity entity, TArgs&& ...args)
{
	EnsureComponentRegistered<TComponent>();

	const int entityId = entity.GetId();
	const int componentId = Component<TComponent>::GetId();

	Location& oldLocation = _locations[entity];
	Signature oldSignature = oldLocation.archetype ? oldLocation.archetype->signature : Signature();

	// Build new signature
	Signature newSignature = oldSignature;
	newSignature.set(componentId);

	// Find or create target archetype
	std::shared_ptr<Archetype> targetArchetype = CreateOrGetArchetype(newSignature);

	// Create new row in target archetype
	uint32_t newRow = targetArchetype->entities.size();
	targetArchetype->entities.push_back(entity);

	Column<TComponent>* column = static_cast<Column<TComponent>*>(targetArchetype->columns[componentId].get());
	column->Push(TComponent(std::forward<TArgs>(args)...));

	// If entity already exists - move it
	if (oldLocation.archetype)
	{
		MoveEntity(oldLocation, targetArchetype, newRow);
	}
	else
	{
		oldLocation.archetype = targetArchetype;
		oldLocation.row = newRow;
	}

	// Update entity component signature
	_entityComponentSignatures[entityId] = newSignature;

	LOG_INFO("Component id = %d was added to entity id %d", componentId, entityId);
}

template<typename TComponent>
void Registry::RemoveComponent(Entity entity)
{
	const int entityId = entity.GetId();
	const int componentId = Component<TComponent>::GetId();

	Location& oldLocation = _locations[entity];
	Signature oldSignature = oldLocation.archetype ? oldLocation.archetype->signature : Signature();

	// Build new signature
	Signature newSignature = oldSignature;
	// Remove bit for old component
	newSignature.set(componentId, false);

	// Find or create target archetype
	std::shared_ptr<Archetype> targetArchetype = CreateOrGetArchetype(newSignature);

	// Create new row in target archetype
	uint32_t newRow = targetArchetype->entities.size();
	targetArchetype->entities.push_back(entity);

	// If entity already exists - move it
	if (oldLocation.archetype)
	{
		MoveEntity(oldLocation, targetArchetype, newRow);
	}
	else
	{
		oldLocation.archetype = targetArchetype;
		oldLocation.row = newRow;
	}

	// Set this component signature for that entity to false
	_entityComponentSignatures[entityId].set(componentId, false);

	LOG_INFO("Component id = %d was removed from entity id %d", componentId, entityId);
}

template<typename TComponent>
bool Registry::HasComponent(Entity entity) const
{
	const int componentId = Component<TComponent>::GetId();
	const int entityId = entity.GetId();

	return _entityComponentSignatures[entityId].test(componentId);
}

template<typename TComponent>
TComponent& Registry::GetComponent(Entity entity) const
{
	const int componentId = Component<TComponent>::GetId();

	// Get location and component index
	const Location& location = _locations.at(entity);
	int componentIndex = location.row;

	// Get according column type
	Column<TComponent>* column = static_cast<Column<TComponent>*>(location.archetype->columns[componentId].get());

	return column->Get(componentIndex);
}

template<typename TSystem, typename ...TArgs>
void Registry::AddSystem(TArgs && ...args)
{
	std::shared_ptr<TSystem> newSystem = std::make_shared<TSystem>(std::forward(args)...);
	_systems.insert(std::make_pair(std::type_index(typeid(TSystem)), newSystem));
}

template<typename TSystem>
void Registry::RemoveSystem()
{
	std::type_index system = _systems.find(std::type_index(typeid(TSystem)));
	_systems.erase(system);
}

template<typename TSystem>
bool Registry::HasSystem() const
{
	return _systems.find(std::type_index(typeid(TSystem))) != _systems.end();
}

template<typename TSystem>
TSystem& Registry::GetSystem() const
{
	auto system = _systems.find(std::type_index(typeid(TSystem)));
	return *(std::static_pointer_cast<TSystem>(system->second));
}

template<typename TComponent, typename ...TArgs>
void Entity::AddComponent(TArgs && ...args)
{
	_registry->AddComponent<TComponent>(*this, std::forward<TArgs>(args)...);
}

template<typename TComponent>
void Entity::RemoveComponent()
{
	_registry->RemoveComponent<TComponent>(*this);
}

template<typename TComponent>
bool Entity::HasComponent() const
{
	return _registry->HasComponent<TComponent>(*this);
}

template<typename TComponent>
TComponent& Entity::GetComponent() const
{
	return _registry->GetComponent<TComponent>(*this);
}