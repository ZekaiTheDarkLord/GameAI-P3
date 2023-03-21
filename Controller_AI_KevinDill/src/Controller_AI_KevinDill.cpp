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
    srand(time(nullptr));

    if (firstTick) {
        desireToAttack = rand() % 3 + 1;
        std::cout << "start AI. Choose deisre pattern first: " << desireToAttack << std::endl;
        firstTick = false;
    }

    // defense in random tick
    if (defenseCount >= defenseDuration) {
        // std::cout << "defense" << std::endl;
        playGame(allyMobs, enemyMobs);
        defenseCount = 0;
        defenseDuration = rand() % 25 + 10;
    } else {
        defenseCount++;
    }
}

void Controller_AI_KevinDill::playGame(std::vector<Entity *> allyMobs, std::vector<Entity *> enemyMobs) {
    assert(m_pPlayer);
    std::cout << "----------------New Round. " << "Desire to attack = " << desireToAttack << "-------------------------" << std::endl;

    if (isWaiting) {
        if (!enemyMobs.empty()) {
            std::cout << "Decision: AI stop waiting because there is enemy placed." << std::endl;
            std::cout << "------------End Waiting-----------" << std::endl;
            isWaiting = false;
        }
        else {
            if (m_pPlayer->getElixir() >= 5) {
                std::cout << "Decision: AI step waiting because there is enough elixir" << std::endl;
                std::cout << "------------End Waiting-----------" << std::endl;
                isWaiting = false;
            }
            else {
                std::cout << "Decision: AI is waiting because he wants to wait until he has sufficient elixir." << std::endl;
                return;
            }
            return;
        }
    }



    // sufficient elixir
    if (m_pPlayer->getElixir() >= 6) {
        std::cout << "Decision Branch: AI has high Elixir." << std::endl;
        // if enemy low threat
        if (getMobsThreatLevel(enemyMobs) <= getThreatTolerance()) {
            std::cout << "Decision Branch: Enemy has low threat." << std::endl;
            // low threat
            if (getMobsThreatLevel(allyMobs) < 3) {
                std::cout << "Decision Branch: Ally has low threat." << std::endl;
                std::cout << "Try to organize attacks since both of the player has place little mob and AI try to do something." << std::endl;
                organizeAttacks(desireToAttack, allyMobs, enemyMobs);
            } else {
                std::cout << "Decision Branch: Ally has high threat." << std::endl;
              
                // wait for enemy's action / attack
                if (desireToAttack != 3) {
                    int decision = rand() % 6;

                    if (decision >= 4) {
                        desireToAttack++;
                        std::cout << "Decision: Enhance this attack. Increase desire to attack since it is a good chance" << std::endl;
                    }
                }

                std::cout << "Decision: Add more mobs on this attack." << std::endl;
                organizeAttacks(desireToAttack, allyMobs, enemyMobs);
            }
        } else {
            std::cout << "Decision Branch: Enemy has high threat." << std::endl;
            // aggressive strategy only cares about archers
            if (desireToAttack == 3) {
                std::vector<Entity *> archers = getMobInCertainType(iEntityStats::Archer, enemyMobs);
                if (!archers.empty()) {
                    std::cout << "Decision: Only defense from archers since AI wants to save elixir to attack." << std::endl;
                    dealWithOneEnemy(allyMobs, enemyMobs, getHighestPriorityEnemy(archers));
                }
            } else {
                std::cout << "Decision: Nomally defense enemy since it has some threat." << std::endl;
                defense(allyMobs, enemyMobs);
            }
        }
    } else if (m_pPlayer->getElixir() < 3) {
        std::cout << "Decision Branch: low Elixir." << std::endl;
        // if insufficient Elixir
        if (getMobsThreatLevel(getMobsOnThisSide(m_pPlayer->isNorth(), enemyMobs)) <= getThreatTolerance() + 1) {
            std::cout << "Decision Branch: low enemy threat level" << std::endl;
            std::cout << "Decision: wait for enough elixir." << std::endl << "------------Start Waiting-----------" << std::endl;
            if (desireToAttack != 3) {
                isWaiting = true;
            }
        }
        else {
            std::cout << "Decision Branch: high enemy threat level" << std::endl;
            if (!enemyMobs.empty()) {
                std::cout << "Decision: defense enemy in low elixir." << std::endl;
                defense(allyMobs, enemyMobs);
            }

            if (getMobsThreatLevel(enemyMobs) - getMobsThreatLevel(allyMobs) > 2.5f) {
                int decision = rand() % 6;
                if (decision > 3 && desireToAttack > 1) {
                    desireToAttack--;
                    std::cout << "Decision: desire to attack decrease." << std::endl;
                }
            }
        }
        
    } else {
        std::cout << "Decision Branch: AI has middle Elixir." << std::endl;
        // defense first, then organize attack
        if (getMobsThreatLevel(getMobsOnThisSide(m_pPlayer->isNorth(), enemyMobs)) <= getThreatTolerance()) {
            std::cout << "Decision Branch: enmey has low threat level." << std::endl;
            std::cout << "Decision: Simply organize attacks." << std::endl;
            // low threat
            organizeAttacks(desireToAttack, allyMobs, enemyMobs);
        } else {
            std::cout << "Decision Branch: enmey has high threat level." << std::endl;
            // aggressive strategy only cares about archers
            if (desireToAttack == 3) {
                std::cout << "Decision Branch: High desire to attack" << std::endl;
                std::vector<Entity*> archers = getMobInCertainType(iEntityStats::Archer, enemyMobs);
                if (!archers.empty()) {
                    std::cout << "Decision Branch: Simply deal archers." << std::endl;
                    dealWithOneEnemy(allyMobs, enemyMobs, getHighestPriorityEnemy(archers));
                }

                std::cout << "Decision: Wait to save elixir for attack." << std::endl;
            } else {
                std::cout << "Decision Branch: middle or low desire to attack" << std::endl;
                std::cout << "Decision: defense first and attack second" << std::endl;
                defense(allyMobs, enemyMobs);
                organizeAttacks(desireToAttack, allyMobs, enemyMobs);
            }
        }
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
    std::cout << "agressive attack mode." << std::endl;

    assert(m_pPlayer);

    std::vector<Entity *> giants = getMobInCertainType(iEntityStats::Giant, allyMobs);
    std::vector<Entity *> swords = getMobInCertainType(iEntityStats::Swordsman, allyMobs);
    std::vector<Entity *> archers = getMobInCertainType(iEntityStats::Archer, allyMobs);
    std::vector<Entity*> rogues = getMobInCertainType(iEntityStats::Rogue, allyMobs);


    // if no giants exist,
    // 1. if enough archer
    // 2. place a giant or a swordsman in front of the bridge
    if (giants.empty() && swords.empty()) {
        if (archers.size() >= 2) {
            if (m_pPlayer->getElixir() >= 2) {
                placeMobInFront(iEntityStats::Archer, m_pPlayer->isNorth(), attackLeftSide());
            }
        }

        if (m_pPlayer->getElixir() >= 7) {
            if (getMobsThreatLevel(enemyMobs) <= 5 && getMobsThreatLevel(enemyMobs) > 2) {
                bool isLeft = isPosOnLeft(enemyMobs.front()->getPosition());
                placeMobInFront(iEntityStats::Giant, m_pPlayer->isNorth(), isLeft);
            } else if (getMobsThreatLevel(enemyMobs) <= 2) {
                placeMobInFront(iEntityStats::Giant, m_pPlayer->isNorth(), attackLeftSide());
            }
        } else if (m_pPlayer->getElixir() >= 5) {
            if (getMobsThreatLevel(enemyMobs) <= 3 && getMobsThreatLevel(enemyMobs) > 0) {
                bool isLeft = isPosOnLeft(enemyMobs.front()->getPosition());
                placeMobInFront(iEntityStats::Swordsman, m_pPlayer->isNorth(), isLeft);
            } else if (getMobsThreatLevel(enemyMobs) == 0) {
                placeMobInFront(iEntityStats::Swordsman, m_pPlayer->isNorth(), attackLeftSide());
            }
        }
    } else {
        bool isLeft = true;
        if (!giants.empty()) {
            isLeft = isPosOnLeft(giants.front()->getPosition());
        } else if (!swords.empty()) {
            isLeft = isPosOnLeft(swords.front()->getPosition());
        }

        if (m_pPlayer->getElixir() >= 2) {
            int decision = rand() % 3;
            
            if (decision > 1 && rogues.empty() && !giants.empty()) {
                gracefullyPlaceMob(iEntityStats::Rogue, giants.front());
            }
            else {
                if (!giants.empty()) {
                    if (!isOnThisSide(m_pPlayer->isNorth(), giants.front()->getPosition())) {
                        placeMobInFront(iEntityStats::Archer, m_pPlayer->isNorth(), isLeft);
                    }
                }
                else {
                    if (!isOnThisSide(m_pPlayer->isNorth(), swords.front()->getPosition())) {
                        placeMobInFront(iEntityStats::Archer, m_pPlayer->isNorth(), isLeft);
                    }
                }
            }
        }
    }
}

void
Controller_AI_KevinDill::normalAttack(const std::vector<Entity *> allyMobs, const std::vector<Entity *> enemyMobs) {
    std::cout << "normal attack mode." << std::endl;

    assert(m_pPlayer);
    
    if (getMobsThreatLevel(getMobsOnThisSide(m_pPlayer->isNorth(), enemyMobs)) >= 3) {
        // reach here
        return;
    }

    std::vector<Entity *> giants = getMobInCertainType(iEntityStats::Giant, allyMobs);
    std::vector<Entity *> swords = getMobInCertainType(iEntityStats::Swordsman, allyMobs);
    std::vector<Entity *> archers = getMobInCertainType(iEntityStats::Archer, allyMobs);
    std::vector<Entity*> rogues = getMobInCertainType(iEntityStats::Rogue, allyMobs);


    if (m_pPlayer->getElixir() >= 5) {
        if (giants.empty()) {
            if (swords.empty()) {
                int decision = rand() % 6;
                if (decision > 3) {
                    placeMobInFront(iEntityStats::Swordsman, m_pPlayer->isNorth(), attackLeftSide());
                }
            } else {
                placeMobInBot(iEntityStats::Archer, m_pPlayer->isNorth(),
                              isPosOnLeft(swords.front()->getPosition()));
            }

            if (allyMobs.empty() && m_pPlayer->getElixir() > 7) {
                placeMobInBot(iEntityStats::Giant, m_pPlayer->isNorth(), attackLeftSide());
            } else if (!getMobsOnThisSide(m_pPlayer->isNorth(), allyMobs).empty() &&
                       getMobsThreatLevel(
                               getMobsOnThisSide(m_pPlayer->isNorth(), allyMobs)) >= getMobsThreatLevel(
                               getMobsOnThisSide(m_pPlayer->isNorth(), enemyMobs))) {

                if (rogues.size() >= 2) {
                    placeMobInBot(iEntityStats::Giant, m_pPlayer->isNorth(),
                        isPosOnLeft(rogues.front()->getPosition()));
                }
                else {
                    placeMobInFront(iEntityStats::Giant, m_pPlayer->isNorth(),
                        isPosOnLeft(allyMobs.front()->getPosition()));
                }
            }
        }
    } else if (m_pPlayer->getElixir() >= 2) {
        if (!giants.empty()) {
            if (isOnThisSide(m_pPlayer->isNorth(), giants.front()->getPosition())) {
                int decision = rand() % 2;

                if (decision == 1) {
                    gracefullyPlaceMob(iEntityStats::Rogue, giants.front());
                } else {
                    gracefullyPlaceMob(iEntityStats::Archer, giants.front());
                }
            } else if (!swords.empty()) {
                gracefullyPlaceMob(iEntityStats::Rogue, giants.front());
            }
        }
    }
}

void
Controller_AI_KevinDill::passiveAttack(const std::vector<Entity *> allyMobs, const std::vector<Entity *> enemyMobs) {
    std::cout << "passive attack mode." << std::endl;

    assert(m_pPlayer);

    std::vector<Entity *> giants = getMobInCertainType(iEntityStats::Giant, allyMobs);
    std::vector<Entity*> rogues = getMobInCertainType(iEntityStats::Rogue, allyMobs);

    if (getMobsThreatLevel(allyMobs) - getMobsThreatLevel(enemyMobs) > 3.f) {
        if (m_pPlayer->getElixir() >= 5) {
            if (getMobsThreatLevel(allyMobs) >= 3) {
                if (giants.empty()) {
                    if (rogues.size() >= 4) {
                        placeMobInBot(iEntityStats::Giant, m_pPlayer->isNorth(), attackLeftSide());
                    }
                    else if (getMobsThreatLevel(allyMobs) - getMobsThreatLevel(rogues) >= 3){
                        placeMobInFront(iEntityStats::Giant, m_pPlayer->isNorth(), attackLeftSide());
                    }
                }
            }
        }
    }
    else if (getMobsThreatLevel(enemyMobs) == 0) {
        if (m_pPlayer->getElixir() > 7) {
            std::cout << "reach here" << std::endl;
            if (getMobsThreatLevel(enemyMobs) == 0) {
                if (rogues.empty()) {
                    int decision = rand() % 2;
                    bool isLeft = decision == 1;
                    placeMobInBot(iEntityStats::Rogue, m_pPlayer->isNorth(), isLeft);
                }
                else {
                    placeMobInBot(iEntityStats::Rogue, m_pPlayer->isNorth(), isPosOnLeft(rogues.front()->getPosition()));

                }
            }
        }
    }
}

void Controller_AI_KevinDill::defense(const std::vector<Entity *> &allyMobs, const std::vector<Entity *> &enemyMobs) {
    assert(m_pPlayer);

    // get all the enemy mobs that pass the bridge
    std::vector<Entity *> mobsPassBridge = getMobsWithinRange(m_pPlayer->isNorth(), enemyMobs);

    // get all mobs that needs extra defense
    std::vector<Entity *> mobsShouldTakeCare = getEnemyShouldTakeCare(mobsPassBridge);

    // 1. if has enemy that should take care:
    // 2. deal with this enemy until it is solved
    // 3. update mobs should take care
    if (!mobsShouldTakeCare.empty()) {
        Entity *e = getHighestPriorityEnemy(mobsShouldTakeCare);
        int timeout = 50;
        int count = 0;

        while (!dealWithOneEnemy(allyMobs, enemyMobs, e) && count <= timeout) {
            e = getHighestPriorityEnemy(mobsPassBridge);
            count++;

            if (m_pPlayer->getElixir() < 2) {
                break;
            }
        }
    }
}

std::vector<Entity *>
Controller_AI_KevinDill::getMobsOnThisSide(bool isNorth, const std::vector<Entity *> &mobs) {
    assert(m_pPlayer);

    std::vector<Entity *> result = std::vector<Entity *>();

    for (auto e: mobs) {
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

bool
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

    // if notice level is enough, do nothing, update treated enemy
    if (noticeOnThisEnemy >= enemy->getStats().getElixirCost() + rand() % 2 - 1) {
        enemyTreated.insert(enemy);
        return false;
    } else {
        // place the mob
        if (m_pPlayer->getElixir() >= 3) {
            int seed = rand() % 2;
            gracefullyPlaceMob(intToMob(seed), enemy);
            return true;
        } else if (m_pPlayer->getElixir() == 2) {
            int seed = rand() % 1;
            gracefullyPlaceMob(intToMob(seed), enemy);
            return true;
        }
    }
}

void Controller_AI_KevinDill::gracefullyPlaceMob(iEntityStats::MobType placeMobType, Entity *enemy) {
    assert(m_pPlayer);
    float leastPlaceDis = enemy->getStats().getSize() * 1.5f;
    Vec2 enemyPos = enemy->getPosition();

    // if place rogue and treat an enemy giant, place behind tower.
    //if (placeMobType == iEntityStats::Rogue && (enemy->getStats().getMobType() == iEntityStats::Giant ||
    //                                            enemy->getStats().getMobType() == iEntityStats::Swordsman)) {
    //    float towerX = enemyPos.x < (float) GAME_GRID_WIDTH / 2 ? PrincessLeftX : PrincessRightX;
    //    float towerY = m_pPlayer->isNorth() ? NorthPrincessY : SouthPrincessY;

    //    enemyPos = Vec2(towerX, towerY);
    //    leastPlaceDis = 2.8f;
    //}

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

float Controller_AI_KevinDill::getMobsThreatLevel(std::vector<Entity *> mobs) {
    float threatLevel = 0;

    for (auto m: mobs) {
        threatLevel += m->getStats().getElixirCost();
    }

    return threatLevel;
}

float Controller_AI_KevinDill::getThreatTolerance() {
    switch (desireToAttack) {
        case 1:
            return 0;
        case 2:
            return 3;
        case 3:
            return 5;
    }
}

bool Controller_AI_KevinDill::attackLeftSide() {
    assert(m_pPlayer);

    iPlayer::EntityData leftTower = m_pPlayer->getOpponentBuilding(1);
    iPlayer::EntityData rightTower = m_pPlayer->getOpponentBuilding(2);

    return leftTower.m_Health < rightTower.m_Health;
}

void Controller_AI_KevinDill::placeMobInFront(iEntityStats::MobType mobType, bool isNorth, bool isLeft) {
    assert(m_pPlayer);

    float placeX = isLeft ? LEFT_BRIDGE_CENTER_X : RIGHT_BRIDGE_CENTER_X;
    float placeY = isNorth ? RIVER_TOP_Y - 0.5f : RIVER_BOT_Y + 0.5f;
    Vec2 placePos = Vec2(placeX, placeY);

    m_pPlayer->placeMob(mobType, placePos);
}

void Controller_AI_KevinDill::placeMobInBot(iEntityStats::MobType mobType, bool isNorth, bool isLeft) {
    assert(m_pPlayer);

    float placeX = isLeft ? LEFT_BRIDGE_CENTER_X : RIGHT_BRIDGE_CENTER_X;
    float placeY = isNorth ? 0 + 0.5f : GAME_GRID_HEIGHT - 0.5f;
    Vec2 placePos = Vec2(placeX, placeY);

    m_pPlayer->placeMob(mobType, placePos);
}

bool Controller_AI_KevinDill::isPosOnLeft(Vec2 pos) {
    return pos.x < BRIDGE_WIDTH / 2;
}

// get the mobs that will pass the bridge
std::vector<Entity*> Controller_AI_KevinDill::getMobsWithinRange(bool isNorth, const std::vector<Entity*>& mobs) {
    assert(m_pPlayer);

    std::vector<Entity *> result = std::vector<Entity *>();

    for (auto e: mobs) {
        if (isWithinRange(m_pPlayer->isNorth(), e->getPosition())) {
            result.push_back(e);
        }
    }

    return result;
}

// check whether the mob is on this side or opponent's side
bool Controller_AI_KevinDill::isWithinRange(bool isNorth, const Vec2& pos){
    if (isNorth) {
        return pos.y < RIVER_TOP_Y - 1.5f;
    } else {
        return pos.y > RIVER_BOT_Y + 1.5f;
    }
}






