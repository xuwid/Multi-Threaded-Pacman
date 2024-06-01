#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <cmath>
#include <iostream>
#include <thread>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <pthread.h>
#include <array>
#include <stdio.h> 
#include <stdlib.h> 
#include <semaphore.h>
#include <unistd.h>
using namespace std;

const int NUM_RESOURCES = 1;
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int PACMAN_SIZE = 6;
const float PACMAN_SPEED = 1.0f;
const int COIN_SIZE = 1;
const int NUM_GHOSTS = 4;
const float GHOST_SPEED = 1.5f;
const int MAX_LIVES = 3;

bool ghostpowerupenabled=false;

sem_t keyexit[4],leaver,ui,drawingmap;

sem_t gameLoopSemaphore,movepacman,energizer;
bool ghostsLeftHouse[NUM_GHOSTS] = {false};
bool energizereaten= false;

sem_t keySemaphore;
sem_t permitSemaphore;
pthread_mutex_t keyMutex;
pthread_mutex_t permitMutex;
sem_t allDoneSemaphore;
int sleepDuration[NUM_GHOSTS]= {10,10,10,10};
pthread_mutex_t ghostfastMutex;
pthread_t threadEnergizer;


enum Cell
{
	Door,
	Empty,
	Energizer,
	Pellet,
	Wall,
	Ghost,GhostHouse,Powerup
};

struct Position
{
	short x;
	short y;

	bool operator==(const Position& i_position)
	{
		return this->x == i_position.x && this->y == i_position.y;
	}
};


pthread_mutex_t pacmanMutex;
pthread_mutex_t ghostMutex;
pthread_mutex_t scoreMutex;
pthread_mutex_t livesMutex;

int score = 0;
int lives = MAX_LIVES;
void* leavehouse(void* arg);
void* leavehouse1(void* arg);

sf::Texture pacmanTexture;
std::vector<sf::Texture> ghostTextures;

sf::Vector2f pacmanPosition(WINDOW_WIDTH / 4 + 22, WINDOW_HEIGHT / 2 + 25);
std::vector<sf::Vector2f> ghostPositions(NUM_GHOSTS);
std::vector<sf::Vector2f> initialGhostPositions(NUM_GHOSTS);


const int MAP_WIDTH = 21; 
const int MAP_HEIGHT = 21;
const int CELL_SIZE = 21;

std::array<std::string, MAP_HEIGHT> map_sketch = {
    " ################### ",
    " #........#........# ",
    " #o##.###.#.###.##o# ",
    " #.................# ",
    " #.##.#.#####.#.##.# ",
    " #....#...#...#....# ",
    " ####.### # ###.#### ",
    "    #.#   0   #.#    ",
    "#####.# ##?## #.#####",
    "     !  #???#  !     ",
    "#####.# ##### #.#####",
    "    #.#       #.#    ",
    " ####.# ##### #.#### ",
    " #........#........# ",
    " #.##.###.#.###.##.# ",
    " #o.#.....P.....#.o# ",
    " ##.#.#.#####.#.#.## ",
    " #....#...#...#....# ",
    " #.######.#.######.# ",
    " #.................# ",
    " ################### "
};

std::array<std::array<Cell, MAP_WIDTH>, MAP_HEIGHT> map{};


sf::RenderWindow window(sf::VideoMode(MAP_WIDTH * CELL_SIZE, MAP_HEIGHT * CELL_SIZE+50), "Pac-Man");

void initializeMap() {
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            char cellChar = map_sketch[y][x];
            switch (cellChar) {
                case '#':
                    ::map[y][x] = Wall;
                    break;
                case '?':
                    ::map[y][x] = GhostHouse;
                    break;
                case '.':
                    ::map[y][x] = Pellet;
                    break;
                case 'o':
                    ::map[y][x] = Energizer;
                    break;
                case 'P':
                    ::map[y][x] = Empty;
                    break;
                case '!':
                    ::map[y][x] = Powerup;
                    break;    
                default:
                    ::map[y][x] = Empty; 
                    break;
            }
        }
    }
}

void drawMap(sf::RenderWindow& window) {
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            switch (::map[y][x]) {
                case Wall: {
                    sf::RectangleShape wallRect(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                    wallRect.setFillColor(sf::Color(128, 0, 0, 255));
                    wallRect.setPosition(x * CELL_SIZE, y * CELL_SIZE);
                    window.draw(wallRect);
                    break;
                }
                case GhostHouse: {
                    sf::RectangleShape wallRect(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                    wallRect.setFillColor(sf::Color::Red);
                    wallRect.setPosition(x * CELL_SIZE, y * CELL_SIZE);
                    window.draw(wallRect);
                    break;
                }
                case Pellet: {
                    sf::CircleShape pelletCircle(CELL_SIZE / 7.0f); 
                    pelletCircle.setFillColor(sf::Color(255, 192, 203, 255));
                    pelletCircle.setOrigin(pelletCircle.getRadius(), pelletCircle.getRadius());
                    pelletCircle.setPosition(x * CELL_SIZE + CELL_SIZE / 2.0f, y * CELL_SIZE + CELL_SIZE / 2.0f);
                    window.draw(pelletCircle);
                    break;
                }
                case Energizer: {
                    sf::CircleShape pelletCircle(CELL_SIZE / 6.0f); 
                    pelletCircle.setFillColor(sf::Color::Yellow);
                    pelletCircle.setOrigin(pelletCircle.getRadius(), pelletCircle.getRadius());
                    pelletCircle.setPosition(x * CELL_SIZE+ CELL_SIZE / 2.0f, y * CELL_SIZE+ CELL_SIZE / 2.0f);
                    window.draw(pelletCircle);
                    break;
                }
                case Powerup: {
                    sf::CircleShape pelletCircle(CELL_SIZE / 3.0f); 
                    pelletCircle.setFillColor(sf::Color::White);
                    pelletCircle.setOrigin(pelletCircle.getRadius(), pelletCircle.getRadius());
                    pelletCircle.setPosition(x * CELL_SIZE+ CELL_SIZE / 2.0f, y * CELL_SIZE+ CELL_SIZE / 2.0f);
                    window.draw(pelletCircle);
                    break;
                }
                case Door: {
                    sf::RectangleShape doorRect(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                    doorRect.setFillColor(sf::Color::Black);
                    doorRect.setPosition(x * CELL_SIZE, y * CELL_SIZE);
                    window.draw(doorRect);
                    break;
                }
                case Empty:
                    continue;
            }
        }
    }
}

void loadTextures() {
    for (int i = 0; i < NUM_GHOSTS; ++i) {
        sf::Texture ghostTexture;
        ghostTexture.loadFromFile("ghosts.png");
        ghostTexture.setSmooth(true);
        ghostTextures.push_back(ghostTexture);
    }
}

void initializePositions() {
    ghostPositions[0] = sf::Vector2f(WINDOW_WIDTH / 4, WINDOW_HEIGHT / 2 - 100);
    ghostPositions[1] = sf::Vector2f(WINDOW_WIDTH / 4+22, WINDOW_HEIGHT / 2 - 100);
    ghostPositions[2] = sf::Vector2f(WINDOW_WIDTH / 4+42, WINDOW_HEIGHT / 2 - 100);
    ghostPositions[3] = sf::Vector2f(WINDOW_WIDTH / 4+22, WINDOW_HEIGHT / 2 - 122);
    
    initialGhostPositions[0] = sf::Vector2f(WINDOW_WIDTH / 4, WINDOW_HEIGHT / 2 - 100);
    initialGhostPositions[1] = sf::Vector2f(WINDOW_WIDTH / 4+22, WINDOW_HEIGHT / 2 - 100);
    initialGhostPositions[2] = sf::Vector2f(WINDOW_WIDTH / 4+42, WINDOW_HEIGHT / 2 - 100);
    initialGhostPositions[3] = sf::Vector2f(WINDOW_WIDTH / 4+22, WINDOW_HEIGHT / 2 - 122);
}

bool ghostsVulnerable[NUM_GHOSTS] = {false};


void* drawuimenu(void* arg){
    sf::Font font;
    if (font.loadFromFile("arial.ttf")) {
        sf::Text scoreText;
        scoreText.setFont(font);
        scoreText.setCharacterSize(24);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(10,450);
        scoreText.setString("Score: " + std::to_string(score));
        window.draw(scoreText);

        sf::Text livesText;
        livesText.setFont(font);
        livesText.setCharacterSize(24);
        livesText.setFillColor(sf::Color::White);
        livesText.setPosition(130, 450);
        livesText.setString("Lives: ");
        window.draw(livesText);
        if(lives==3){
        	sf::CircleShape pacmanCircle(10);
        	int f=30;
    		for(int i=0;i<3;i++){
    			pacmanCircle.setFillColor(sf::Color::Yellow);
				pacmanCircle.setOrigin(10,10);
				pacmanCircle.setPosition(180+f,465);
				window.draw(pacmanCircle);
				f+=30;
			}
    		f=30;
        }
        else if(lives==2){
        	sf::CircleShape pacmanCircle(10);
        	int f=30;
    		for(int i=0;i<2;i++){
    			pacmanCircle.setFillColor(sf::Color::Yellow);
				pacmanCircle.setOrigin(10,10);
				pacmanCircle.setPosition(180+f,465);
				window.draw(pacmanCircle);
				f+=30;
			}
    		f=30;
        } 
        else if(lives==1){
        	sf::CircleShape pacmanCircle(10);
        	int f=30;
    		for(int i=0;i<1;i++){
    			pacmanCircle.setFillColor(sf::Color::Yellow);
				pacmanCircle.setOrigin(10,10);
				pacmanCircle.setPosition(180+f,465);
				window.draw(pacmanCircle);
				f+=30;
			}
    		f=30;
        }
    }

	//power up on or not	
    sf::Text livesText;
    livesText.setFont(font);
    livesText.setCharacterSize(20);
    livesText.setFillColor(sf::Color::White);
    livesText.setPosition(300, 452);
    livesText.setString("Power Up: ");
    window.draw(livesText);
    if(energizereaten){
    	sf::CircleShape energizeron(10);
    	energizeron.setFillColor(sf::Color::Blue);
		energizeron.setOrigin(10,10);
		energizeron.setPosition(410,465);
		window.draw(energizeron);	
    } 
    
    sem_post(&ui);
    return NULL;
}

void* draw(void* arg) {
    window.clear();
	drawMap(window);
	
    sf::CircleShape pacmanCircle(PACMAN_SIZE);
    pacmanCircle.setFillColor(sf::Color::Yellow);
    pacmanCircle.setOrigin(PACMAN_SIZE, PACMAN_SIZE);
    pacmanCircle.setPosition(pacmanPosition);
    window.draw(pacmanCircle);

    for (int i = 0; i < NUM_GHOSTS; ++i) {
    sf::CircleShape ghostCircle(PACMAN_SIZE);
    if (ghostsVulnerable[i]) {
        ghostCircle.setFillColor(sf::Color::Blue); // Set to blue if ghosts are vulnerable
    } 
    else {
    	    switch (i) {
    	        case 0:
    	            ghostCircle.setFillColor(sf::Color::Green);
    	            break;
    	        case 1:
    	            ghostCircle.setFillColor(sf::Color::Cyan);
    	            break;
    	        case 2:
    	            ghostCircle.setFillColor(sf::Color::Magenta);
    	            break;
    	        case 3:
    	            ghostCircle.setFillColor(sf::Color::White);
    	            break;
    	        default:
    	            ghostCircle.setFillColor(sf::Color::Red); // Default color
    	            break;
    	    }
    	}
    	ghostCircle.setOrigin(PACMAN_SIZE, PACMAN_SIZE);
    	ghostCircle.setPosition(ghostPositions[i]);
    	window.draw(ghostCircle);
	}
	
	
    
    sem_init(&ui, 0, 0);
	pthread_t uithread;
    pthread_create(&uithread, NULL, &drawuimenu,NULL);
	sem_wait(&ui);
	
    window.display();
    
    sem_post(&drawingmap);
    return NULL;
}

void waitinginghosthouse(int i){
	pthread_t thread1;
   //pthread_create(&thread1, NULL, &leavehouse1,&i);
     sem_wait(&keySemaphore);
     cout << "Ghost " <<i << " acquired a key." << endl;

     // Attempt to acquire permits
     sem_wait(&permitSemaphore);
     cout << "Ghost " << i << " acquired a permit." << endl;
     
     cout << "Ghost " << i << " is leaving the ghost house." << endl;
  
     // Release keys
     sem_post(&keySemaphore);
     usleep(500000);
     //T
     // Release permits
     sem_post(&permitSemaphore);
     usleep(500000);
     cout << "Ghost " << i << " released key and permit." << endl;
     // ghostPositions[i]=initialGhostPositions[i];

    usleep(2000000);
    

   	ghostPositions[i] = sf::Vector2f(WINDOW_WIDTH / 4 +12, WINDOW_HEIGHT / 2 - 145);
    	
}
void* energizerEffect(void* arg) {
    // Cast the argument back to the appropriate type
    bool* ghostsVulnerable = static_cast<bool*>(arg);

	energizereaten = true;
    for(int i = 0; i < 4; i++) {
        ghostsVulnerable[i] = true;
    }
    

    time_t startTime = std::time(nullptr);
    while (std::difftime(std::time(nullptr), startTime) < 10) {
        // Wait
    }

    for(int i = 0; i < 4; i++) {
        ghostsVulnerable[i] = false;
    }
    
    energizereaten = false;

    pthread_exit(NULL);
}

void makeghostfaster(int id){
    pthread_mutex_lock(&ghostfastMutex);
    sleepDuration[id]=5;
    pthread_mutex_unlock(&ghostfastMutex);
}

void* ghostcollisionchecker(void*){
		 while(true){
		 sf::FloatRect pacmanRect(pacmanPosition.x - PACMAN_SIZE / 2, pacmanPosition.y - PACMAN_SIZE / 2, PACMAN_SIZE, PACMAN_SIZE);

		 //ghost collision
         for (int i = 0; i < NUM_GHOSTS; ++i) {
            pthread_mutex_lock(&ghostMutex);
        
            sf::FloatRect ghostRect(ghostPositions[i].x - PACMAN_SIZE / 2, ghostPositions[i].y - PACMAN_SIZE / 2, PACMAN_SIZE, PACMAN_SIZE);
   		
   		    pthread_mutex_unlock(&ghostMutex);
        	const float CollisionSizeIncrease = 4.0f;
        	
			pacmanRect.left -= CollisionSizeIncrease;
			pacmanRect.top -= CollisionSizeIncrease;
			pacmanRect.width += CollisionSizeIncrease;
			pacmanRect.height += CollisionSizeIncrease;

			ghostRect.left -= CollisionSizeIncrease;
			ghostRect.top -= CollisionSizeIncrease;
			ghostRect.width += CollisionSizeIncrease;
			ghostRect.height += CollisionSizeIncrease;

            if (pacmanRect.intersects(ghostRect)){
                if (ghostsVulnerable[i]) {
                    // Reset ghost position
					ghostPositions[i] = sf::Vector2f(WINDOW_WIDTH / 4 + 12, WINDOW_HEIGHT / 2 - 105);
					waitinginghosthouse(i);
                    ghostsVulnerable[i] =false;
                } else {
                    // Handle collision with non-vulnerable ghost
                    pthread_mutex_lock(&livesMutex);
                    lives--;
                    pthread_mutex_unlock(&livesMutex);
                    if (lives <= 0) {
                        std::cout << "Game Over!" << std::endl;
                        exit(0); 
                    } else {
                        // Reset Pacman's position
                        pacmanPosition = sf::Vector2f(WINDOW_WIDTH / 4 + 22, WINDOW_HEIGHT / 2 + 25);
                        // Reset ghost positions
                        ghostPositions[0] = sf::Vector2f(WINDOW_WIDTH / 4 +12, WINDOW_HEIGHT / 2 - 105);
                        ghostPositions[1] = sf::Vector2f(WINDOW_WIDTH / 4 +12, WINDOW_HEIGHT / 2 - 105);
                        ghostPositions[2] = sf::Vector2f(WINDOW_WIDTH / 4 +12, WINDOW_HEIGHT / 2 - 105);
                        ghostPositions[3] = sf::Vector2f(WINDOW_WIDTH / 4 +12, WINDOW_HEIGHT / 2 - 105);
                        
                        for(int l=0;l<NUM_GHOSTS;l++){
                           int* ghostId = new int(l); // Allocate memory dynamically to prevent data race

    // Create thread
                      pthread_t thread4;
                        pthread_create(&thread4, NULL, leavehouse1, ghostId);
                        } 
                    }
                }
            }
        }
        
        
        if(ghostpowerupenabled){
        	for(int i=0;i<4;i++){
        		for (int y = 0; y < MAP_HEIGHT; ++y) {
        		    for (int x = 0; x < MAP_WIDTH; ++x) {
        		        if (::map[y][x] == Powerup) {
        		            float pelletX = x * CELL_SIZE + CELL_SIZE / 2.0f;
        		            float pelletY = y * CELL_SIZE + CELL_SIZE / 2.0f;
        		            float distance = std::sqrt(std::pow(ghostPositions[i].x - pelletX, 2) + std::pow(ghostPositions[i].y - pelletY, 2));
        		            if (distance <= PACMAN_SIZE + COIN_SIZE / 2.0f) {
       		 	                //ghost took the power up 
       		 	                //now it's speed gets faster
       		 	                makeghostfaster(i);
       		 	                
      		 	                ::map[y][x] = Empty;
      		  	            }
      		  	        }
      		  	    }
      		  	}  
        	}
        }
       }
    return NULL;
}
void* coinUpdaterThread(void*) {
    while (true) {
        pthread_mutex_lock(&scoreMutex);
        
        //pellet collision
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            for (int x = 0; x < MAP_WIDTH; ++x) {
                if (::map[y][x] == Pellet) {
                    float pelletX = x * CELL_SIZE + CELL_SIZE / 2.0f;
                    float pelletY = y * CELL_SIZE + CELL_SIZE / 2.0f;
                    float distance = std::sqrt(std::pow(pacmanPosition.x - pelletX, 2) + std::pow(pacmanPosition.y - pelletY, 2));
                    if (distance <= PACMAN_SIZE + COIN_SIZE / 2.0f) {
                        score += 1;
                        if(score==146){
                        	cout<<"Game Won"<<endl;
                        	exit(0);
                        }
                        else if(score==50){
                        	ghostpowerupenabled=true;
                        }
                        ::map[y][x] = Empty;
                    }
                }
            }
        }
        
        if(!energizereaten)
        // Energizer collision
		for (int y = 0; y < MAP_HEIGHT; ++y) {
			for (int x = 0; x < MAP_WIDTH; ++x) {
				if (::map[y][x] == Energizer) {
				    float energizerX = x * CELL_SIZE + CELL_SIZE / 2.0f;
				    float energizerY = y * CELL_SIZE + CELL_SIZE / 2.0f;
				    float distance = std::sqrt(std::pow(pacmanPosition.x - energizerX, 2) + std::pow(pacmanPosition.y - energizerY, 2));
				    if (distance <= PACMAN_SIZE + COIN_SIZE / 2.0f) {
				        ::map[y][x] = Empty;
				        // Make ghosts vulnerable for 10 seconds
				        pthread_create(&threadEnergizer, NULL, energizerEffect, static_cast<void*>(ghostsVulnerable));
				    }
				}
			}
		}
        
        sem_post(&energizer);
        
        pthread_mutex_unlock(&scoreMutex);
    }
   
    return NULL;
}


bool checkGhostCollision(const sf::Vector2f& ghostPosition, const std::array<std::array<Cell, MAP_WIDTH>, MAP_HEIGHT>& map) {
    int ghostGridLeft = static_cast<int>((ghostPosition.x - PACMAN_SIZE) / CELL_SIZE);
    int ghostGridRight = static_cast<int>((ghostPosition.x + PACMAN_SIZE) / CELL_SIZE);
    int ghostGridTop = static_cast<int>((ghostPosition.y - PACMAN_SIZE) / CELL_SIZE);
    int ghostGridBottom = static_cast<int>((ghostPosition.y + PACMAN_SIZE) / CELL_SIZE);

    // Check collision with all four corners of the ghost's rectangle
    if (ghostGridLeft < 0 || ghostGridRight >= MAP_WIDTH || ghostGridTop < 0 || ghostGridBottom >= MAP_HEIGHT) {
        return true; // Ghost is out of bounds
    }

    return map[ghostGridTop][ghostGridLeft] == Wall ||
           map[ghostGridTop][ghostGridRight] == Wall ||
           map[ghostGridBottom][ghostGridLeft] == Wall ||
           map[ghostGridBottom][ghostGridRight] == Wall;
}


sf::Vector2f getRandomDirection() {
    srand(time(NULL));
    int dir = rand() % 4; 
    sf::Vector2f direction;

    switch (dir) {
        case 0:
            direction = sf::Vector2f(0, -1); // Up
            break;
        case 1:
            direction = sf::Vector2f(0, 1); // Down
            break;
        case 2:
            direction = sf::Vector2f(-1, 0); // Left
            break;
        case 3:
            direction = sf::Vector2f(1, 0); // Right
            break;
    }

    return direction;
}

bool checkWallCollision(const sf::Vector2f& pacmanPosition, const std::array<std::array<Cell, MAP_WIDTH>, MAP_HEIGHT>& map) {
    int pacmanGridLeft = static_cast<int>((pacmanPosition.x - PACMAN_SIZE) / CELL_SIZE);
    int pacmanGridRight = static_cast<int>((pacmanPosition.x + PACMAN_SIZE) / CELL_SIZE);
    int pacmanGridTop = static_cast<int>((pacmanPosition.y - PACMAN_SIZE) / CELL_SIZE);
    int pacmanGridBottom = static_cast<int>((pacmanPosition.y + PACMAN_SIZE) / CELL_SIZE);

    // Check collision with all four corners of the pacman's rectangle
    if (pacmanGridLeft < 0 || pacmanGridRight >= MAP_WIDTH || pacmanGridTop < 0 || pacmanGridBottom >= MAP_HEIGHT) {
        return true; 
    }

    return map[pacmanGridTop][pacmanGridLeft] == Wall ||
           map[pacmanGridTop][pacmanGridRight] == Wall ||
           map[pacmanGridBottom][pacmanGridLeft] == Wall ||
           map[pacmanGridBottom][pacmanGridRight] == Wall;
}

bool movingLeft = false;
bool movingRight = false;
bool movingUp = false;
bool movingDown = false;

void* movePacman(void* arg) {
	sf::Vector2f intendedPosition = pacmanPosition;
	
    if (movingLeft) {
        intendedPosition.x -= PACMAN_SPEED;
    } else if (movingRight) {
        intendedPosition.x += PACMAN_SPEED;
    } else if (movingUp) {
        intendedPosition.y -= PACMAN_SPEED;
    } else if (movingDown) {
        intendedPosition.y += PACMAN_SPEED;
    }
    
    //wraparound
    if (intendedPosition.x > 430 && movingRight)
        intendedPosition.x = 10;
    if (intendedPosition.x < 10 && movingLeft)
        intendedPosition.x = 430;
    
	//collision with walls
    if (!checkWallCollision(intendedPosition, ::map)) { 
        pacmanPosition = intendedPosition;
    }
    
	sem_post(&movepacman);
    return NULL;
}

void specialKeys(sf::RenderWindow& window, sf::Event::KeyEvent key) {
    switch (key.code) {
        case sf::Keyboard::Left:
            movingLeft = true;
            movingRight = false;
            movingUp = false;
            movingDown = false;
            break;
        case sf::Keyboard::Right:
            movingLeft = false;
            movingRight = true;
            movingUp = false;
            movingDown = false;
            break;
        case sf::Keyboard::Up:
            movingLeft = false;
            movingRight = false;
            movingUp = true;
            movingDown = false;
            break;
        case sf::Keyboard::Down:
            movingLeft = false;
            movingRight = false;
            movingUp = false;
            movingDown = true;
            break;
    }
}

bool gamestarted = false;
bool moved[NUM_GHOSTS] = {false};
int directions[4] = {0,0,1,1};

void scattermode(int ghostId)
{
	sf::Vector2f direction;

	if(directions[ghostId] == 0){
		direction = sf::Vector2f(-1, 0); //Left;
	}
	else if(directions[ghostId] == 1){
		direction = sf::Vector2f(1, 0); //Right;
	}
	else if(directions[ghostId] == 2){
		direction = sf::Vector2f(0, -1); //Up;
	}
	else if(directions[ghostId] == 3){
		direction = sf::Vector2f(0, 1); //Down;
	}
	
	sf::Vector2f newPosition;
	
	if(ghostId == 0){

        	pthread_mutex_lock(&ghostMutex);
            newPosition = ghostPositions[ghostId] + direction;
            pthread_mutex_unlock(&ghostMutex);
        
        	bool cantMove = false;
        
        	pthread_mutex_lock(&ghostMutex);
        	cantMove = checkGhostCollision(newPosition, ::map);
        	pthread_mutex_unlock(&ghostMutex);
        	
        int reverse;
		int dir = directions[ghostId];
		
        if (cantMove) {
        	int num = rand() % 4;      	
        		
			if(dir==0){
				reverse =1;
				if(num==reverse){
					num=2;
				}
			}
			else if(dir==1){
				reverse=0;
				if(num==reverse){
					num=2;
				}
			}
			else if(dir == 2){
				reverse=3;
				if(num==reverse){
					num=0;
				}
			}
			else if(dir == 3){
				reverse=2;
				if(num==reverse){
					num--;
				}
			}
        		
        	//if(num == directions[ghostId] ||(num == directions[ghostId] && num==3)){
        	//	if(num==0){
        	//		num++;
        	//	}
        	//	else  num--;
        	//}

        	directions[ghostId] = num;
            //direction = getRandomDirection();
        } else {
            // Update ghost's position if it can move
            pthread_mutex_lock(&ghostMutex);
            ghostPositions[ghostId] = newPosition;
            pthread_mutex_unlock(&ghostMutex);
        }
	}
	else if(ghostId == 1){
			//cout<<"Ghost "<<ghostId<<" is going "<<directions[ghostId]<<endl;
			
            pthread_mutex_lock(&ghostMutex);
            newPosition = ghostPositions[ghostId] + direction;
            pthread_mutex_unlock(&ghostMutex);
        
        	bool cantMove = false;
        
        	pthread_mutex_lock(&ghostMutex);
        	cantMove = checkGhostCollision(newPosition, ::map);
        	pthread_mutex_unlock(&ghostMutex);
        
		int reverse;
		int dir = directions[ghostId];
		
        if (cantMove) {
        	int num = rand() % 4;      	
        		
			if(dir==0){
				reverse =1;
				if(num==reverse){
					num=2;
				}
			}
			else if(dir==1){
				reverse=0;
				if(num==reverse){
					num=2;
				}
			}
			else if(dir == 2){
				reverse=3;
				if(num==reverse){
					num=0;
				}
			}
			else if(dir == 3){
				reverse=2;
				if(num==reverse){
					num--;
				}
			}
        	directions[ghostId] = num;
            //direction = getRandomDirection();
        } else {
            // Update ghost's position if it can move
            pthread_mutex_lock(&ghostMutex);
            ghostPositions[ghostId] = newPosition;
            pthread_mutex_unlock(&ghostMutex);
        }

	}
	else if(ghostId == 2){
	
			//cout<<"Ghost "<<ghostId<<" is going "<<directions[ghostId]<<endl;
			
            pthread_mutex_lock(&ghostMutex);
            newPosition = ghostPositions[ghostId] + direction;
            pthread_mutex_unlock(&ghostMutex);
        
        	bool cantMove = false;
        
        	pthread_mutex_lock(&ghostMutex);
        	cantMove = checkGhostCollision(newPosition, ::map);
        	pthread_mutex_unlock(&ghostMutex);
        
		int reverse;
		int dir = directions[ghostId];
		
        if (cantMove) {
        	int num = rand() % 4;      	
        		
			if(dir==0){
				reverse =1;
				if(num==reverse){
					num=2;
				}
			}
			else if(dir==1){
				reverse=0;
				if(num==reverse){
					num=2;
				}
			}
			else if(dir == 2){
				reverse=3;
				if(num==reverse){
					num=0;
				}
			}
			else if(dir == 3){
				reverse=2;
				if(num==reverse){
					num--;
				}
			}
        	directions[ghostId] = num;
            //direction = getRandomDirection();
        } else {
            // Update ghost's position if it can move
            pthread_mutex_lock(&ghostMutex);
            ghostPositions[ghostId] = newPosition;
            pthread_mutex_unlock(&ghostMutex);
        }
	}
	else if(ghostId == 3){
			//cout<<"Ghost "<<ghostId<<" is going "<<directions[ghostId]<<endl;
			
            pthread_mutex_lock(&ghostMutex);
            newPosition = ghostPositions[ghostId] + direction;
            pthread_mutex_unlock(&ghostMutex);
        
        	bool cantMove = false;
        
        	pthread_mutex_lock(&ghostMutex);
        	cantMove = checkGhostCollision(newPosition, ::map);
        	pthread_mutex_unlock(&ghostMutex);
        

        int reverse;
		int dir = directions[ghostId];
		
        if (cantMove) {
        	int num = rand() % 4;      	
        		
			if(dir==0){
				reverse =1;
				if(num==reverse){
					num=2;
				}
			}
			else if(dir==1){
				reverse=0;
				if(num==reverse){
					num=2;
				}
			}
			else if(dir == 2){
				reverse=3;
				if(num==reverse){
					num=0;
				}
			}
			else if(dir == 3){
				reverse=2;
				if(num==reverse){
					num--;
				}
			}
        	directions[ghostId] = num;
        } else {
            pthread_mutex_lock(&ghostMutex);
            ghostPositions[ghostId] = newPosition;
            pthread_mutex_unlock(&ghostMutex);
        }
	}
	
     // milliseconds
    clock_t start = clock();
    while ((clock() - start) * 500 / CLOCKS_PER_SEC < sleepDuration[ghostId]) {}
}
void* leavehouse1(void* arg){
	int ghostId = (*reinterpret_cast<int*>(arg));
    delete reinterpret_cast<int*>(arg); 
    cout<<"Ghost ID"<<ghostId<<endl;
    
     sem_wait(&keySemaphore);
     cout << "Ghost " <<ghostId << " acquired a key." << endl;

     // Attempt to acquire permits
     sem_wait(&permitSemaphore);
     cout << "Ghost " << ghostId << " acquired a permit." << endl;
     
     cout << "Ghost " << ghostId << " is leaving the ghost house." << endl;
  cout<<"This is leave house 1 :"<<endl;
     // Release keys
     sem_post(&keySemaphore);
     usleep(500000);
     
     // Release permits
     sem_post(&permitSemaphore);
     usleep(5000000);
     cout << "Ghost " << ghostId << " released key and permit." << endl;
    ghostPositions[ghostId] = sf::Vector2f(WINDOW_WIDTH / 4 +15, WINDOW_HEIGHT / 2 - 145);
	if(!moved[ghostId]){
		ghostPositions[ghostId] = sf::Vector2f(WINDOW_WIDTH / 4 +15, WINDOW_HEIGHT / 2 - 145);
		moved[ghostId]=true;
		cout<<"Ghost "<<ghostId<<"left the ghost house"<<endl;
	}
    sem_post(&leaver);
    return NULL;
}
void* leavehouse(void* arg){
	int ghostId = (*reinterpret_cast<int*>(arg));
    
     sem_wait(&keySemaphore);
     cout << "Ghost " <<ghostId << " acquired a key." << endl;

     // Attempt to acquire permits
     sem_wait(&permitSemaphore);
     cout << "Ghost " << ghostId << " acquired a permit." << endl;
     
     cout << "Ghost " << ghostId << " is leaving the ghost house." << endl;
  
     // Release keys
     sem_post(&keySemaphore);
     usleep(500000);
     
     // Release permits
     sem_post(&permitSemaphore);
     usleep(500000);
     cout << "Ghost " << ghostId << " released key and permit." << endl;
    ghostPositions[ghostId]=initialGhostPositions[ghostId];

	if(!moved[ghostId]){
		ghostPositions[ghostId] = sf::Vector2f(WINDOW_WIDTH / 4 +15, WINDOW_HEIGHT / 2 - 145);
		moved[ghostId]=true;
		cout<<"Ghost "<<ghostId<<"left the ghost house"<<endl;
	}
	
	while(true){
		scattermode(ghostId);
	}
	sem_post(&leaver);
}

void* ghostControllerThread(void* arg) {
	int ghostId = (*reinterpret_cast<int*>(arg));
	
    sem_init(&leaver, 0, 0);
    pthread_t leave;
    pthread_create(&leave, NULL, &leavehouse,&ghostId);

	sem_wait(&leaver);
    return NULL;
}

bool check=true;
sem_t ghostSemaphores[NUM_GHOSTS];

void* gameLoopThread(void*) {
	loadTextures();
    initializePositions();
    initializeMap(); 
	
    sf::Event event;
    bool pacmanMoved = false; //game started or not
	
	
    while (window.isOpen()) 
    {
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::KeyPressed) {
                specialKeys(window, event.key);
                pacmanMoved = true; //game started            
            }
        }
		
		//ghost movements start only if pacman starts
        if (pacmanMoved && check) {
        	check=false;
        	
        	sem_init(&energizer, 0, 0);
            pthread_t coinUpdaterThreadID;
            pthread_create(&coinUpdaterThreadID, NULL, coinUpdaterThread, NULL);
            
			pthread_t ghostcollision;
            pthread_create(&ghostcollision, NULL, ghostcollisionchecker, NULL);
    		     
			sem_wait(&energizer);
			sem_destroy(&energizer);
    
            pthread_t ghostThreads[NUM_GHOSTS];
            int ghostIds[NUM_GHOSTS];
            
            for(int i=0;i<4;i++) sem_init(&keyexit[i],0,1);
            
            for (int i = 0; i < NUM_GHOSTS; ++i) {sem_post(&ghostSemaphores[i]);}
            
            for (int i = 0; i < NUM_GHOSTS; ++i) {
                ghostIds[i] = i;
                pthread_create(&ghostThreads[i], NULL, ghostControllerThread, &ghostIds[i]);
                sem_wait(&ghostSemaphores[i]);
                sem_destroy(&ghostSemaphores[i]);
            }
            
            for(int i=0;i<4;i++) sem_wait(&keyexit[i]);
                     
            pacmanMoved = false; 
            
            
        }

		sem_init(&movepacman, 0, 0);
		
        pthread_t movingpacman;
    	pthread_create(&movingpacman, NULL, movePacman, NULL);
    	sem_wait(&movepacman);
    	sem_destroy(&movepacman);
    	
    	sem_init(&movepacman, 0, 0);
		
        pthread_t drawing;
    	pthread_create(&drawing, NULL, draw, NULL);
    	sem_wait(&drawingmap);
    	sem_destroy(&drawingmap);
    }

	sem_post(&gameLoopSemaphore);
    return NULL;
}

int main() {
	
    pthread_mutex_init(&pacmanMutex, NULL);
    pthread_mutex_init(&ghostMutex, NULL);
    pthread_mutex_init(&scoreMutex, NULL);
    pthread_mutex_init(&livesMutex, NULL);
    pthread_mutex_init(&ghostfastMutex, NULL);

	sem_init(&keySemaphore, 0, NUM_RESOURCES);
    sem_init(&permitSemaphore, 0, NUM_RESOURCES);
    
    // Initialize key mutex
    pthread_mutex_init(&keyMutex, NULL);
    // Initialize permit mutex
    pthread_mutex_init(&permitMutex, NULL);
    
    sem_init(&gameLoopSemaphore, 0, 0);
    pthread_t gameLoopThreadId;
    pthread_create(&gameLoopThreadId, NULL, gameLoopThread, NULL);
    sem_wait(&gameLoopSemaphore);
	sem_destroy(&gameLoopSemaphore);
    
    pthread_mutex_destroy(&pacmanMutex);
    pthread_mutex_destroy(&ghostMutex);
    pthread_mutex_destroy(&scoreMutex);
    pthread_mutex_destroy(&livesMutex);

    return 0;
}