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

#include "Mob.h"

#include "Constants.h"
#include "Game.h"
#include "HelperFunctions.h"

#include <algorithm>
#include <vector>


Mob::Mob(const iEntityStats &stats, const Vec2 &pos, bool isNorth)
        : Entity(stats, pos, isNorth), m_pWaypoint(NULL) {
    assert(dynamic_cast<const iEntityStats_Mob *>(&stats) != NULL);
}

void Mob::tick(float deltaTSec) {
    // Tick the entity first.  This will pick our target, and attack it if it's in range.
    Entity::tick(deltaTSec);

    // calculate the hidden time
    if (this->getStats().getMobType() == iEntityStats::MobType::Rogue) {
        m_chargeSpringAttack = m_hiddenTime >= 2.f;

        if (isHidden()) {
            std::cout << "Hide: " << m_hiddenTime << std::endl;
            m_hiddenTime += deltaTSec;
        } else {
            m_hiddenTime = 0.f;
        }
    }

    // if our target isn't in range, move towards it.
    if (!targetInRange()) {
        move(deltaTSec);
    }     // if target is in the range, don't move towards it but still check for the collision
    else {
        std::vector<Entity *> otherEntities = checkCollision();
        for (Entity *e: otherEntities) {
            if (e) {
                processCollision(e, deltaTSec, Vec2(0, 0));
            }
        }
        handleEdgeCollsion(Vec2(0, 0));
    }
}

bool Mob::isHidden() const {
    // Project 2: This is where you should put the logic for checking if a Rogue is
    // hidden or not.  It probably involves something related to calling Game::Get()
    // to get the Game, then calling getPlayer() on the game to get each player, then
    // going through all the entities on the players and... well, you can take it
    // from there.  Once you've implemented this function, you can use it elsewhere to
    // change the Rogue's behavior, damage, etc.  It is also used in by the Graphics
    // to change the way the character renders (Rogues on the South team will render
    // as grayed our when hidden, ones on the North team won't render at all).

    // TODO: This special case code for the Rogue doesn't belong in the Mob class - we
    // need some way to encapsulate and decouple.  I'm thinking maybe a lambda in the
    // EntityStats??


    // search for the giants for possible shelter to block sight
    std::vector<Entity *> possibleNoticeEntities = entityNoticeThisMob();

    std::vector<Entity *> allyEntities = std::vector<Entity *>();
    std::vector<Entity *> allyMobs = Game::get().getMobs(this->isNorth());
    std::vector<Entity *> allyBuildings = Game::get().getBuildings(this->isNorth());

    for (Entity *e: allyMobs) {
        if (e->getStats().getName() == "Giant") {
            allyEntities.push_back(e);
        }
    }

    allyEntities.insert(allyEntities.end(), allyBuildings.begin(), allyBuildings.end());

    // iterate through all enemies and ally giants and towers to see the rogue can be hidden from those
    for (auto e: possibleNoticeEntities) {
        bool canHideThisEnemy = false;

        for (auto a: allyEntities) {
            Vec2 allyPos = a->getPosition();
            Vec2 enemyPos = e->getPosition();

            // the direction enemy point to rogue
            Vec2 enemyToRogueDirection = this->getPosition() - e->getPosition();

            enemyToRogueDirection.normalize();

            // the euclid distance between enemy and ally
            float enemyToAllyDistance = euclidDistance(enemyPos, allyPos);

            // the displacement = direction * distance
            Vec2 displacement = enemyToRogueDirection * enemyToAllyDistance;

            //  if enemy position + displacement is in the range of the ally entity, then it is not spot
            Vec2 sight = enemyPos + displacement;
            canHideThisEnemy = canHideThisEnemy || (sight.x > allyPos.x - a->getStats().getSize() / 2 &&
                                                    sight.x < allyPos.x + a->getStats().getSize() / 2 &&
                                                    sight.y > allyPos.y - a->getStats().getSize() / 2 &&
                                                    sight.y < allyPos.y + a->getStats().getSize() / 2);
        }

        // if any of the ally can block the sight, this rogue is not be sighted by that enemy
        if (!canHideThisEnemy) {
            return false;
        }
    }

    // otherwise, the rogue is hidden at current tick
    return true;
}

void Mob::move(float deltaTSec) {
    // Project 2: You'll likely need to do some work in this function to get the
    // Rogue to move correctly (i.e. hide behind Giant or Tower, move with the Giant,
    // spring out when doing a sneak attack, etc).
    //   Of note, putting special case code for a single type of unit into the base
    // class function like this IS AN AWFUL IDEA, because it wouldn't scale as more
    // types of units are added.  We only have the one special unit type, so we can
    // afford to be a lazy in this instance... but if this were production code, I
    // would never do it this way!!
    //   TODO: Build a better system for encapsulating unit-specific logic, perhaps
    // by repurposing and expanding the EntityStats subclasses (which need some love
    // as well!)

    // If we have a target and it's on the same side of the river, we move towards it.
    //  Otherwise, we move toward the bridge.
    bool bMoveToTarget = false;
    if (!!m_pTarget) {
        bool imTop = m_Pos.y < (GAME_GRID_HEIGHT / 2);
        bool otherTop = m_pTarget->getPosition().y < (GAME_GRID_HEIGHT / 2);

        if (imTop == otherTop) {
            bMoveToTarget = true;
        }
    }

    Vec2 destPos;
    if (bMoveToTarget) {
        m_pWaypoint = NULL;
        destPos = m_pTarget->getPosition();
    } else {
        if (!m_pWaypoint) {
            m_pWaypoint = pickWaypoint();
        }
        destPos = m_pWaypoint ? *m_pWaypoint : m_Pos;
    }

    // Actually do the moving
    Vec2 moveVec = destPos - m_Pos;
    float distRemaining = moveVec.normalize();
    float moveDist = m_Stats.getSpeed() * deltaTSec;

    // if we're moving to m_pTarget, don't move into it
    if (bMoveToTarget) {
        assert(m_pTarget);
        distRemaining -= (m_Stats.getSize() + m_pTarget->getStats().getSize()) / 2.f;
        distRemaining = std::max(0.f, distRemaining);
    }

    // if this mob is a rogue, check whether it can spring attack and whether the target i in the spring range
    if (this->getStats().getMobType() == iEntityStats::MobType::Rogue) {
        // std::cout << "-----------------Rogue---------------------------\n";

        if (m_hiddenTime >= 2.f && distRemaining < this->getStats().getSpringRange() && bMoveToTarget) {
            m_Pos = distRemaining < this->getStats().getSpringSpeed() ?
                    m_Pos + moveVec * distRemaining :
                    m_Pos + moveVec * this->getStats().getSpringSpeed();

            m_hiddenTime = 0.f;

            std::cout << "spring and attack an enemy";

            return;
        }

        // if it is not hidden, go to the closest target
        if (!targetInRange() && m_hiddenTime != 0) {
            // search for giants and towers, hide if they are within range
            std::vector<Entity *> collidingEntities = std::vector<Entity *>();

            // get all the entities include buildings and mobs
            std::vector<Entity *> entities = std::vector<Entity *>();
            std::vector<Entity *> mobs = Game::get().getMobs(this->isNorth());
            std::vector<Entity *> buildings = Game::get().getBuildings(this->isNorth());

            // the closest distance from rogue to shelter
            float closestDistance = 999;
            Vec2 hidePosition = Vec2(0, 0);

            // whether the rogue hide behind a giant or a buliding
            bool hideGiant = false;
            bool hideBuilding = false;
            float hideSize = 0.f;

            // go through all mobs that is a giant
            for (const Entity *m: mobs) {
                if (m->getStats().getMobType() == iEntityStats::MobType::Giant) {
                    // if within prefer giant range and is the closer giant
                    float eucDis = euclidDistance(this->getPosition(), m->getPosition());

                    if (eucDis < this->getStats().preferGiantRange() && eucDis < closestDistance) {
                        closestDistance = eucDis;
                        hidePosition = Vec2(m->getPosition().x, m->getPosition().y);
                        hideSize = m->getStats().getSize();
                        hideGiant = true;
                    }
                }
            }

            // if follow giant, follow a giant, else hide from tower
            if (hideGiant) {
                moveAround(enemiesInSight(), hidePosition, hideSize, moveDist, hideGiant, deltaTSec, moveVec);
                return;
            } else {
                // judge whether will hide from building
                for (const Entity *b: buildings) {
                    // if within prefer giant range and is the closer giant
                    float eucDis = euclidDistance(this->getPosition(), b->getPosition());

                    // if current close to a building, move around building, otherwise pass and walk to building
                    if (eucDis < 1.f + b->getStats().getSize() / 2 && eucDis < closestDistance) {
                        closestDistance = eucDis;
                        hidePosition = Vec2(b->getPosition().x, b->getPosition().y);
                        hideBuilding = true;
                        hideSize = b->getStats().getSize();
                    }
                }

                // if the rogue can hide bedhind a building
                if (hideBuilding) {
                    std::cout << "hide from building\n";
                    moveAround(enemiesInSight(), hidePosition, hideSize / 2, moveDist, hideGiant, deltaTSec, moveVec);

                    // the rogue should hide on the back side of the buildin
                    bool inFrontTower = false; 
                    if (this->isNorth() && this->getPosition().y > hidePosition.y - hideSize / 2) {
                        moveVec = Vec2(hidePosition.x, hidePosition.y - hideSize / 2 - 0.5f) - m_Pos;
                        distRemaining = moveVec.normalize();
                        inFrontTower = true;
                    }
                    else if (!this->isNorth() && this->getPosition().y < hidePosition.y + hideSize / 2) {
                        // add more force in order to prevent the rogue from blocking by the tower
                        float sign = m_Pos.x - hidePosition.x > 0 ? 1.f : -1.f;
                        moveVec = Vec2(hidePosition.x + sign, hidePosition.y + hideSize / 2 + 0.5f) - m_Pos;
                        distRemaining = moveVec.normalize();
                        inFrontTower = true;
                    }

                    // this move should appear when there is no enemies on sight
                    if (inFrontTower && enemiesInSight().size() == 0) {
                        if (moveDist <= distRemaining) {
                            // if the mob will collid with the edge in the next tick, handle the collision
                            if (!handleEdgeCollsion(moveVec * moveDist)) {
                                // else, move the mob
                                m_Pos += moveVec * moveDist;
                            }
                        }
                        else {
                            // if the mob will collid with the edge in the next tick, handle the collision
                            if (!handleEdgeCollsion(moveVec * distRemaining)) {
                                // else, move the mob
                                m_Pos += moveVec * distRemaining;
                            }
                        }
                    }

                    return;
                } else {
                    // neither hide behind a giant nor building, moving towards closest giant or building
                    std::vector<Entity *> entitiesInSight = seekEntityWithinRadius(this->getStats().getSightRadius(),
                                                                                   this->isNorth());

                    // find the giant closest and within sight range
                    bool findGiant = false;
                    Vec2 closestHidePos = Vec2(0, 0);
                    float closestHideDis = 999;

                    // iterate all entities in sight
                    for (auto e: entitiesInSight) {
                        float distanceToGiant = euclidDistance(this->getPosition(), e->getPosition());

                        if (e->getStats().getName() == "Giant" &&
                            euclidDistance(this->getPosition(), e->getPosition()) < closestHideDis) {
                            closestHidePos = e->getPosition();
                            closestHideDis = distanceToGiant;
                            findGiant = true;
                        }
                    }


                    // if not find the giant, go to the closest building
                    if (!findGiant) {
                        std::vector<Entity *> allyBuildings = Game::get().getBuildings(this->isNorth());
                        for (auto e: allyBuildings) {
                            float distanceToTower = euclidDistance(this->getPosition(), e->getPosition());

                            if (euclidDistance(this->getPosition(), e->getPosition()) < closestHideDis) {
                                closestHidePos = e->getPosition();
                                closestHideDis = distanceToTower;
                                findGiant = true;
                            }
                        }
                    }

                    // set the target towards the next hide pos
                    moveVec = closestHidePos - m_Pos;
                    distRemaining = moveVec.normalize();

                    std::cout << "move Vec set. " << moveVec.x << " " << moveVec.y << std::endl;
                }
            }
        }
    }

    if (moveDist <= distRemaining) {
        // if the mob will collid with the edge in the next tick, handle the collision
        if (!handleEdgeCollsion(moveVec * moveDist)) {
            // else, move the mob
            m_Pos += moveVec * moveDist;
        }
    } else {
        // if the mob will collid with the edge in the next tick, handle the collision
        if (!handleEdgeCollsion(moveVec * distRemaining)) {
            // else, move the mob
            m_Pos += moveVec * distRemaining;
        }

        // if the destination was a waypoint, find the next one and continue movement
        if (m_pWaypoint) {
            m_pWaypoint = pickWaypoint();
            destPos = m_pWaypoint ? *m_pWaypoint : m_Pos;
            moveVec = destPos - m_Pos;
            moveVec.normalize();
            m_Pos += moveVec * distRemaining;
        }
    }

    // Project 1: This is where your collision code will be called from
    // Move process Collision before move
    std::vector<Entity *> otherEntities = checkCollision();

    // get all the entities that may collide with the mob
    for (Entity *e: otherEntities) {
        if (e) {
            // handle each collision
            processCollision(e, deltaTSec, moveVec);
        }
    }
}

const Vec2 *Mob::pickWaypoint() {
    // Project 2:  You may need to make some adjustments here, so that Rogues will go
    // back to a friendly tower when they have nothing to attack or hide behind, rather
    // than suicide-rushing the enemy tower (which they can't damage).
    //   Again, special-case code in a base class function bad.  Encapsulation good.

    if (this->getStats().getMobType() != iEntityStats::MobType::Rogue) {
        float smallestDistSq = FLT_MAX;
        const Vec2 *pClosest = NULL;

        for (const Vec2 &pt: Game::get().getWaypoints()) {
            // Filter out any waypoints that are behind (or barely in front of) us.
            // NOTE: (0, 0) is the top left corner of the screen
            float yOffset = pt.y - m_Pos.y;
            if ((m_bNorth && (yOffset < 1.f)) ||
                (!m_bNorth && (yOffset > -1.f))) {
                continue;
            }

            float distSq = m_Pos.distSqr(pt);
            if (distSq < smallestDistSq) {
                smallestDistSq = distSq;
                pClosest = &pt;
            }
        }

        return pClosest;
    } else {
        return NULL;
    }
}

// Project 1: 
//  1) return a vector of mobs that we're colliding with
//  2) handle collision with towers & river 
// change the return type of the checkCollision to a vector
std::vector<Entity *> Mob::checkCollision() {
    std::vector<Entity *> collidingEntities = std::vector<Entity *>();


    // check collision north and south
    for (int i = 0; i < 2; i++) {
        bool northOrSouth = (i == 1);

        // get all the entities include buildings and mobs
        std::vector<Entity *> entities = std::vector<Entity *>();
        std::vector<Entity *> mobs = Game::get().getMobs(northOrSouth);
        std::vector<Entity *> buildings = Game::get().getBuildings(northOrSouth);
        entities.insert(entities.end(), mobs.begin(), mobs.end());
        entities.insert(entities.end(), buildings.begin(), buildings.end());

        for (const Entity *e: entities) {
            // get all the mobs in the radius of average size
            float sizeAverage = (m_Stats.getSize() + e->getStats().getSize()) / 2;
            float xDif = abs(this->getPosition().x - e->getPosition().x);
            float yDif = abs(this->getPosition().y - e->getPosition().y);

            if (this == e) {
                // ignore this
                continue;
            } else if (xDif < sizeAverage && yDif < sizeAverage) {
                Entity *entityFound = new Entity(e->getStats(), e->getPosition(), e->isNorth());
                collidingEntities.push_back(entityFound);
            }

            // project 1: your code checking for a collision goes here
        }
    }

    return collidingEntities;
}

// handle the collision of entities
void Mob::processCollision(Entity *otherEntity, float deltaTSec, Vec2 moveVec) {

    // PROJECT 1: YOUR COLLISION HANDLING CODE GOES HERE
    float otherEntityMass = 0.f;

    if (otherEntity->getStats().getName() == "Princess Tower" || otherEntity->getStats().getName() == "King Tower") {
        otherEntityMass = 999.f;
    } else {
        otherEntityMass = otherEntity->getStats().getMass();
    }

    if (this->getStats().getMass() > otherEntityMass) {
        // If mass larger than the colliding mob, do nothing
        return;
    } else {
        // if the other Mob is on the route of this mob, process the collision
        Vec2 relativePos = (otherEntity->getPosition() - this->getPosition()) * (-1);
        float sizeAverage = (m_Stats.getSize() + otherEntity->getStats().getSize()) / 2;

        // move a little for for aesthetic purpose
        if (relativePos.x == 0) {
            relativePos.x = (relativePos.y > 0) ? (relativePos.x + sizeAverage) : (relativePos.x - sizeAverage);
        }

        // finalMove = relative position * (distance of two entities - size difference between two mobs)
        Vec2 finalMoveVec = relativePos;
        finalMoveVec.normalize();

        m_Pos += finalMoveVec * abs(relativePos.length() - sizeAverage);
    }
}

std::vector<Entity *> Mob::entityNoticeThisMob() const {
    std::vector<Entity *> awareEntities = std::vector<Entity *>();

    bool enemySide = !this->isNorth();

    // get all the entities include buildings and mobs
    std::vector<Entity *> entities = std::vector<Entity *>();
    std::vector<Entity *> mobs = Game::get().getMobs(enemySide);
    std::vector<Entity *> buildings = Game::get().getBuildings(enemySide);
    entities.insert(entities.end(), mobs.begin(), mobs.end());
    entities.insert(entities.end(), buildings.begin(), buildings.end());

    for (const Entity *e: entities) {
        // get all the mobs in the radius of average size
        float distance = euclidDistance(this->getPosition(), e->getPosition());

        if (distance < e->getStats().getSightRadius()) {
            Entity *entityFound = new Entity(e->getStats(), e->getPosition(), e->isNorth());
            awareEntities.push_back(entityFound);
        }
    }

    return awareEntities;
}

std::vector<Entity *> Mob::enemiesInSight() const {
    std::vector<Entity *> awareEntities = std::vector<Entity *>();

    bool enemySide = !this->isNorth();

    // get all the entities include buildings and mobs
    std::vector<Entity *> entities = std::vector<Entity *>();
    std::vector<Entity *> mobs = Game::get().getMobs(enemySide);
    std::vector<Entity *> buildings = Game::get().getBuildings(enemySide);
    entities.insert(entities.end(), mobs.begin(), mobs.end());
    entities.insert(entities.end(), buildings.begin(), buildings.end());

    for (const Entity *e: entities) {
        // get all the mobs in the radius of average size
        float xDif = abs(this->getPosition().x - e->getPosition().x);
        float yDif = abs(this->getPosition().y - e->getPosition().y);
        float euclidDistance = sqrt(xDif * xDif + yDif * yDif);

        if (euclidDistance < this->getStats().getSightRadius()) {
            Entity *entityFound = new Entity(e->getStats(), e->getPosition(), e->isNorth());
            awareEntities.push_back(entityFound);
        }
    }

    return awareEntities;
}

void Mob::moveAround(std::vector<Entity *> enemySpotted, Vec2 shelterPos, float shelterSize, float moveDist,
                     bool hideGiant, float deltaTSec, Vec2 moveVec) {
    Vec2 finalPos = Vec2(0, 0);
    Vec2 finalMove = Vec2(0, 0);

    for (auto e: enemySpotted) {
        Vec2 relativePos = (e->getPosition() - shelterPos) * -1;

        finalMove += relativePos;
    }

    finalMove.normalize();
    finalPos = m_Pos + finalMove * moveDist;

    Vec2 toShelterVec = shelterPos - finalPos;
    float toShelterDis = toShelterVec.normalize() - shelterSize / 2 - this->getStats().getHideDistance();
    toShelterDis = hideGiant ? toShelterDis : toShelterDis - this->getStats().getSize() * 2;

    if (toShelterDis < 0) {
        toShelterVec = Vec2(0, 0);

    }

    m_Pos = finalPos + toShelterVec * toShelterDis;

    // Move process Collision before move
    std::vector<Entity *> otherEntities = checkCollision();

    // get all the entities that may collide with the mob
    for (Entity *e: otherEntities) {
        if (e) {
            // handle each collision
            processCollision(e, deltaTSec, moveVec);
        }
    }

}

std::vector<Entity *> Mob::seekEntityWithinRadius(float radius, bool side) {
    std::vector<Entity *> returnEntities = std::vector<Entity *>();

    // get all the entities include buildings and mobs
    std::vector<Entity *> entities = std::vector<Entity *>();
    std::vector<Entity *> mobs = Game::get().getMobs(side);
    std::vector<Entity *> buildings = Game::get().getBuildings(side);
    entities.insert(entities.end(), mobs.begin(), mobs.end());
    entities.insert(entities.end(), buildings.begin(), buildings.end());

    for (const Entity *e: entities) {
        // get all the mobs in the radius of average size
        float eucDis = euclidDistance(e->getPosition(), this->getPosition());

        if (eucDis < radius) {
            Entity *entityFound = new Entity(e->getStats(), e->getPosition(), e->isNorth());
            returnEntities.push_back(entityFound);
        }
    }

    return returnEntities;
}

// handle the collision of the edge
bool Mob::handleEdgeCollsion(Vec2 moveVec) {
    // get the future position
    Vec2 futurePos = this->getPosition() + moveVec;

    // since the mobs has a size, add size to consideration
    if (this->isNorth()) {
        futurePos.y += this->getStats().getSize() / 2 + 0.1f;
    } else {
        futurePos.y -= this->getStats().getSize() / 2 + 0.1f;
    }

    // handle the collision of the edge on the x scope
    if (this->getPosition().x < 0) {
        Vec2 finalDisplacement = Vec2(abs((moveVec * abs(this->getPosition().x - 0)).x), 0.f);
        m_Pos += finalDisplacement;
    } else if (this->getPosition().x > GAME_GRID_WIDTH) {
        Vec2 finalDisplacement = Vec2(abs((moveVec * abs(this->getPosition().x - GAME_GRID_WIDTH)).x), 0.f);
        m_Pos -= finalDisplacement;
    }

    // handle the collision of the edge on the y scope
    if (this->getPosition().y < 0) {
        Vec2 finalDisplacement = Vec2(0.f, abs((moveVec * abs(this->getPosition().y - 0)).y));
        m_Pos += finalDisplacement;
    } else if (this->getPosition().y > GAME_GRID_HEIGHT) {
        Vec2 finalDisplacement = Vec2(0.f, abs((moveVec * abs(this->getPosition().y - GAME_GRID_WIDTH)).y));
        m_Pos -= finalDisplacement;
    }

    // handle the situation which entities is in the river
    bool nowInTheBridge = false;
    if (this->getPosition().y > RIVER_TOP_Y && this->getPosition().y < RIVER_BOT_Y) {
        bool nowInLeftBridge = (this->getPosition().x > (LEFT_BRIDGE_CENTER_X - BRIDGE_WIDTH / 2.f))
                               && (this->getPosition().x < (LEFT_BRIDGE_CENTER_X + BRIDGE_WIDTH / 2.f));
        bool nowInRightBridge = (this->getPosition().x > (RIGHT_BRIDGE_CENTER_X - BRIDGE_WIDTH / 2.f))
                                && (this->getPosition().x < (RIGHT_BRIDGE_CENTER_X + BRIDGE_WIDTH / 2.f));

        // if the entity is on the bridge, pass
        if (nowInLeftBridge || nowInRightBridge) {
            nowInTheBridge = true;
        } else {
            // else handle the collision with river
            float xDisPlacement = 0.f;
            float yDisPlacement;

            // handle the collision of mob horizontally on the left half of the map
            if (this->getPosition().x < GAME_GRID_WIDTH / 2) {
                // move the mob to left bridge
                if (this->getPosition().x < LEFT_BRIDGE_CENTER_X - BRIDGE_WIDTH / 2) {
                    xDisPlacement = LEFT_BRIDGE_CENTER_X - this->getPosition().x;
                } else if (this->getPosition().x > LEFT_BRIDGE_CENTER_X + BRIDGE_WIDTH / 2) {
                    xDisPlacement = abs(LEFT_BRIDGE_CENTER_X - this->getPosition().x) * -1.f;
                }
            }
                // handle the collision of mob horizontally on the right half of the map
            else {
                // move the mob to right bridge
                if (this->getPosition().x < RIGHT_BRIDGE_CENTER_X - BRIDGE_WIDTH / 2) {
                    xDisPlacement = RIGHT_BRIDGE_CENTER_X - this->getPosition().x;
                } else if (this->getPosition().x > RIGHT_BRIDGE_CENTER_X + BRIDGE_WIDTH / 2) {
                    xDisPlacement = abs(RIGHT_BRIDGE_CENTER_X - this->getPosition().x) * -1.f;
                }
            }

            // handle the mob on the y axis
            if (this->getPosition().y < GAME_GRID_HEIGHT / 2) {
                yDisPlacement = abs(this->getPosition().y - RIVER_TOP_Y) * -1.f;
            } else {
                yDisPlacement = abs(this->getPosition().y - RIVER_BOT_Y);
            }


            // since the move vector is too big, mutiply a number in order to let it not exceed the speed
            Vec2 move = Vec2(xDisPlacement * 0.2f, yDisPlacement);
            move.normalize();
            move *= this->getStats().getSpeed();
            move *= 0.1f;
            m_Pos += move;
        }
    }

    // handle the situation which the mob is not in the river but will move to the river in the next tick
    if (futurePos.y > RIVER_TOP_Y && futurePos.y < RIVER_BOT_Y) {
        // std::cout << "---in the river\n---";

        // handle the situation when the mob move into the river from north or south
        bool inLeftBridge = (futurePos.x > (LEFT_BRIDGE_CENTER_X - BRIDGE_WIDTH / 2.f))
                            && (futurePos.x < (LEFT_BRIDGE_CENTER_X + BRIDGE_WIDTH / 2.f));
        bool inRightBridge = (futurePos.x > (RIGHT_BRIDGE_CENTER_X - BRIDGE_WIDTH / 2.f))
                             && (futurePos.x < (RIGHT_BRIDGE_CENTER_X + BRIDGE_WIDTH / 2.f));
        if (inLeftBridge || inRightBridge) {
            return false;
        } else {
            // if now on the bridge, just move forward or backward, else just move leftward or rightward
            if (nowInTheBridge) {
                m_Pos += Vec2(0.f, moveVec.y);
            } else {
                m_Pos += Vec2(moveVec.x, 0.f);
            }

            return true;
        }
    } else {
        return false;
    }

}


