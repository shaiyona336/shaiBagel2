/**
 * @file worms.h
 * @brief A simplified Worms-like game implementation using ECS architecture
 *
 * This module implements a simplified version of the Worms game using the Entity Component System
 * architecture. The game features players (worms) that can move, use weapons, and destroy terrain.
 * Physics simulation is applied to all dynamic objects, and collisions are handled accordingly.
 */

#pragma once

#include <vector>
#include <string>
#include "bagel.h"

namespace worms {

/**
 * @brief Component for storing position information
 *
 * This is a "dense" component as specified in the design document.
 * It stores the 2D position of an entity in the game world.
 */
struct Position {
    float x = 0.0f;
    float y = 0.0f;
};

/**
 * @brief Component for storing health information
 *
 * This is a "dense" component as specified in the design document.
 * It stores the health value of a player entity.
 */
struct Health {
    int value = 100; // Default health value
};

/**
 * @brief Component for storing weapon information
 *
 * This is a "sparse" component as specified in the design document.
 * It stores information about a weapon, including its kind, the entity holding it,
 * and the number of ammunition available.
 */
struct Weapon {
    enum class Kind {
        BAZOOKA,
        GRENADE,
        SHOTGUN
    };

    Kind kind = Kind::BAZOOKA;
    int ammo = 10;             // Ammunition count
};

/**
 * @brief Component for storing physics information
 *
 * This is a "dense" component as specified in the design document.
 * It stores physics-related properties like acceleration, velocity, and weight.
 */
struct Physics {
    float accelX = 0.0f;
    float accelY = 0.0f;
    float velX = 0.0f;
    float velY = 0.0f;
    float weight = 1.0f;
    bool isAffectedByGravity = true;
};

/**
 * @brief Component for handling input
 *
 * This is a "sparse" component as specified in the design document.
 * It stores information about player input for controlling entities.
 * Simplified version that focuses on the core movement and aiming.
 */
struct Input {
    float moveDirection = 0.0f; // -1.0 for left, 1.0 for right
    bool jump = false;
    bool fire = false;
    float aimAngle = 0.0f;
    int selectedWeapon = 0; // Index of weapon in player's inventory
};

/**
 * @brief Component for terrain collectable items
 *
 * Represents an item that can be collected by players.
 */
struct Collectable {
    enum class Kind {
        HEALTH,
        AMMO,
        WEAPON
    };

    Kind kind = Kind::HEALTH;
    int value = 25; // Health/ammo amount or weapon ID
};

// Define systems using bagel ECS framework

/**
 * @brief System for handling collisions
 *
 * This system detects and handles collisions between entities,
 * affecting health, position, and potentially creating explosions.
 */
class CollisionSystem {
public:
    static void update(float deltaTime);

private:
    static bagel::Mask getMask();
};

/**
 * @brief System for simulating physics
 *
 * This system updates positions based on physics properties,
 * applying forces, gravity, and other physical constraints.
 */
class PhysicsSystem {
public:
    static void update(float deltaTime);

private:
    static bagel::Mask getMask();
};

/**
 * @brief System for handling weapons
 *
 * This system manages weapon selection, firing, and ammunition.
 */
class WeaponSystem {
public:
    static void update(float deltaTime);

private:
    static bagel::Mask getMask();
};

/**
 * @brief System for processing player input
 *
 * This system reads input and updates relevant components accordingly.
 */
class InputSystem {
public:
    static void update(float deltaTime);

private:
    static bagel::Mask getMask();
};

/**
 * @brief System for managing health
 *
 * This system handles health changes, death conditions, and related effects.
 */
class HealthSystem {
public:
    static void update(float deltaTime);

private:
    static bagel::Mask getMask();
};

// Entity creation functions

/**
 * @brief Creates a player entity
 *
 * @param x Initial X position
 * @param y Initial Y position
 * @return bagel::Entity The created player entity
 */
bagel::Entity createPlayer(float x, float y);

/**
 * @brief Creates a projectile entity
 *
 * @param x Initial X position
 * @param y Initial Y position
 * @param velX Initial X velocity
 * @param velY Initial Y velocity
 * @param weaponKind Kind of weapon that fired this projectile
 * @param owner Entity that fired this projectile
 * @return bagel::Entity The created projectile entity
 */
bagel::Entity createProjectile(float x, float y, float velX, float velY, Weapon::Kind weaponKind, bagel::ent_type owner = {-1});

/**
 * @brief Creates a terrain surface entity
 *
 * @param x X position
 * @param y Y position
 * @return bagel::Entity The created terrain entity
 */
bagel::Entity createTerrain(float x, float y);

/**
 * @brief Creates a collectable item entity
 *
 * @param x X position
 * @param y Y position
 * @param kind Kind of collectable
 * @param value Value associated with the collectable
 * @return bagel::Entity The created collectable entity
 */
bagel::Entity createCollectable(float x, float y, Collectable::Kind kind, int value);

} // namespace worms