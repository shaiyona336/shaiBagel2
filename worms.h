/**
 * @file worms.h
 * @brief A simplified Worms-like game implementation using ECS architecture
 */
 #pragma once

 #include <vector>
 #include <string>
 #include "bagel.h"

 constexpr float TIME_TO_LIVE = 3.0f;
 constexpr int STARTING_HEALTH = 100;
 constexpr float DEFAULT_WEIGHT = 1.0f;
 constexpr int DEFAULT_AMMO = 10;
 constexpr int DEFAULT_PACK_VALUE = 25;

 namespace worms {

 //components

 /**
  * @brief component for storing position information
  * dense component storing x,y coordinates of an entity
  */
 struct Position {
     float x = 0.0f;
     float y = 0.0f;
 };

 /**
  * @brief component for storing health information
  * dense component to store health of diffrent players
  */
 struct Health {
     int value = STARTING_HEALTH;
 };

 /**
  * @brief component for storing weapon information
  * sparse component for weapon, store type and ammo
  */
 struct Weapon {
     enum class Kind {
         BAZOOKA,
         GRENADE,
         SHOTGUN
     };

     Kind kind = Kind::BAZOOKA;
     int ammo = DEFAULT_AMMO;
 };

 /**
  * @brief component for storing physics information
  * dense component for storing physics information of entitys
  * store acceleration, velocity, weight and if affected by gravity
  */
 struct Physics {
     float accelX = 0.0f;
     float accelY = 0.0f;
     float velX = 0.0f;
     float velY = 0.0f;
     float weight = DEFAULT_WEIGHT;
     bool isAffectedByGravity = true;
 };

 /**
  * @brief component for storing projectile data information
  * sparse component for storing projectile data for projectiles
  * store weapon kind and time for explosion (if grenade for example)
  */
 struct ProjectileData {
     Weapon::Kind kind;
     float timeToLive = -1.0f;

     ProjectileData(Weapon::Kind k) : kind(k) {
         if (k == Weapon::Kind::GRENADE) {
             timeToLive = TIME_TO_LIVE;
         }
     }
 };

 /**
  * @brief component for input
  * sparse component for storing current state of input on entity
  * store move direction, is jump, is fire, aim angle
  */
 struct Input {
     float moveDirection = 0.0f; //-1.0 left to 1.0 right
     bool jump = false;
     bool fire = false;
     float aimAngle = 0.0f;
 };

 /**
  * @brief component for collectable items
  * Represents an item that can be collected by players.
  * sparse component for storing information on collectable items
  * store kind of item to pick, and value if it is health or ammo
  */
 struct Collectable {
     enum class Type {
         HEALTH,
         AMMO,
         WEAPON
     };

     Type kind = Type::HEALTH;
     int value = DEFAULT_PACK_VALUE;
 };

 //systems

 /**
  * @brief system for handling collisions
  * affect health, position
  */
 class CollisionSystem {
 public:
     static void update(float deltaTime);

 private:
     static bagel::Mask getMask();
 };

 /**
  * @brief system for handling physics
  * update positions of entities based on their physics properties
  */
 class PhysicsSystem {
 public:
     static void update(float deltaTime);

 private:
     static bagel::Mask getMask();
 };

 /**
  * @brief system for handling weapons
  * based on input and weapon, handling ammo, fire projectiles, weapon selection
  */
 class WeaponSystem {
 public:
     static void update(float deltaTime);

 private:
     static bagel::Mask getMask();
 };

 /**
  * @brief system for handling projectiles
  * update projectile timing, triggering appropriate effects based on weapon kind
  */
 class ProjectileSystem {
 public:
     static void update(float deltaTime);

 private:
     static bagel::Mask getMask();
 };

 /**
  * @brief system for handling player input
  * based on input, update relevant components
  */
 class InputSystem {
 public:
     static void update(float deltaTime);

 private:
     static bagel::Mask getMask();
 };

 /**
  * @brief system for managing health
  * based on health handling scenrios in the game like if health < 0 delete entity
  * another example health < 40 turn worm to red, or after health pack turn worm to green for a while
  */
 class HealthSystem {
 public:
     static void update(float deltaTime);

 private:
     static bagel::Mask getMask();
 };

 //entities

 /**
  * @brief creates a player entity
  * @param x initial x position
  * @param y initial y position
  * @return bagel::Entity the created player entity
  */
 bagel::Entity createPlayer(float x, float y);

 /**
  * @brief creates a projectile entity
  * @param x initial x position
  * @param y initial y position
  * @param velX initial x velocity
  * @param velY initial y velocity
  * @param weaponKind weapon that fired this projectile
  */
 bagel::Entity createProjectile(float x, float y, float velX, float velY, Weapon::Kind weaponKind);

 /**
  * @brief creates a terrain surface entity
  * @param x x position
  * @param y y position
  * @return bagel::Entity the created terrain entity
  */
 bagel::Entity createTerrain(float x, float y);

 /**
  * @brief creates a collectable item entity
  *
  * @param x x position
  * @param y y position
  * @param type type of collectable (health/ammo/weapon)
  * @param value value of collectable (amount health/ammo or kind of weapon)
  * @return bagel::Entity the created collectable entity
  */
 bagel::Entity createCollectable(float x, float y, Collectable::Type type, int value);

 }