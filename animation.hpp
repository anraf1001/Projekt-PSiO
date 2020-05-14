#ifndef PROJEKT_ANIMATION_HPP
#define PROJEKT_ANIMATION_HPP

#include <SFML/Graphics.hpp>

class Animation {
public:
    sf::IntRect uvRect;

    Animation(sf::Texture *texture, sf::Vector2u imageCount, float switchTime);

    void update(float deltaTime, unsigned int row = 0);

private:
    sf::Vector2u m_imageCount;
    sf::Vector2u m_currentImage;

    float m_totalTime;
    float m_switchTime;
};


#endif //PROJEKT_ANIMATION_HPP
