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

#include "FileLogger.h"

//Node.  Uses RectangleShap.  Square is represented as (row, col) in GUI
struct Vertex
{
    sf::RectangleShape shape;
    unsigned int col;
    unsigned int row;
    bool isPath;
    bool visited;
    Vertex * top;
    Vertex * bottom;
    Vertex * left;
    Vertex * right;

    //Simple Constructor
    Vertex(float xpos, float ypos, float blockSize);
};


class Graph
{private:
    sf::RenderWindow * window;
    float debugOffset; // Offsets Mouse Position to add GUI at top of window.  

    sf::Event ev; // General Event to take player inputs.  Does not need to be initialized

    unsigned int gridSize;
    float blockSize;

    using Matrix = std::vector<std::vector<Vertex*>>;
    Matrix grid;
    Matrix djikstraAdjMat; // Not implemented yet.  But will need this to store data to create Djikstra Algo

    Vertex * start;
    Vertex * end;

    // Containers to help BFS and DFS 
    std::queue<Vertex*> bfsQueue;
    std::stack<Vertex*> dfsStack;

    // Vectors to help animation during BFS and DFS
    std::vector<Vertex*> bfsColorMemoization;
    std::vector<Vertex*> dfsColorMemoization;

    //GUI
    sf::Font debugFont;
    sf::Text debugText;

    //Log
    MazeLog::FileLogger logger;

    //Private Initializers
    void initGraph();
    void initMatrix();
    void initGui();
    void initLogger();
    void initWindow();
public:

    //Constructor and Destructor
    Graph(unsigned int size);
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
    void colorVisited(const float& mouseXpos, const float& mouseYpos);
    void colorUnvisited(const float& mouseXpos, const float& mouseYpos);
    void updateNeightbors(Vertex * vertex);
    void setStartSquare(); // Uses Keystroke to manually set start square
    void setEndSquare(); // Uses Keystroke to manually set end square
    void resetAllSquares(); // Resets the program.  
    void randomizeStartEnd(); // This gets called in resetAllSquares(). Sets Start in top left quadrant and End in bottom right quadrant
    const void createLog(const std::string&& logLine, MazeLog::FileLogger::e_logType logType); // Simple logger.  Creates maze_log.txt in root folder

    //BFS Functions
    void BFSexplore(); 
    void makeVisited(Vertex * vertex);
    void SearchBFSNeighbors(Vertex * vertex);

    //DFS Functions
    void DFSexplore(); //Recursive
    void DFSrecurse(Vertex * vertex, std::stack<Vertex*> stack);
};
#endif // !GRAPH_H

