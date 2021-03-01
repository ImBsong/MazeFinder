#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <unordered_map>
#include <queue>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <string>
#include <stack>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>

#include "FileLogger.h"

//Node.  Uses RectangleShape.  Square is represented as (row, col) in GUI
struct Vertex
{
    sf::RectangleShape shape;

    unsigned int col;
    unsigned int row;
    bool haveNeighbors;

    bool isPath;
    bool visited;
    bool explosionHole;
    unsigned int pathDistance;

    // This is for A star
    unsigned int g_cost;
    unsigned int h_cost;
    unsigned int f_cost;
    Vertex * aStarParent;

    Vertex * top;
    Vertex * bottom;
    Vertex * left;
    Vertex * right;

    //Simple Constructor
    Vertex(float xpos, float ypos, float blockSize);
};

class Graph
{
private:
    sf::RenderWindow * window;
    float debugOffset; // Offsets Mouse Position to add GUI at top of window.  
    unsigned int mazeDivideCounter; // 0 = no hole, 1-3 makes hole
    bool endFound;

    sf::Event ev; // General Event to take player inputs.  Does not need to be initialized

    unsigned int gridSize; // N x N.  Size of the matrix
    float blockSize; // Length/Width of each individual blocks

    using Matrix = std::vector<std::vector<Vertex*>>;
    Matrix grid;

    Vertex * start; // Start Square
    Vertex * end; // End Square

    // Containers to help BFS and DFS.  
    std::queue<Vertex*> bfsQueue;
    std::stack<Vertex*> dfsStack;
    std::stack<Vertex*> pathStack;
    std::vector<Vertex*> pathVec;

    // Container for A* algo
    std::vector<Vertex*> priorityQueue; // Open
    std::unordered_map<unsigned int, Vertex*> closedMap; // Closed

    //GUI
    sf::Font debugFont;
    sf::Text debugTextGridInfo;
    sf::Text debugTextHotKeyInfo;
    sf::Text debugPathDistance;

    //Log
    MazeLog::FileLogger logger;

    //Sound
    sf::Music music;

    //Private Initializers
    void initGraph(float blockSize);
    void initMatrix();
    void initGui();
    void initSound();
    void initLogger();
    void initWindow();
    void initOutside();

public:
    //Constructor and Destructor
    Graph(unsigned int gridsize, float blockSize);
    virtual ~Graph();

    //Called from main() and starts everything
    void run();

    //Update Function
    void update(); // Main Update Function
    void updateGui(); // Runs inside update()

    //Render Function
    void render(); // Main Render Function
    void renderGui(); // Runs inside render()

    //Accessor
    const Vertex * getStart() const;
    const Vertex * getEnd() const;

    //General Functions
    void pollEvents();
    void makeVisited(unsigned int row, unsigned int col);
    void makeUnvisited(unsigned int row, unsigned int col);
    void colorVisited(const float& mouseXpos, const float& mouseYpos);
    void colorUnvisited(const float& mouseXpos, const float& mouseYpos);
    void updateNeighborsRecursive(Vertex * vertex);
    bool isValidNeighbor(unsigned int row, unsigned int col) const;
    bool isNeighborsSet(unsigned int row, unsigned int col) const;
    void setStartSquare(); // Uses Keystroke to manually set start square
    void setEndSquare(); // Uses Keystroke to manually set end square
    void resetAllSquares(); // Resets the program.  
    void setDefaultStartEnd();
    void randomizeStartEnd(); // This gets called in resetAllSquares(). Sets Start in top left quadrant and End in bottom right quadrant
    const void createLog(const std::string&& logLine, MazeLog::FileLogger::e_logType logType); // Simple logger.  Creates maze_log.txt in root folder
    unsigned int getPathDistance(const unsigned int& row, const unsigned int& col);
    void colorPath(Vertex * vertex);

    //BFS Functions (Djikstra Shortest path)
    void BFSexplore(); 
    void makeVisited(Vertex * vertex);
    void SearchBFSNeighbors(Vertex * vertex);
    void createPath(Vertex * node); // Start with end node
    void displayPath(std::stack<Vertex *> pathStack);

    // A* Star and Heap
    void aStarExplore();
    unsigned int getLeftChild(const unsigned int& index) { return index * 2 + 1; }
    unsigned int getRightChild(const unsigned int& index) { return index * 2 + 2; }
    unsigned int getParent(const unsigned int& index) { return (index - 1) / 2; }
    unsigned int cantor(const unsigned int& row, const unsigned int& col) { return (row + col) * (row + col + 1) / 2 + col; }
    void swap(unsigned int index1, unsigned int index2);
    void heapInsert(Vertex * vertex);
    void MinHeapify(unsigned int index);
    Vertex * heapExtractMin();
    unsigned int absDiff(const unsigned int& valueOne, const unsigned int& valueTwo);
    void updateCosts(Vertex * vertex);
    void createAStarPath(Vertex * vertex);

    //DFS Functions
    void DFSexplore(); //Recursive
    void DFSrecurse(Vertex * vertex, std::stack<Vertex*> stack);

    //Maze Creator (Recursive)
    void mazeCreator();
    unsigned int randMazeVal(unsigned int size) const;
    unsigned int holeMaker(unsigned int RangeOne, unsigned int RangeTwo);
    void drawQuadrantLines(Vertex * topLeft, Vertex * botRight, unsigned int midHorizontal, unsigned int midVertical);
    void mazeCreatorRecursive(Vertex * topLeft, Vertex * botRight);
    void setExplosionHole(unsigned int row, unsigned int col);
};
#endif // !GRAPH_H