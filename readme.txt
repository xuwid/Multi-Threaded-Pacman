
Developed a multi-threaded Pacman game using C++ and the pthread library,
employing semaphores and mutexes for thread synchronization without using
joins. Implemented game logic to handle concurrent movements of ghosts and
the player, optimizing real-time gameplay.
Use this command to compile and run the game:
g++ -std=c++11 -Wall -Wextra -I/usr/include/SFML -o project project.cpp -lsfml-graphics -lsfml-window -lsfml-system
./project
