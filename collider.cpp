#include "collider.hpp"

Collider::Collider(sf::RectangleShape &body) : body(body) {}

void Collider::Move(float dx, float dy) {
    body.move(dx, dy);
}

bool Collider::checkCollision(Collider &other, sf::Vector2f &direction) {
    sf::Vector2f otherPosition = other.getPosition();
    sf::Vector2f otherHalfSize = other.getHalfSize();
    sf::Vector2f thisPosition = getPosition();
    sf::Vector2f thisHalfSize = getHalfSize();

    float deltaX = otherPosition.x - thisPosition.x;
    float deltaY = otherPosition.y - thisPosition.y;

    float intersectX = std::abs(deltaX) - (otherHalfSize.x + thisHalfSize.x);
    float intersectY = std::abs(deltaY) - (otherHalfSize.y + thisHalfSize.y);

    if(intersectX < 0.0f && intersectY < 0.0f){
        if(intersectX > intersectY){
            if(deltaX > 0.0f){
                Move(0.0f, 0.0f);
                other.Move(-intersectX, 0.0f);

                direction.x = 1.0f;
                direction.y = 0.0f;
            }
            else{
                Move(0.0f, 0.0f);
                other.Move(intersectX, 0.0f);

                direction.x = -1.0f;
                direction.y = 0.0f;
            }
        }
        else{
            if(deltaY > 0.0f){
                Move(0.0f, 0.0f);
                other.Move(0.0f, -intersectY);

                direction.x = 0.0f;
                direction.y = 1.0f;
            }
            else{
                Move(0.0f, 0.0f);
                other.Move(0.0f, intersectY);

                direction.x = 0.0f;
                direction.y = -1.0f;
            }
        }

        return true;
    }

    return false;
}

sf::Vector2f Collider::getPosition() {
    return body.getPosition();
}

sf::Vector2f Collider::getHalfSize() {
    return body.getSize() / 2.0f;
}
