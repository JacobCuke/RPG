//
//  Manager.cpp
//  RPG
//
//  Created by Jacob Cuke on 2020/02/19.
//  Copyright © 2020 Jacob Cuke. All rights reserved.
//

#include "Manager.hpp"
#include "Components.hpp"
#include "TextureManager.hpp"
#include "TileMap.hpp"

Entity& Manager::addEntity()
{
    Entity* e = new Entity();
    std::unique_ptr<Entity> uPtr { e };
    entities.emplace_back(std::move(uPtr));

    return *e;
}

void Manager::PositionSystem(TileMap* tilemap)
{
    for (auto& e : entities)
    {
        if (!e->hasComponent<PositionComponent>()) continue;
        
        auto& positionCom = e->getComponent<PositionComponent>();
        
        // Calculations
        // Store original values
        int originalX = positionCom.destX;
        int originalY = positionCom.destY;
        
        // Attempt to move
        positionCom.destX += positionCom.deltaX * 2;
        positionCom.destY += positionCom.deltaY * 2;
        
        // If there's a collision the move fails
        if (CollisionSystem(positionCom, tilemap))
        {
            positionCom.destX = originalX;
            positionCom.destY = originalY;
        }
    }
}

void Manager::DrawSystem()
{
    for (auto& e : entities)
    {
        if (!e->hasComponent<PositionComponent>()) continue;
        if (!e->hasComponent<SpriteComponent>()) continue;
        
        auto& positionCom = e->getComponent<PositionComponent>();
        auto& spriteCom = e->getComponent<SpriteComponent>();
        
        SDL_Texture* objTexture = TextureManager::loadTexture(spriteCom.sourceFile);
        SDL_Rect srcRect, destRect;
        
        srcRect.h = SpriteComponent::SPRITE_HEIGHT;
        srcRect.w = SpriteComponent::SPRITE_WIDTH;
        srcRect.x = spriteCom.srcX;
        srcRect.y = spriteCom.srcY;
        
        destRect.h = SpriteComponent::INGAME_HEIGHT;
        destRect.w = SpriteComponent::INGAME_WIDTH;
        destRect.x = positionCom.destX;
        destRect.y = positionCom.destY;
        
        SDL_RenderCopy(Game::renderer, objTexture, &srcRect, &destRect);
    }
}

void Manager::AnimationSystem()
{
    for (auto& e : entities)
    {
        if (!e->hasComponent<PositionComponent>()) continue;
        if (!e->hasComponent<SpriteComponent>()) continue;
        if (!e->hasComponent<AnimationComponent>()) continue;
        
        auto& positionCom = e->getComponent<PositionComponent>();
        auto& spriteCom = e->getComponent<SpriteComponent>();
        auto& animationCom = e->getComponent<AnimationComponent>();
        
        // If entity is standing still, use the middle sprite
        if (positionCom.deltaX == 0 && positionCom.deltaY == 0)
        {
            spriteCom.srcX = SpriteComponent::SPRITE_WIDTH * 1;
            animationCom.deltaFrame = 1;
            animationCom.frameCount = 0;
        }
        
        // Direction
        if (positionCom.deltaX == 1) // Right
        {
            spriteCom.srcY = SpriteComponent::RIGHT_SPRITE;
        }
        else if (positionCom.deltaX == -1) // Left
        {
            spriteCom.srcY = SpriteComponent::LEFT_SPRITE;
        }
        else if (positionCom.deltaY == 1) // Down
        {
            spriteCom.srcY = SpriteComponent::DOWN_SPRITE;
        }
        else if (positionCom.deltaY == -1) // Up
        {
            spriteCom.srcY = SpriteComponent::UP_SPRITE;
        }
        
        // Animation frame calculation
        if (animationCom.frameCount == 9)
        {
            spriteCom.srcX += (SpriteComponent::SPRITE_WIDTH * animationCom.deltaFrame);
            if (spriteCom.srcX == (SpriteComponent::SPRITE_WIDTH * 2))
            {
                animationCom.deltaFrame = -1;
            }
            else if (spriteCom.srcX == 0)
            {
                animationCom.deltaFrame = 1;
            }
            animationCom.frameCount = 0;
        }
        else
        {
            animationCom.frameCount++;
        }
        
    }
}

void Manager::ControlSystem(int& keyPressed)
{
    for (auto& e : entities)
    {
        if (!e->hasComponent<PositionComponent>()) continue;
        if (!e->hasComponent<ControlComponent>()) continue;
        
        if (!e->getComponent<ControlComponent>().inControl) continue;
        
        auto& positionCom = e->getComponent<PositionComponent>();
        
        // Reset direction
        positionCom.deltaX = positionCom.deltaY = 0;
        
        switch (keyPressed) {
            case SDLK_w:
            case SDLK_UP:
                positionCom.deltaY = -1;
                break;
                
            case SDLK_a:
            case SDLK_LEFT:
                positionCom.deltaX = -1;
                break;
                
            case SDLK_s:
            case SDLK_DOWN:
                positionCom.deltaY = 1;
                break;
                
            case SDLK_d:
            case SDLK_RIGHT:
                positionCom.deltaX = 1;
                break;
                
            default:
                break;
        }
    }
}

bool Manager::CollisionSystem(PositionComponent& positionCom, TileMap* tilemap)
{
    // Check that player is not out of bounds
    if (positionCom.destX < 0) return true;
    if (positionCom.destY < 0) return true;
    
    if (positionCom.destX > Game::GAME_WIDTH - SpriteComponent::INGAME_WIDTH)
    {
        return true;
    }
    
    if (positionCom.destY > Game::GAME_HEIGHT - SpriteComponent::INGAME_HEIGHT)
    {
        return true;
    }
    
    // TODO: Figure out what tiles the player's bottom half are touching
    int xCoord = positionCom.destX;
    int yCoord = positionCom.destY + (SpriteComponent::INGAME_HEIGHT / 2);
    
    int position;
    // Top left corner
    position = getPositionFromCoords(xCoord, yCoord);
    if (hasCollidableAtPosition(position, tilemap->collidableTiles)) return true;
    
    // Top right corner
    position = getPositionFromCoords(xCoord + (SpriteComponent::INGAME_WIDTH - 1), yCoord);
    if (hasCollidableAtPosition(position, tilemap->collidableTiles)) return true;
    
    // Bottom left corner
    position = getPositionFromCoords(xCoord, yCoord + (SpriteComponent::INGAME_WIDTH - 1));
    if (hasCollidableAtPosition(position, tilemap->collidableTiles)) return true;
    
    // Bottom right corner
    position = getPositionFromCoords(xCoord + (SpriteComponent::INGAME_WIDTH - 1), yCoord + (SpriteComponent::INGAME_WIDTH - 1));
    if (hasCollidableAtPosition(position, tilemap->collidableTiles)) return true;
    
    return false;
}

int Manager::getPositionFromCoords(int x, int y)
{
    int positionX = x / Game::TILE_SIZE;
    int positionY = y / Game::TILE_SIZE;
    
//    std::cout << "X: " << positionX << " Y: " << positionY << '\n';
    
    return (positionX + 1) + (positionY * (Game::GAME_WIDTH/Game::TILE_SIZE));
}

bool Manager::hasCollidableAtPosition(int position, std::vector<int> collidableTiles)
{
    std::vector<int> tiles = TileMap::getTilesAtPosition("tilemaps/testlevel.tmx", position);
    
    for (auto i = tiles.begin(); i != tiles.end(); ++i)
    {
        if (std::find(collidableTiles.begin(), collidableTiles.end(), *i) != collidableTiles.end()) return true;
    }
    
    return false;
}
