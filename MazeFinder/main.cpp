#include "graph.h"
#include "FileLogger.h"

#include <iostream>

/*
    Goal: Make an animated pathfinder

    Done:
    - Implement Grid
    - Implement Logging System
    - Implement GUI
    - Implement BFS
    - Implement DFS

    Todo:
    - Implement Djikstra to find the shortest path
    - Implement A* to find the shortest path
 
    Note:
    - This is un-weighted, un-directed graph (maze)

 */

int main()
{
    //Randomize Time Seed
    std::srand(static_cast<unsigned int>(time(0)));

    float gridSize;
    float blockSize;

    std::cout << "Please enter desired GridSize (Ex: 30): ";
    std::cin >> gridSize;
    std::cout << '\n\n';

    std::cout << "Please enter desired BlockSize (Ex: 30) ";
    std::cin >> blockSize;

    Graph graph(gridSize, blockSize); // Grid Size, Block Size
    graph.run();
    
    return 0;
}  