#include "Game.h"

Game::Game(sf::RenderWindow* window, sf::View view)
{
	background.setOrigin(352.f, 160.f);
	background.setPosition(720.f, 450.f);
	background.setScale(4.f, 4.f);
	backgroundTexture.loadFromFile("Texture/Background.png");
	background.setTexture(backgroundTexture);

	spawnTimerMax = 100.f;

	font.loadFromFile("Font/dogica.ttf");
	enemyHp.setFont(font);
	enemyHp.setFillColor(sf::Color::White);
	enemyHp.setCharacterSize(16);

	maxPlayerHp = 20;
	currentPlayerHp = maxPlayerHp;

	duringWave = false;

	this->window = window;
	this->view = view;
}

void Game::pollEvent()
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1))
		currentSlot = 0;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2))
		currentSlot = 1;
}

void Game::screenUIupdate()
{
	gui.screenUI(view.getCenter(),exp, maxExp, wave, weaponSlot[currentSlot], 
		weaponDamage[currentSlot], playerBaseDamage, currentPlayerHp, maxPlayerHp);
}

void Game::takeItemUpdate()
{
	for (size_t i = 0; i < drop.size(); i++)
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)
			&& drop[i]->getBound().intersects(player.getBound()))
		{
			if (weaponSlot[0] == 0)
			{
				weaponSlot[0] = drop[i]->getType();
				currentSlot = 0;
				weaponDamage[0] = drop[i]->getDamage();
			}
			else if (weaponSlot[1] == 0)
			{
				weaponSlot[1] = drop[i]->getType();
				currentSlot = 1;
				weaponDamage[1] = drop[i]->getDamage();
			}
			else
			{
				weaponSlot[currentSlot] = drop[i]->getType();
				weaponDamage[currentSlot] = drop[i]->getDamage();
			}
			drop.erase(drop.begin() + i);
		}
	}
}

void Game::attackUpdate()
{
	attackTimer = attackCoolDownClock.getElapsedTime().asSeconds();
	enableToAttack = false;
	if (playerWeapon == DAGGER)
		attackTimerMax = 0.5f;
	if (playerWeapon == SWORD)
		attackTimerMax = 1.f;

	if (attackTimer >= attackTimerMax)
	{
		std::cout << "Enable to Attack" << std::endl;
		attackCooldown = true;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		{
			attackCooldown = false;
			attackCoolDownClock.restart();
		}
		if (!attackCooldown)
		{
			enableToAttack = true;
		}
	}
	else
	{
		std::cout << "Unable to Attack" << std::endl;
	}

	std::cout << "Current : ";
	if (playerWeapon == NOTHING)
	{
		std::cout << "Bare hand" << std::endl;
	}
	if (playerWeapon == DAGGER)
	{
		std::cout << "Dagger" << std::endl;
	}
	if (playerWeapon == SWORD)
	{
		std::cout << "Sword" << std::endl;
	}

	std::cout << "Damage : " << weaponDamage[currentSlot] << "+" << playerBaseDamage << std::endl;
	std::cout << std::endl;
}

void Game::playerAttackRange()
{
	weaponHitbox.setHitbox(player.getPos().x + 8,
		player.getPos().y + 8, weaponSlot[currentSlot]);
}

void Game::playerLevelUpdate()
{
	maxExp = 12 * playerLevel;
	playerBaseDamage = 2 + playerLevel;
	maxPlayerHp = 15 + (playerLevel * 5);
	if (exp >= maxExp)
	{
		exp = 0;
		playerLevel++;
	}
}

void Game::waveUpdate()
{
	if (spawnCount == 0)
	{
		spawnTimer += 0.1f;
		if (spawnTimer >= spawnTimerMax)
		{
			spawnTimer = 0;
			wave++;
		}
	}
}

void Game::enemyInit()
{
	spawnTimer = enemySpawningClock.getElapsedTime().asSeconds();
	if (spawnTimer >= 2.f && !duringWave)
	{
		wave++;
		for (size_t i = 0; i < 8; i++)
		{
			enemyType = rand() % 2 + 1;
			enemies.push_back(new Enemy(enemyType, rand() % 600 + 1,
				rand() % 600 + 1, wave));
			enemyLeft++;
		}
		duringWave = true;
	}
	else if (enemyLeft == 0 && duringWave == true)
	{
		duringWave = false;
		enemySpawningClock.restart();
	}


	for (size_t i = 0; i < enemies.size(); i++)
	{
		enemies[i]->update();
		enemies[i]->movement(player.getPos());
	}
}

void Game::enemyUpdate()
{
	for (size_t i = 0; i < enemies.size(); i++)
	{
		if (enemies[i]->getBound().intersects(weaponHitbox.getBound()) &&
			sf::Keyboard::isKeyPressed(sf::Keyboard::Space) &&
			playerWeapon != NOTHING && enableToAttack)
		{
			if (enemies[i]->getHp() > 0)
			{
				enemies[i]->takeDamage(weaponDamage[currentSlot] + playerBaseDamage);
			}
			if (enemies[i]->getHp() <= 0)
			{
				exp += enemies[i]->getEXP();
				if (rand() % 10 + 1 == 2)
					drop.push_back(new ItemDrop(enemyType, wave, enemies[i]->getPos()));
				enemies.erase(enemies.begin() + i);
				currentPlayerHp -= 5;
				killCount++;
				enemyLeft--;
			}
		}
		/*for (size_t j = 0; j < i; j++)
		{
			if(i != j && enemies[i] != nullptr && enemies[j] != nullptr)
				enemies[i]->checkObstruct(enemies[i]->getBound(), enemies[j]->getBound());
		}*/
	}
}

void Game::update()
{
	pollEvent();
	getPlayerPos();
	takeItemUpdate();
	playerAttackRange();
	playerLevelUpdate();
	attackUpdate();
	enemyInit();
	waveUpdate();
	enemyUpdate();
	screenUIupdate();

	player.update();

	playerWeapon = weaponSlot[currentSlot];

	std::cout << "PlayerLevel : " << playerLevel << " | ";
	std::cout << "KillCount : " << killCount << " | ";
	std::cout << "EnemyLeft : " << enemyLeft << std::endl;

}

void Game::render()
{
	window->clear();

	view.setCenter(player.getPos());
	window->setView(view);
	
	window->draw(background);

	for (size_t i = 0; i < drop.size(); i++)
	{
		drop[i]->render(*window);
	}

	player.render(*window);
	weaponHitbox.render(*window);

	for (size_t i = 0; i < enemies.size(); i++)
	{
		enemies[i]->render(*window);
		gui.enemyUI(enemies[i]->getHp(), enemies[i]->getMaxHp(),
			enemies[i]->getPos(), enemies[i]->getSize(), *window);
	}

	gui.render(*window);

	window->display();
}