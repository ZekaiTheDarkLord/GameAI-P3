// MIT License
// 
// Copyright(c) 2020 Kevin Dill
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this softwareand associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// 
// The above copyright noticeand this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <set>
#include "iController.h"
#include "../../Game/src/Entity.h"



class Controller_AI_KevinDill : public iController
{
public:
    Controller_AI_KevinDill() {}
    virtual ~Controller_AI_KevinDill() {}

    void tick(float deltaTSec, std::vector<Entity *> allyMobs, std::vector<Entity *> enemyMobs);

    int GetFoo() const { return m_foo; }


private:
    int m_foo = 0;

    // wait until high elixir has enemy higher than tolerance
    bool isWaiting = false;
    int defenseCount = 0;
    int defenseDuration = 30;
    int desireToAttack = 1;
    bool firstTick = true;

    // the enemy that already treated by AI, what if the enemy is in the list but no mobs nearby to deal with it?
    std::set<Entity*> enemyTreated = std::set<Entity*>();

    // play the game with attacks and defense
    void playGame(std::vector<Entity*> allyMobs, std::vector<Entity*> enemyMobs);

    // organize attacks module: 3 = very aggressive, 2 = medium, 1 = passive
    void organizeAttacks(int aggressiveLevel, std::vector<Entity*> allyMobs, std::vector<Entity*> enemyMobs);

    // * all in elixir on attack
    // 1. if no enemy, organize attack on top of bridge
    // 2. if enemy has relatively low threat(far from tower, low attack ability), organize attack on another side
    // 3. if enemy has very large threat(may break the king tower), defense at lowest level -> most threat mob, ignore
    // single archer and rogue
    // 4. if has giant or knight at front, place archer when enough elixir
    void aggressiveAttack(std::vector<Entity*> allyMobs, std::vector<Entity*> enemyMobs);

    // * if no enemy, organize attack when elixir = 10, else always save 2 elixir
    // 1. if no enemy, place giant on the bottom first, then add archers when giant pass the bridge
    // 2. if has enemy, defense first. After defense, if has mobs left, when mob almost reach bridge, place giant on
    // bridge; if no mobs left, wait until elixir = 10
    void normalAttack(std::vector<Entity*> allyMobs, std::vector<Entity*> enemyMobs);

    // * will not actively attack, move when enemy move, will save rogues on back of the tower when elixir = 10
    // attack when there are mobs left and enough for a giant
    void passiveAttack(std::vector<Entity*> allyMobs, std::vector<Entity*> enemyMobs);

    // * will not move when current mobs can protect tower from this attack
    // * place one mob each time in order to behave like human players
    // hide rogue + archer/swordsman -> giant, archer/swordsman -> swordsman, swordsman -> rogue, swordsman -> archer
    void defense(const std::vector<Entity*>& allyMobs, const std::vector<Entity*>& enemyMobs);

    // get the mobs that will pass the bridge
    std::vector<Entity*> getMobsOnThisSide(bool isNorth, const std::vector<Entity*>& mobs);

    // check whether the mob is on this side or opponent's side
    static bool isOnThisSide(bool isNorth, const Vec2& pos);

    // get all the enemy that should take care -> enemies not in the treated set
    // also remove all the archers or rogues that without protection of swordsman or giant
    std::vector<Entity *> getEnemyShouldTakeCare(std::vector<Entity *> entity);

    // defense from a enemy depends on the exist ally mobs
    bool dealWithOneEnemy(const std::vector<Entity*>& allyMobs, const std::vector<Entity*>& enemyMobs,Entity* enemy);

    // generate a random number in a range
    static float randomInRange(float lowerBound, float upperBound);

    // place mob behind another mob within its attack range
    void gracefullyPlaceMob(iEntityStats::MobType placeMobType, Entity *enemy);

    // random helper
    iEntityStats::MobType intToMob(int num);

    // get the enemy that with the highest priority to be dealt with
    // basic idea: archer behind giant > rogue behind giant > swordsman behind giant >
    // giant far from tower > swordsman > giant far from tower
    Entity* getHighestPriorityEnemy(std::vector<Entity *> enemyMobs);

    // get certain type of mob in a vector
    std::vector<Entity *> getMobInCertainType(iEntityStats::MobType mobType, const std::vector<Entity *>& mobs);

    // returns whether one entity is behind another
    bool aBehindB(bool isNorth, Entity *mobA, Entity *mobB);

    // get the closest mob to a position
    Entity* getClosestMob(std::vector<Entity* > mobs, Vec2 pos);

    // evaluate the threat level of a group of mobs
    float getMobsThreatLevel(std::vector<Entity* > mobs);

    // get the tolerance of the mob depends on the current strategy.
    float getThreatTolerance();

    // decide whether attack the right side or the left side
    bool attackLeftSide();

    // place the mob behind the bridge
    void placeMobInFront(iEntityStats::MobType mobType, bool isNorth, bool isLeft);

    // place the mob behind the tower
    void placeMobInBot(iEntityStats::MobType mobType, bool isNorth, bool isLeft);

    // return the position on left or right side of the map
    bool isPosOnLeft(Vec2 pos);

    // get the mobs that will pass the bridge
    std::vector<Entity*> getMobsWithinRange(bool isNorth, const std::vector<Entity*>& mobs);

    // check whether the mob is on this side or opponent's side
    static bool isWithinRange(bool isNorth, const Vec2& pos);
};