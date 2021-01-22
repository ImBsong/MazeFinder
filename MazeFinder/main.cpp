#include "graph.h"
#include "FileLogger.h"

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

    Graph graph(20); // n x n grid
    graph.run();
    
    return 0;
}  