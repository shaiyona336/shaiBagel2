#include "worms.h"
#include <iostream>

namespace worms {

// CollisionSystem implementation
bagel::Mask CollisionSystem::getMask() {
    bagel::MaskBuilder builder;
    return builder
        .set<Position>()
        .set<Physics>()
        .build();
}

void CollisionSystem::update(float deltaTime) {
    bagel::Mask mask = getMask();
    bagel::Mask healthMask = bagel::MaskBuilder().set<Health>().build();

    // Iterate through all entities that have Position and Physics components
    for (bagel::ent_type entity = {0}; entity.id <= bagel::World::maxId().id; ++entity.id) {
        if (bagel::World::mask(entity).test(mask)) {
            // Empty loop body as per the assignment requirements
            // In a real implementation, we would check for collisions here
        }
    }
}

// PhysicsSystem implementation
bagel::Mask PhysicsSystem::getMask() {
    bagel::MaskBuilder builder;
    return builder
        .set<Position>()
        .set<Physics>()
        .build();
}

void PhysicsSystem::update(float deltaTime) {
    bagel::Mask mask = getMask();

    // Iterate through all entities that have Position and Physics components
    for (bagel::ent_type entity = {0}; entity.id <= bagel::World::maxId().id; ++entity.id) {
        if (bagel::World::mask(entity).test(mask)) {
            // Empty loop body as per the assignment requirements
            // In a real implementation, we would update positions based on physics here
        }
    }
}

// WeaponSystem implementation
bagel::Mask WeaponSystem::getMask() {
    bagel::MaskBuilder builder;
    return builder
        .set<Weapon>()
        .set<Input>()
        .build();
}

void WeaponSystem::update(float deltaTime) {
    bagel::Mask mask = getMask();

    // Iterate through all entities that have Weapon and Input components
    for (bagel::ent_type entity = {0}; entity.id <= bagel::World::maxId().id; ++entity.id) {
        if (bagel::World::mask(entity).test(mask)) {
            // Empty loop body as per the assignment requirements
            // In a real implementation, we would handle weapon selection and firing here
        }
    }
}

// ExplosionSystem implementation
bagel::Mask ExplosionSystem::getMask() {
    bagel::MaskBuilder builder;
    return builder
        .set<Explosion>()
        .set<Position>()
        .build();
}

void ExplosionSystem::update(float deltaTime) {
    bagel::Mask mask = getMask();

    // Iterate through all entities that have Explosion and Position components
    for (bagel::ent_type entity = {0}; entity.id <= bagel::World::maxId().id; ++entity.id) {
        if (bagel::World::mask(entity).test(mask)) {
            // Empty loop body as per the assignment requirements
            // In a real implementation, we would animate explosions and apply damage here
        }
    }
}

// InputSystem implementation
bagel::Mask InputSystem::getMask() {
    bagel::MaskBuilder builder;
    return builder
        .set<Input>()
        .set<Physics>()
        .build();
}

void InputSystem::update(float deltaTime) {
    bagel::Mask mask = getMask();

    // Iterate through all entities that have Input and Physics components
    for (bagel::ent_type entity = {0}; entity.id <= bagel::World::maxId().id; ++entity.id) {
        if (bagel::World::mask(entity).test(mask)) {
            // Empty loop body as per the assignment requirements
            // In a real implementation, we would process player input here
        }
    }
}

// HealthSystem implementation
bagel::Mask HealthSystem::getMask() {
    bagel::MaskBuilder builder;
    return builder
        .set<Health>()
        .build();
}

void HealthSystem::update(float deltaTime) {
    bagel::Mask mask = getMask();

    // Iterate through all entities that have a Health component
    for (bagel::ent_type entity = {0}; entity.id <= bagel::World::maxId().id; ++entity.id) {
        if (bagel::World::mask(entity).test(mask)) {
            // Empty loop body as per the assignment requirements
            // In a real implementation, we would check for death conditions here
        }
    }
}

// Entity creation functions
bagel::Entity createPlayer(float x, float y, bool isActive) {
    bagel::Entity entity = bagel::Entity::create();

    // Add required components
    Position position{x, y};
    Health health{};
    Physics physics{};
    Input input{};

    // Set default physics properties for a player
    physics.weight = 1.0f;
    physics.isAffectedByGravity = true;

    // Set active player status if needed
    if (isActive) {
        input.isAiming = true;
    }

    // Add components to the entity
    entity.addAll(position, health, physics, input);

    // Weapon is optional and can be added separately if needed
    if (isActive) {
        Weapon weapon{Weapon::Type::BAZOOKA, entity.entity(), 10};
        entity.add(weapon);
    }

    return entity;
}

bagel::Entity createProjectile(float x, float y, float velX, float velY, Weapon::Type weaponType, bagel::ent_type owner) {
    bagel::Entity entity = bagel::Entity::create();

    // Add required components
    Position position{x, y};
    Physics physics{};

    // Set physics properties for the projectile
    physics.velX = velX;
    physics.velY = velY;
    physics.isAffectedByGravity = true;
    physics.weight = 0.5f;  // Projectiles are lighter than players

    // Add components to the entity
    entity.addAll(position, physics);

    return entity;
}

bagel::Entity createTerrain(float x, float y) {
    bagel::Entity entity = bagel::Entity::create();

    // Terrain only needs a position component
    Position position{x, y};
    entity.add(position);

    return entity;
}

bagel::Entity createCollectable(float x, float y, Collectable::Type type, int value) {
    bagel::Entity entity = bagel::Entity::create();

    // Add required components
    Position position{x, y};
    Collectable collectable{type, value};

    // Add components to the entity
    entity.addAll(position, collectable);

    return entity;
}

bagel::Entity createExplosion(float x, float y, Explosion::Type type) {
    bagel::Entity entity = bagel::Entity::create();

    // Add required components
    Position position{x, y};
    Explosion explosion{type};
    Physics physics{};

    // Explosions don't move but need physics for collision detection
    physics.isAffectedByGravity = false;
    physics.weight = 0.0f;

    // Set explosion properties based on type
    switch (type) {
        case Explosion::Type::SMALL:
            explosion.radius = 30.0f;
            break;
        case Explosion::Type::MEDIUM:
            explosion.radius = 50.0f;
            break;
        case Explosion::Type::LARGE:
            explosion.radius = 80.0f;
            break;
    }

    // Add components to the entity
    entity.addAll(position, explosion, physics);

    return entity;
}

} // namespace worms