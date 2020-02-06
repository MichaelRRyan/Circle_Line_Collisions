// Author: Dr Noel O'Hara
// Top down game starter
#ifdef _DEBUG 
#pragma comment(lib,"sfml-graphics-d.lib") 
#pragma comment(lib,"sfml-audio-d.lib") 
#pragma comment(lib,"sfml-system-d.lib") 
#pragma comment(lib,"sfml-window-d.lib") 
#pragma comment(lib,"sfml-network-d.lib") 
#else 
#pragma comment(lib,"sfml-graphics.lib") 
#pragma comment(lib,"sfml-audio.lib") 
#pragma comment(lib,"sfml-system.lib") 
#pragma comment(lib,"sfml-window.lib") 
#pragma comment(lib,"sfml-network.lib") 
#endif 

#include <SFML/Graphics.hpp>
// I use a couple of h files from thor library.
//https://github.com/Bromeon/Thor
#include "VectorAlgebra2D.h"


#include <iostream>
#include <stdlib.h> 
#include <time.h> 
#include <list>
enum  PlayerState { ready, drawingLine, Moving };
class Game
{
public:
	//create Window
	sf::RenderWindow window;
	sf::View view;
	float randomNum = 0;

	sf::Font m_font;
	sf::Text m_scoreText;

	sf::CircleShape m_ball;
	
	sf::VertexArray m_lines{ sf::Lines };

	PlayerState playerState = ready;

	sf::Vector2f velocity = { 0,0 };
	float maxSpeed = 400;
	int m_score = 0;
	float m_gravity{ 3.0f };
	float m_elasticity{ 1.0f };

	Game()
	{
		window.create(sf::VideoMode(800, 600), "Untitled Physics Games");
	}
	
	void init()
	{

		view = window.getDefaultView();

		m_ball.setRadius(20);
		m_ball.setPosition(160, 500);
		m_ball.setOrigin(sf::Vector2f(20, 20));
		m_ball.setFillColor(sf::Color::Blue);

		if (!m_font.loadFromFile("arial.ttf"))
		{
			std::cout << "Error loading font file" << std::endl;
		}

		m_scoreText.setFont(m_font);
		m_scoreText.setString("Score: " + std::to_string(m_score));

		m_lines.append({ {400.0f, 300.0f}, sf::Color::White }); // A
		m_lines.append({ {500.0f, 300.0f}, sf::Color::White }); // B

		m_lines.append({ {500.0f, 400.0f}, sf::Color::Red }); // C
		m_lines.append({ m_lines[0].position, sf::Color::Red }); // A

		m_lines.append({ m_lines[1].position, sf::Color::Green }); // A
		m_lines.append({ m_lines[2].position, sf::Color::Green }); // C
	}

	void run()
	{
	
		sf::Time timePerFrame = sf::seconds(1.0f / 60.0f);


		sf::Time timeSinceLastUpdate = sf::Time::Zero;

		
		sf::Clock clock;

		clock.restart();

		while (window.isOpen())
		{
			
			sf::Event event;
			while (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					window.close();
			}

			
			timeSinceLastUpdate += clock.restart();

			

			if (timeSinceLastUpdate > timePerFrame)
			{
				if (playerState == ready)
				{
					if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
					{
						playerState = drawingLine;
					}
				}
				if (playerState == drawingLine)
				{
					if (!sf::Mouse::isButtonPressed(sf::Mouse::Left))
					{
						sf::Vector2f localPosition = (sf::Vector2f)sf::Mouse::getPosition(window);
						m_ball.setPosition(localPosition);
						velocity = { 0.0f, 0.0f };

						playerState = ready;
					}
				}

				// Line - Ball collisions
				handleLineCollisions();

				// Wall Collisions
				handleWallCollisions();

				velocity.y += m_gravity;
				m_ball.move(velocity.x * timeSinceLastUpdate.asSeconds(), velocity.y * timeSinceLastUpdate.asSeconds());

				// Rotate the line
				float rotAngle = 0.2f;

				sf::Vector2f lineLength = m_lines[1].position - m_lines[0].position;
				thor::rotate(lineLength, rotAngle);
				m_lines[1].position = m_lines[0].position + lineLength;

				lineLength = m_lines[2].position - m_lines[3].position;
				thor::rotate(lineLength, rotAngle);
				m_lines[2].position = m_lines[0].position + lineLength;

				
				m_lines[4].position = m_lines[1].position;
				m_lines[5].position = m_lines[2].position;

				draw();
				
				timeSinceLastUpdate = sf::Time::Zero;
			}
		}
	}

	void handleLineCollisions()
	{
		for (int i = 0; i + 1 < m_lines.getVertexCount(); i += 2)
		{
			sf::Vector2f lineVectorLength = m_lines[i + 1].position - m_lines[i].position; // Move p1 – p2 to 0, 0 where p1 = 0, 0
			sf::Vector2f ballPosition = m_ball.getPosition() - m_lines[i].position; // Move circle by the same amount

			float angle = -thor::polarAngle(lineVectorLength); // Angle = –thor::polarAngle(p2)

			thor::rotate(lineVectorLength, angle); // Rotate the line by –thor::polarAngle(p2)

			sf::Vector2f tempVelocity = ballPosition + velocity; //	Tempvel = circle center + velocity

			thor::rotate(ballPosition, angle); // Rotate the circle center point  by –thor::polarAngle(p2)

			thor::rotate(tempVelocity, angle); // Rotate the tempVel by –thor::polarAngle(p2)

			tempVelocity -= ballPosition; // Velocity = tempvel - circle center

			//	Find if rectangle b intersects with line 0, p2
			if (tempVelocity.y >= 0)
			{
				if (ballPosition.x + m_ball.getRadius() >= 0.0f
					&& ballPosition.x - m_ball.getRadius() <= lineVectorLength.x)
				{
					if (ballPosition.y + m_ball.getRadius() >= 0.0f
						&& ballPosition.y - m_ball.getRadius() <= 0.0f)
					{
						ballPosition.y = -m_ball.getRadius(); //	Move c.y to –radius

						tempVelocity.y = -tempVelocity.y; // velocity.y = -velocity.y

						tempVelocity += ballPosition; // Add center of circle to velocity

						//	Rotate center of circle by –angle
						thor::rotate(ballPosition, -angle);

						thor::rotate(tempVelocity, -angle); //	Rotate velocity by –angle

						tempVelocity -= ballPosition; //	Move velocity back to 0, 0

						ballPosition += m_lines[i].position; // Move circle same amount that line was moved

						// Set ball position and velocity
						m_ball.setPosition(ballPosition);
						velocity = tempVelocity;

						std::cout << "Collision" << std::endl;
					}
				}
			}
		}
	}

	void handleWallCollisions()
	{
		if (m_ball.getPosition().y > 600 - m_ball.getRadius())
		{
			velocity.y = -velocity.y * m_elasticity;
			m_ball.setPosition(m_ball.getPosition().x, 600.0f - m_ball.getRadius());
		}

		if (m_ball.getPosition().y < m_ball.getRadius())
		{
			velocity.y = -velocity.y * m_elasticity;
			m_ball.setPosition(m_ball.getPosition().x, m_ball.getRadius());
		}

		if (m_ball.getPosition().x > 800 - m_ball.getRadius())
		{
			velocity.x = -velocity.x * m_elasticity;
			m_ball.setPosition(800.0f - m_ball.getRadius(), m_ball.getPosition().y);
		}

		if (m_ball.getPosition().x < m_ball.getRadius())
		{
			velocity.x = -velocity.x * m_elasticity;
			m_ball.setPosition(m_ball.getRadius(), m_ball.getPosition().y);
		}
	}

	void draw()
	{
		window.clear();
		if (playerState == drawingLine)
		{
			sf::Vector2i localPosition = sf::Mouse::getPosition(window);
			sf::Vertex line[] =
			{
				sf::Vertex(sf::Vector2f(localPosition.x, localPosition.y)),
				sf::Vertex(sf::Vector2f(m_ball.getPosition().x, m_ball.getPosition().y))
			};

			window.draw(line, 2, sf::Lines);
		}

		window.draw(m_ball);

		window.draw(m_lines);

		window.display();
	}
};


int main()
{


	Game game;
	

	game.init();

	game.run();



	return 0;
}