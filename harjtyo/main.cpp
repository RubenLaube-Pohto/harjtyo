#include <vector>
#include <SFML\Graphics.hpp>
#include <SFML\Audio.hpp>
#include <iostream>
#include "playerCharacter.h"
#include "enemy.h"
#include "math.h"
#include "constants.h"
#include "highscoresmanager.h"
#include "healthmeter.h"
#include "explosion.h"

float xSpeed, ySpeed;
bool gameover = false;
int killCount = 0;

PlayerCharacter* player = NULL;
HealthMeter* healthMeter = NULL;
sf::View view;
sf::Sprite crosshair;
sf::RectangleShape background;
std::vector<Enemy*> enemies;
std::vector<Bullet> bullets;
std::vector<Bullet> enemyBullets;
std::vector<Explosion> explosions;

sf::Sound gameoverSound;
sf::Sound playerDeathSound;
sf::Sound enemyDeathSound;
sf::Sound playerShootSound;
sf::Sound enemyShootSound;

sf::Texture enemyTexture;
sf::Texture explosionTexture;

HighscoresManager* highscores = NULL;

int fps = 0;
sf::Text framerateText;
sf::Clock fpsTimer;

void update(sf::RenderWindow&);
void draw(sf::RenderWindow&);
void handleInput();
void cleanUp();
void initializeEnemies(sf::Texture&);
void createNewEnemy(const sf::Texture&);

int main() {
	srand((unsigned int)time(NULL));

	// Create framerate display for debug
	sf::Font fontArial;
	fontArial.loadFromFile(FONT_FILE);
	framerateText = sf::Text();
	framerateText.setFont(fontArial);
	framerateText.setColor(sf::Color::Red);

	// Load window
	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGTH), TITLE);
	window.setFramerateLimit(62);
	window.setMouseCursorVisible(false);

	// Load textures
	sf::Texture backgroundTexture;
	sf::Texture playerTexture;
	sf::Texture crosshairTexture;

	try {
		if (!backgroundTexture.loadFromFile(BG_TEXTURE_FILE))
			throw string(BG_TEXTURE_FILE);
		backgroundTexture.setRepeated(true);
	}
	catch (string e) {
		std::cout << "Failed to load " << e << std::endl;
	}
	
	try {
		if (!playerTexture.loadFromFile(PLAYER_TEXTURE_FILE))
			throw string(PLAYER_TEXTURE_FILE);
	}
	catch (string e) {
		std::cout << "Failed to load " << e << std::endl;
	}

	try {
		if (!crosshairTexture.loadFromFile(CROSSHAIR_TEXTURE_FILE))
			throw string(CROSSHAIR_TEXTURE_FILE);
	}
	catch (string e) {
		std::cout << "Failed to load " << e << std::endl;
	}

	try {
		if (!enemyTexture.loadFromFile(ENEMY_TEXTURE_FILE))
			throw string(ENEMY_TEXTURE_FILE);
	}
	catch (string e) {
		std::cout << "Failed to load " << e << std::endl;
	}

	try {
		if (!explosionTexture.loadFromFile(EXPLOSION_TEXTURE_FILE))
			throw string(EXPLOSION_TEXTURE_FILE);
	}
	catch (string e) {
		std::cout << "Failed to load " << e << std::endl;
	}

	// Load background
	background.setTexture(&backgroundTexture);
	background.setTextureRect(sf::IntRect(0, 0, (int)GROUND_WIDTH, (int)GROUND_HEIGTH));
	background.setSize(sf::Vector2f(GROUND_WIDTH, GROUND_HEIGTH));

	// Load player
	player = new PlayerCharacter(playerTexture);
	player->setPosition(GROUND_WIDTH / 2, GROUND_HEIGTH / 2);
	player->setHitbox(player->getGlobalBounds());

	// Initialize camera and moving speeds
	view = sf::View(player->getPosition(), sf::Vector2f((float)WIDTH, (float)HEIGTH));
	xSpeed = 0.0f;
	ySpeed = 0.0f;

	// Load enemies
	initializeEnemies(enemyTexture);

	// Load healthMeter
	healthMeter = new HealthMeter();
	healthMeter->setSize(HEALTHMETER_SIZE);
	healthMeter->setPosition(view.getCenter());
	healthMeter->move(HEALTHMETER_OFFSET);
	
	// Load crosshair cursor
	crosshair.setTexture(crosshairTexture);
	crosshair.setOrigin(crosshairTexture.getSize().x * 0.5f, crosshairTexture.getSize().y * 0.5f);
	crosshair.setScale(0.2f, 0.2f);

	// Load music
	sf::Music musicPlayer;
	musicPlayer.openFromFile(BG_MUSIC);
	musicPlayer.setLoop(true);
	musicPlayer.setVolume(30.0f);
	musicPlayer.play();

	// Load sound effects
	sf::SoundBuffer sbGameoverSound;
	sf::SoundBuffer sbPlayerDeathSound;
	sf::SoundBuffer sbEnemyDeathSound;
	sf::SoundBuffer sbPlayerShootSound;
	sf::SoundBuffer sbEnenmyShootSound;

	sbGameoverSound.loadFromFile(GAMEOVER_AUDIO);
	sbPlayerDeathSound.loadFromFile(PLAYER_DEATH_AUDIO);
	sbEnemyDeathSound.loadFromFile(ENEMY_DEATH_AUDIO);
	sbPlayerShootSound.loadFromFile(PLAYER_SHOOT_AUDIO);
	sbEnenmyShootSound.loadFromFile(ENEMY_SHOOT_AUDIO);

	gameoverSound = sf::Sound(sbGameoverSound);
	playerDeathSound = sf::Sound(sbPlayerDeathSound);
	enemyDeathSound = sf::Sound(sbEnemyDeathSound);
	playerShootSound = sf::Sound(sbPlayerShootSound);
	enemyShootSound = sf::Sound(sbEnenmyShootSound);

	enemyShootSound.setVolume(50.0f);

	// Load highscores manager
	highscores = new HighscoresManager(HIGHSCORES_FILE);
	highscores->setFont(fontArial);

	// Game loop
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
			else if (event.type == sf::Event::KeyPressed) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
					window.close();
				}
				if (gameover && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
					gameover = false;

					delete player;
					player = new PlayerCharacter(playerTexture);
					player->setPosition(GROUND_WIDTH / 2, GROUND_HEIGTH / 2);
					view.setCenter(player->getPosition());

					delete healthMeter;
					healthMeter = new HealthMeter();
					healthMeter->setSize(HEALTHMETER_SIZE);
					healthMeter->setPosition(view.getCenter());
					healthMeter->move(HEALTHMETER_OFFSET);

					while (enemies.size()) {
						delete enemies.back();
						enemies.back() = NULL;
						enemies.pop_back();

					}
					initializeEnemies(enemyTexture);

					bullets.clear();
					enemyBullets.clear();
				}
			}
		}

		if (!gameover) {
			handleInput();
			update(window);
			draw(window);
			window.display();
		}
		else {
			highscores->setPosition(view.getCenter());
			draw(window);
			window.draw(*highscores);
			window.display();
		}
	}

	cleanUp();

	return 0;
}

void update(sf::RenderWindow& window) {
	// Update player
	if (xSpeed != 0.0f || ySpeed != 0.0f) {
		// Update player position
		sf::Vector2f playerPos = player->getPosition();
		sf::Vector2f playerSize = player->getSize();

		player->animate();

		bool crossLeft = (playerPos.x - playerSize.x * 0.5f + xSpeed) < 0.0f;
		bool crossRight = (playerPos.x + playerSize.x * 0.5f + xSpeed) > GROUND_WIDTH;
		bool crossTop = (playerPos.y - playerSize.y * 0.5f + ySpeed) < 0.0f;
		bool crossBottom = (playerPos.y + playerSize.y * 0.5f + ySpeed) > GROUND_HEIGTH;

		if (crossTop && crossLeft)
			player->setPosition(playerSize.x * 0.5f, playerSize.y * 0.5f);
		else if (crossTop && crossRight)
			player->setPosition(GROUND_WIDTH - playerSize.x * 0.5f, playerSize.y * 0.5f);
		else if (crossBottom && crossLeft)
			player->setPosition(playerSize.x * 0.5f, GROUND_HEIGTH - playerSize.y * 0.5f);
		else if (crossBottom && crossRight)
			player->setPosition(GROUND_WIDTH - playerSize.x * 0.5f, GROUND_HEIGTH - playerSize.y * 0.5f);
		else if (crossLeft) {
			player->setPosition(playerSize.x * 0.5f, playerPos.y);
			player->move(0.0f, ySpeed);
		}
		else if (crossRight) {
			player->setPosition(GROUND_WIDTH - playerSize.x * 0.5f, playerPos.y);
			player->move(0.0f, ySpeed);
		}
		else if (crossTop) {
			player->setPosition(playerPos.x, playerSize.y * 0.5f);
			player->move(xSpeed, 0.0f);
		}
		else if (crossBottom) {
			player->setPosition(playerPos.x, GROUND_HEIGTH - playerSize.y * 0.5f);
			player->move(xSpeed, 0.0f);
		}
		else
			player->move(xSpeed, ySpeed);

		player->setHitbox(player->getGlobalBounds());

		// Update view position
		playerPos = player->getPosition();

		crossLeft = playerPos.x < WIDTH * 0.5f - CAMERA_MAX_OFFSET;
		crossRight = playerPos.x > GROUND_WIDTH - WIDTH * 0.5f + CAMERA_MAX_OFFSET;
		crossTop = playerPos.y < HEIGTH * 0.5f - CAMERA_MAX_OFFSET;
		crossBottom = playerPos.y > GROUND_HEIGTH - HEIGTH * 0.5f + CAMERA_MAX_OFFSET;

		if (crossTop && crossLeft)
			view.setCenter(WIDTH * 0.5f - CAMERA_MAX_OFFSET, HEIGTH * 0.5f - CAMERA_MAX_OFFSET);
		else if (crossTop && crossRight)
			view.setCenter(GROUND_WIDTH - WIDTH * 0.5f + CAMERA_MAX_OFFSET, HEIGTH * 0.5f - CAMERA_MAX_OFFSET);
		else if (crossBottom && crossLeft)
			view.setCenter(WIDTH * 0.5f - CAMERA_MAX_OFFSET, GROUND_HEIGTH - HEIGTH * 0.5f + CAMERA_MAX_OFFSET);
		else if (crossBottom && crossRight)
			view.setCenter(GROUND_WIDTH - WIDTH * 0.5f + CAMERA_MAX_OFFSET, GROUND_HEIGTH - HEIGTH * 0.5f + CAMERA_MAX_OFFSET);
		else if (crossLeft) {
			view.setCenter(WIDTH * 0.5f - CAMERA_MAX_OFFSET, playerPos.y);
			view.move(0.0f, ySpeed);
		}
		else if (crossRight) {
			view.setCenter(GROUND_WIDTH - WIDTH * 0.5f + CAMERA_MAX_OFFSET, playerPos.y);
			view.move(0.0f, ySpeed);
		}
		else if (crossTop) {
			view.setCenter(playerPos.x, HEIGTH * 0.5f - CAMERA_MAX_OFFSET);
			view.move(xSpeed, 0.0f);
		}
		else if (crossBottom) {
			view.setCenter(playerPos.x, GROUND_HEIGTH - HEIGTH * 0.5f + CAMERA_MAX_OFFSET);
			view.move(xSpeed, 0.0f);
		}
		else
			view.setCenter(playerPos);

		// Update healthMeter pos
		healthMeter->setPosition(view.getCenter());
		healthMeter->move(HEALTHMETER_OFFSET);

		// Reset speeds
		xSpeed = 0.0f;
		ySpeed = 0.0f;
	}
	player->updateTimer();

	// Update cursor position
	sf::Vector2i mousePosition;
	mousePosition = sf::Mouse::getPosition(window);
	crosshair.setPosition(view.getCenter() + sf::Vector2f(mousePosition) - sf::Vector2f(WIDTH / 2.0f, HEIGTH / 2.0f));

	// Update bullets
	for (unsigned int i = 0; i < bullets.size(); i++) {
		bullets.at(i).travel();
		if (!bullets.at(i).isAlive()) {
			bullets.erase(bullets.begin() + i);
		}
	}
	for (unsigned int i = 0; i < enemyBullets.size(); i++) {
		enemyBullets.at(i).travel();
		if (!enemyBullets.at(i).isAlive()) {
			enemyBullets.erase(enemyBullets.begin() + i);
		}
	}

	// Update enemies
	for (unsigned int i = 0; i < enemies.size(); ++i) {
		Enemy* enemy = NULL;
		enemy = enemies.at(i);
		enemy->update();
		if (enemy->isAlive()) {
			if (Math::vector2fLength(player->getPosition() - enemy->getPosition()) < ENEMY_SHOOTING_DISTANCE) {
				if (enemy->isReadyToFire()) {
					sf::Vector2f bv = player->getPosition() - enemy->getPosition();
					Bullet b(Math::vector2fUnit(bv), enemy->getPosition(), WIDTH * 0.5f);
					b.setFillColor(sf::Color::Magenta);
					b.setSpeed(ENEMY_BULLET_SPEED);
					enemyBullets.push_back(b);
					enemy->setReadyToFire(false);
					enemyShootSound.play();
				}
			}
			else {
				sf::Vector2f nextStep = Math::vector2fUnit(player->getPosition() - enemy->getPosition());
				enemy->move(nextStep);
				enemy->setHitbox(enemy->getGlobalBounds());
			}
		}
		else {
			enemyDeathSound.play();

			Explosion exp(explosionTexture);
			exp.setOrigin(enemy->getOrigin());
			exp.setPosition(enemy->getPosition());
			explosions.push_back(exp);

			createNewEnemy(enemyTexture);

			delete enemies.at(i);
			enemies.at(i) = NULL;
			enemies.erase(enemies.begin() + i);

			++killCount;
		}
	}

	// Update displayed framerate
	if (DEBUG) {
		framerateText.setPosition(view.getCenter() + sf::Vector2f(-WIDTH * 0.5f + 20.0f, -HEIGTH * 0.5f + 20.0f));
		if (fpsTimer.getElapsedTime().asSeconds() <= 1.0f) {
			fps++;
		}
		else {
			framerateText.setString(std::to_string(fps));
			fpsTimer.restart();
			fps = 0;
		}
	}

	// Check hits on enemies
	for (unsigned int i = 0; i < enemies.size(); ++i) {
		Enemy* enemy = NULL;
		enemy = enemies.at(i);
		for (int j = bullets.size() - 1; j >= 0; --j) {
			if (enemy->checkHit(bullets.at(j).getPosition())) {
				bullets.erase(bullets.begin() + j);
			}
		}
	}

	// Check hits on player
	for (int i = enemyBullets.size() - 1; i >= 0; --i) {
		if (player->checkHit(enemyBullets.at(i).getPosition())) {
			// Gameover condition and handling
			if (!player->isAlive()) {
				playerDeathSound.play();
				gameoverSound.play();
				gameover = true;

				if (killCount > highscores->getWorstScore()) {
					// I have no idea how to implement text input and I'm running out of time.
					// "Player" is used as default name.
					highscores->newScore("Player", killCount);
				}
			}
			enemyBullets.erase(enemyBullets.begin() + i);
			healthMeter->setFillWidth((float)player->getHealthPoints() / (float)PLAYER_HEALTH);
		}
	}

	// Update explosions
	for (int i = explosions.size() - 1; i >= 0; --i) {
		if (explosions.at(i).isDone()) {
			explosions.erase(explosions.begin() + i);
		}
	}
	for (unsigned int i = 0; i < explosions.size(); ++i) {
		explosions.at(i).animate();
	}
}

void handleInput() {
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) &&
		sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
		xSpeed = -MOVE_SPEED * cos(Math::PI * 0.25f);
		ySpeed = -MOVE_SPEED * sin(Math::PI * 0.25f);
		player->setDirection(Direction::NORTHWEST);
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) &&
		sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
		xSpeed = MOVE_SPEED * cos(Math::PI * 0.25f);
		ySpeed = -MOVE_SPEED * sin(Math::PI * 0.25f);
		player->setDirection(Direction::NORTHEAST);
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) &&
		sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
		xSpeed = -MOVE_SPEED * cos(Math::PI * 0.25f);
		ySpeed = MOVE_SPEED * sin(Math::PI * 0.25f);
		player->setDirection(Direction::SOUTHWEST);
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) &&
		sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
		xSpeed = MOVE_SPEED * cos(Math::PI * 0.25f);
		ySpeed = MOVE_SPEED * sin(Math::PI * 0.25f);
		player->setDirection(Direction::SOUTHEAST);
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
		ySpeed = -MOVE_SPEED;
		player->setDirection(Direction::NORTH);
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
		xSpeed = -MOVE_SPEED;
		player->setDirection(Direction::WEST);
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
		ySpeed = MOVE_SPEED;
		player->setDirection(Direction::SOUTH);
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
		xSpeed = MOVE_SPEED;
		player->setDirection(Direction::EAST);
	}

	if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
		if (player->isReadyToFire()) {
			sf::Vector2f bv = crosshair.getPosition() - player->getPosition();
			bullets.push_back(Bullet(Math::vector2fUnit(bv), player->getPosition(), WIDTH * 0.5f));
			player->setReadyToFire(false);
			playerShootSound.play();
		}
	}
}

void cleanUp() {
	delete player;
	player = NULL;

	delete highscores;
	highscores = NULL;

	delete healthMeter;
	healthMeter = NULL;

	while (enemies.size()) {
		delete enemies.back();
		enemies.back() = NULL;
		enemies.pop_back();
	}
}

void draw(sf::RenderWindow& window) {
	window.clear();
	window.setView(view);
	window.draw(background);
	for (unsigned int i = 0; i < enemyBullets.size(); i++) {
		window.draw(enemyBullets.at(i));
	}
	for (unsigned int i = 0; i < bullets.size(); i++) {
		window.draw(bullets.at(i));
	}
	for (unsigned int i = 0; i < enemies.size(); i++) {
		window.draw(*(enemies.at(i)));
	}
	for (unsigned int i = 0; i < explosions.size(); i++) {
		window.draw(explosions.at(i));
	}
	window.draw(*player);
	healthMeter->draw(window);
	window.draw(crosshair);
	if (DEBUG) {
		window.draw(framerateText);
	}
}

void initializeEnemies(sf::Texture& texture) {
	float screenX = view.getCenter().x - 0.5f * WIDTH;
	float screenY = view.getCenter().y - 0.5f * HEIGTH;
	float screenWidth = view.getSize().x;
	float screenHeight = view.getSize().y;

	sf::FloatRect screen(screenX, screenY, screenWidth, screenHeight);

	unsigned int i = 0;
	while (i < ENEMIES_ON_FIELD) {
		int randX = rand() % (int)GROUND_WIDTH + 1;
		int randY = rand() % (int)GROUND_HEIGTH + 1;
		sf::Vector2f pos((float)randX, (float)randY);
		if (screen.contains(pos)) {
			continue;
		}
		else {
			Enemy* enemy = new Enemy(texture);
			enemy->setPosition(pos);
			enemy->setHitbox(enemy->getGlobalBounds());
			enemies.push_back(enemy);
			++i;
		}
	}
}

void createNewEnemy(const sf::Texture& texture) {
	float screenX = view.getCenter().x - 0.5f * WIDTH;
	float screenY = view.getCenter().y - 0.5f * HEIGTH;
	float screenWidth = view.getSize().x;
	float screenHeight = view.getSize().y;

	sf::FloatRect screen(screenX, screenY, screenWidth, screenHeight);

	bool done = false;
	while (!done) {
		int randX = rand() % (int)GROUND_WIDTH + 1;
		int randY = rand() % (int)GROUND_HEIGTH + 1;
		sf::Vector2f pos((float)randX, (float)randY);
		if (screen.contains(pos)) {
			continue;
		}
		else {
			Enemy* enemy = new Enemy(texture);
			enemy->setPosition(pos);
			enemy->setHitbox(enemy->getGlobalBounds());
			enemies.push_back(enemy);
			done = true;
		}
	}
}