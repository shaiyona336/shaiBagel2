#include "worms.h"
#include <iostream>
constexpr float BAZOOKA_PROJECTILE_WEIGHT = 0.5f;
constexpr float GRENADE_PROJECTILE_WEIGHT = 0.7f;
constexpr float SHOTGUN_PROJECTILE_WEIGHT = 0.2f;


namespace worms {

//systems

bagel::Mask CollisionSystem::getMask() {
    bagel::MaskBuilder builder;
    return builder.set<Position>().build();
}

void CollisionSystem::update(float deltaTime) {
    bagel::Mask mask = getMask();

    for (bagel::ent_type entity = {0}; entity.id <= bagel::World::maxId().id; ++entity.id) {
        if (bagel::World::mask(entity).test(mask)) { }
    }
}


bagel::Mask PhysicsSystem::getMask() {
    bagel::MaskBuilder builder;
    return builder.set<Position>().set<Physics>().build();
}


void PhysicsSystem::update(float deltaTime) {
    bagel::Mask mask = getMask();

    for (bagel::ent_type entity = {0}; entity.id <= bagel::World::maxId().id; ++entity.id) {
        if (bagel::World::mask(entity).test(mask)) { }
    }
}


bagel::Mask WeaponSystem::getMask() {
    bagel::MaskBuilder builder;
    return builder.set<Weapon>().set<Input>().build();
}

void WeaponSystem::update(float deltaTime) {
    bagel::Mask mask = getMask();

    for (bagel::ent_type entity = {0}; entity.id <= bagel::World::maxId().id; ++entity.id) {
        if (bagel::World::mask(entity).test(mask)) { }
    }
}

bagel::Mask ProjectileSystem::getMask() {
    bagel::MaskBuilder builder;
    return builder.set<ProjectileData>().set<Position>().build();
}

void ProjectileSystem::update(float deltaTime) {
    bagel::Mask mask = getMask();

    for (bagel::ent_type entity = {0}; entity.id <= bagel::World::maxId().id; ++entity.id) {
        if (bagel::World::mask(entity).test(mask)) { }
    }
}

bagel::Mask InputSystem::getMask() {
    bagel::MaskBuilder builder;
    return builder.set<Input>().set<Physics>().build(); //possible to change in future to not require physics
}

void InputSystem::update(float deltaTime) {
    bagel::Mask mask = getMask();

    for (bagel::ent_type entity = {0}; entity.id <= bagel::World::maxId().id; ++entity.id) {
        if (bagel::World::mask(entity).test(mask)) { }
    }
}

bagel::Mask HealthSystem::getMask() {
    bagel::MaskBuilder builder;
    return builder.set<Health>().build();
}

void HealthSystem::update(float deltaTime) {
    bagel::Mask mask = getMask();

    for (bagel::ent_type entity = {0}; entity.id <= bagel::World::maxId().id; ++entity.id) {
        if (bagel::World::mask(entity).test(mask)) { }
    }
}

//entities

bagel::Entity createPlayer(float x, float y) {
    bagel::Entity entity = bagel::Entity::create();
    Position position{x, y};
    Health health{};
    Physics physics{};
    Input input{};

    physics.weight = 1.0f;
    physics.isAffectedByGravity = true;
    entity.addAll(position, health, physics, input);

    return entity;
}
//potentially will need to improve, velocity changes based on the direction of the shot, but can adjust speed based on weapon
bagel::Entity createProjectile(float x, float y, float velX, float velY, Weapon::Kind weaponKind) {
    bagel::Entity entity = bagel::Entity::create();
    Position position{x, y};
    Physics physics{};
    ProjectileData projectileData(weaponKind);

    physics.velX = velX;
    physics.velY = velY;
    physics.isAffectedByGravity = true;
    switch (weaponKind) {
        case Weapon::Kind::BAZOOKA:
            physics.weight = BAZOOKA_PROJECTILE_WEIGHT;
            break;
        case Weapon::Kind::GRENADE:
            physics.weight = GRENADE_PROJECTILE_WEIGHT;
            break;
        case Weapon::Kind::SHOTGUN:
            physics.weight = SHOTGUN_PROJECTILE_WEIGHT;
            break;
    }
    entity.addAll(position, physics, projectileData);

    return entity;
}

bagel::Entity createTerrain(float x, float y) {
    bagel::Entity entity = bagel::Entity::create();

    Position position{x, y};
    entity.add(position);

    return entity;
}

bagel::Entity createCollectable(float x, float y, Collectable::Type type, int value) {
    bagel::Entity entity = bagel::Entity::create();

    Position position{x, y};
    Collectable collectable{type, value};
    entity.addAll(position, collectable);

    return entity;
}

}