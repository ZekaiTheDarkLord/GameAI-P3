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

#include "Controller_AI_KevinDill.h"
#include "Constants.h"
#include "EntityStats.h"
#include "iPlayer.h"
#include "Vec2.h"

void Controller_AI_KevinDill::tick(float deltaTSec, const std::vector<Entity *> allyMobs,
                                   const std::vector<Entity *> enemyMobs) {
    assert(m_pPlayer);

    // defense in random tick
    if (defenseCount >= defenseDuration) {
        std::cout << "defense" << std::endl;
        defense(allyMobs, enemyMobs);
        defenseCount = 0;
        defenseDuration = rand() % 20 + 15;
    } else {
        defenseCount++;
    }
}

void Controller_AI_KevinDill::organizeAttacks(int aggressiveLevel, const std::vector<Entity *> allyMobs,
                                              const std::vector<Entity *> enemyMobs) {
    switch (aggressiveLevel) {
        case 1:
            passiveAttack(allyMobs, enemyMobs);
            break;
        case 2:
            normalAttack(allyMobs, enemyMobs);
            break;
        case 3:
            aggressiveAttack(allyMobs, enemyMobs);
            break;
        default:
            std::cout << "No attack pattern recognized!" << std::endl;
            break;
    }
}

void
Controller_AI_KevinDill::aggressiveAttack(const std::vector<Entity *> allyMobs, const std::vector<Entity *> enemyMobs) {
    assert(m_pPlayer);
}

void
Controller_AI_KevinDill::normalAttack(const std::vector<Entity *> allyMobs, const std::vector<Entity *> enemyMobs) {
    assert(m_pPlayer);
}

void
Controller_AI_KevinDill::passiveAttack(const std::vector<Entity *> allyMobs, const std::vector<Entity *> enemyMobs) {
    assert(m_pPlayer);
}

void Controller_AI_KevinDill::defense(const std::vector<Entity *> &allyMobs, const std::vector<Entity *> &enemyMobs) {
    assert(m_pPlayer);

    // get all the enemy mobs that pass the bridge
    std::vector<Entity *> mobsPassBridge = getPassBridgeEnemies(m_pPlayer->isNorth(), enemyMobs);

    // get all mobs that needs extra defense
    std::vector<Entity *> mobsShouldTakeCare = getEnemyShouldTakeCare(mobsPassBridge);

    // 1. if has enemy that should take care:
    // 2. deal with this enemy until it is solved
    // 3. update mobs should take care
    if (!mobsShouldTakeCare.empty()) {
        Entity *e = getHighestPriorityEnemy(mobsShouldTakeCare);
        dealWithOneEnemy(allyMobs, enemyMobs, e);
    }

//    for (auto e: mobsShouldTakeCare) {
//        dealWithOneEnemy(allyMobs, e);
//    }
}

std::vector<Entity *>
Controller_AI_KevinDill::getPassBridgeEnemies(bool north, const std::vector<Entity *> &enemyMobs) {
    assert(m_pPlayer);

    std::vector<Entity *> result = std::vector<Entity *>();

    for (auto e: enemyMobs) {
        if (isOnThisSide(m_pPlayer->isNorth(), e->getPosition())) {
            result.push_back(e);
        }
    }

    return result;
}

bool Controller_AI_KevinDill::isOnThisSide(bool isNorth, const Vec2 &pos) {
    if (isNorth) {
        return pos.y < RIVER_TOP_Y;
    } else {
        return pos.y > RIVER_BOT_Y;
    }
}

std::vector<Entity *>
Controller_AI_KevinDill::getEnemyShouldTakeCare(std::vector<Entity *> enemyMobs) {
    std::vector<Entity *> result = std::vector<Entity *>();

    for (auto e: enemyMobs) {
        // if the enemy is untreated
        if (enemyTreated.find(e) == enemyTreated.end()) {
            result.push_back(e);
        }
    }

    return result;
}

void
Controller_AI_KevinDill::dealWithOneEnemy(const std::vector<Entity *> &allyMobs, const std::vector<Entity *> &enemyMobs,
                                          Entity *enemy) {
    assert(m_pPlayer);

    const Vec2 &enemyPos = enemy->getPosition();

    // notice = sum of ally mobs within their attack range and is the closest enemy to them
    float noticeOnThisEnemy = 0;

    // calculate the notice level
    for (auto ally: allyMobs) {
        Vec2 allyPos = ally->getPosition();
        float distance = (allyPos - enemyPos).normalize();
        float enemySize = enemy->getStats().getSize();

        // is the enemy closest to this ally?
        bool isClosestToThat = (getClosestMob(enemyMobs, allyPos)->getPosition() == enemyPos);

        // if this ally mob notice the enemy mob
        noticeOnThisEnemy = ((distance < ally->getStats().getAttackRange() + enemySize) && isClosestToThat) ?
                            noticeOnThisEnemy +
                            ally->getStats().getElixirCost()
                                                                                                            : noticeOnThisEnemy;
    }

    std::cout << noticeOnThisEnemy << std::endl;

    // if notice level is enough, do nothing, update treated enemy
    if (noticeOnThisEnemy >= enemy->getStats().getElixirCost() + rand() % 2 - 1) {
        enemyTreated.insert(enemy);
        return;
    } else {
        // place the mob
        if (m_pPlayer->getElixir() >= 3) {
            int seed = rand() % 2;
            gracefullyPlaceMob(intToMob(seed), enemy);
            // noticeOnThisEnemy = seed == 1 ? noticeOnThisEnemy + 3 : noticeOnThisEnemy + 2;
        } else if (m_pPlayer->getElixir() == 2) {
            int seed = rand() % 1;
            gracefullyPlaceMob(intToMob(seed), enemy);
            // noticeOnThisEnemy += 2;
        }
        // if notice level enough, update treated enemy
//        if (noticeOnThisEnemy >= enemy->getStats().getElixirCost()) {
//            enemyTreated.insert(enemy);
//        }
    }
}

void Controller_AI_KevinDill::gracefullyPlaceMob(iEntityStats::MobType placeMobType, Entity *enemy) {
    assert(m_pPlayer);
    float leastPlaceDis = enemy->getStats().getSize() * 1.5f;
    Vec2 enemyPos = enemy->getPosition();

    // if place rogue and treat an enemy giant, place behind tower.
    if (placeMobType == iEntityStats::Rogue && (enemy->getStats().getMobType() == iEntityStats::Giant ||
                                                enemy->getStats().getMobType() == iEntityStats::Swordsman)) {
        float towerX = enemyPos.x < (float) GAME_GRID_WIDTH / 2 ? PrincessLeftX : PrincessRightX;
        float towerY = m_pPlayer->isNorth() ? NorthPrincessY : SouthPrincessY;

        enemyPos = Vec2(towerX, towerY);
        leastPlaceDis = 2.8f;
    }

    // if place archer, place in a further distance
    if (placeMobType == iEntityStats::Archer) {
        leastPlaceDis += randomInRange(2.5f, 5.f);
    }

    Vec2 placeVec = m_pPlayer->isNorth() ?
                    Vec2(randomInRange(0, 1), -1.f * randomInRange(0, 1)) :
                    Vec2(randomInRange(0, 1), randomInRange(0, 1));

    Vec2 placePos = enemyPos + placeVec * leastPlaceDis;
    m_pPlayer->placeMob(placeMobType, placePos);
}

float Controller_AI_KevinDill::randomInRange(float lowerBound, float upperBound) {
    return lowerBound + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (upperBound - lowerBound)));
}

iEntityStats::MobType Controller_AI_KevinDill::intToMob(int num) {
    switch (num) {
        case 0:
            return iEntityStats::Archer;
        case 1:
            return iEntityStats::Swordsman;
        default:
            return iEntityStats::Rogue;
    }
}

Entity *Controller_AI_KevinDill::getHighestPriorityEnemy(std::vector<Entity *> enemyMobs) {
    // basic idea: archer behind giant > rogue behind giant > swordsman behind giant >
    // giant > swordsman
    assert(m_pPlayer);

    std::vector<Entity *> giants = getMobInCertainType(iEntityStats::Giant, enemyMobs);
    std::vector<Entity *> archers = getMobInCertainType(iEntityStats::Archer, enemyMobs);
    std::vector<Entity *> swords = getMobInCertainType(iEntityStats::Swordsman, enemyMobs);
    std::vector<Entity *> rogues = getMobInCertainType(iEntityStats::Rogue, enemyMobs);

    float towerY = m_pPlayer->isNorth() ? NorthPrincessY : SouthPrincessY;
    Vec2 leftTowerPos = Vec2(PrincessLeftX, towerY);
    Vec2 rightTowerPos = Vec2(PrincessRightX, towerY);

    //---------------------------behind giant----------------------------------

    // search for archer behind giant
    for (auto a: archers) {
        for (auto g: giants) {
            if (aBehindB(m_pPlayer->isNorth(), a, g)) {
                return a;
            }
        }
    }

    // search for rogue behind giant
    for (auto r: rogues) {
        for (auto g: giants) {
            if (aBehindB(m_pPlayer->isNorth(), r, g)) {
                return r;
            }
        }
    }

    // search for swordsman behind giant
    for (auto s: swords) {
        for (auto g: giants) {
            if (aBehindB(m_pPlayer->isNorth(), s, g)) {
                return s;
            }
        }
    }

    //---------------------------behind swordsman----------------------------------
    // search for archer behind swordsman
    for (auto a: archers) {
        for (auto g: swords) {
            if (aBehindB(m_pPlayer->isNorth(), a, g)) {
                return a;
            }
        }
    }

    // search for rogue behind swordsman
    for (auto r: rogues) {
        for (auto g: swords) {
            if (aBehindB(m_pPlayer->isNorth(), r, g)) {
                return r;
            }
        }
    }

    // search for swordsman behind swordsman
    for (auto s: swords) {
        for (auto g: swords) {
            if (aBehindB(m_pPlayer->isNorth(), s, g)) {
                return s;
            }
        }
    }

    // search for closest: giants -> swordsman -> archers -> rogues
    if (!giants.empty()) {
        Entity *leftClosestMob = getClosestMob(giants, leftTowerPos);
        Entity *rightClosestMob = getClosestMob(giants, rightTowerPos);

        float leftToLeftDis = (leftClosestMob->getPosition() - leftTowerPos).normalize();
        float rightToRightDis = (rightClosestMob->getPosition() - rightTowerPos).normalize();

        Entity *returnEntity = leftToLeftDis < rightToRightDis ? leftClosestMob : rightClosestMob;
        return returnEntity;
    }

    if (!swords.empty()) {
        Entity *leftClosestMob = getClosestMob(swords, leftTowerPos);
        Entity *rightClosestMob = getClosestMob(swords, rightTowerPos);

        float leftToLeftDis = (leftClosestMob->getPosition() - leftTowerPos).normalize();
        float rightToRightDis = (rightClosestMob->getPosition() - rightTowerPos).normalize();

        Entity *returnEntity = leftToLeftDis < rightToRightDis ? leftClosestMob : rightClosestMob;
        return returnEntity;
    }

    if (!archers.empty()) {
        Entity *leftClosestMob = getClosestMob(archers, leftTowerPos);
        Entity *rightClosestMob = getClosestMob(archers, rightTowerPos);

        float leftToLeftDis = (leftClosestMob->getPosition() - leftTowerPos).normalize();
        float rightToRightDis = (rightClosestMob->getPosition() - rightTowerPos).normalize();

        Entity *returnEntity = leftToLeftDis < rightToRightDis ? leftClosestMob : rightClosestMob;
        return returnEntity;
    }

    if (!rogues.empty()) {
        Entity *leftClosestMob = getClosestMob(rogues, leftTowerPos);
        Entity *rightClosestMob = getClosestMob(rogues, rightTowerPos);

        float leftToLeftDis = (leftClosestMob->getPosition() - leftTowerPos).normalize();
        float rightToRightDis = (rightClosestMob->getPosition() - rightTowerPos).normalize();

        Entity *returnEntity = leftToLeftDis < rightToRightDis ? leftClosestMob : rightClosestMob;
        return returnEntity;
    }

    return nullptr;
}

std::vector<Entity *>
Controller_AI_KevinDill::getMobInCertainType(iEntityStats::MobType mobType, const std::vector<Entity *> &mobs) {
    std::vector<Entity *> result = std::vector<Entity *>();

    for (auto e: mobs) {
        if (e->getStats().getMobType() == mobType) {
            result.push_back(e);
        }
    }

    return result;
}

bool Controller_AI_KevinDill::aBehindB(bool isNorth, Entity *mobA, Entity *mobB) {
    bool isBehind = isNorth ? mobA->getPosition().y < mobB->getPosition().y :
                    mobA->getPosition().y > mobB->getPosition().y;
    bool onSameSide = ((mobA->getPosition().x < (float) GAME_GRID_WIDTH / 2.f) ==
                       (mobB->getPosition().x < (float) GAME_GRID_WIDTH / 2.f));
    return isBehind && onSameSide;
}

Entity *Controller_AI_KevinDill::getClosestMob(std::vector<Entity *> mobs, Vec2 pos) {
    float closestDistance = 9999;
    Entity *returnMob = nullptr;

    for (auto e: mobs) {
        float disTo = (e->getPosition() - pos).normalize();
        closestDistance = disTo < closestDistance ? disTo : closestDistance;
        returnMob = disTo == closestDistance ? e : returnMob;
    }

    return returnMob;
}






