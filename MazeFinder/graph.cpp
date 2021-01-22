#include "graph.h"

void Graph::initGraph()
{
    debugOffset = 120.f; // Just to create an offset size for debug text at top of screen
    blockSize = 40.f;
    createLog(": Graph::initGraph()", MazeLog::FileLogger::e_logType::LOG_INFO);
}

Vertex::Vertex(float xpos, float ypos, float blockSize)
{
    visited = false;
    isPath = true;
    shape.setFillColor(sf::Color::Black);
    shape.setOutlineThickness(1.f);
    shape.setOutlineColor(sf::Color::White);
    shape.setPosition(xpos, ypos);
    shape.setSize(sf::Vector2f(blockSize, blockSize));
}

void Graph::initMatrix()
{
    // Initialize the grid with vertexes
    for (size_t i = 0; i < gridSize; i++)
    {
        std::vector<Vertex*> temp;
        grid.emplace_back(temp);
        for (size_t j = 0; j < gridSize; j++)
        {
            grid[i].emplace_back(new Vertex( j * blockSize, (i * blockSize) + debugOffset, blockSize)); // x, y + offset, blocksize
            grid[i][j]->col = j;
            grid[i][j]->row = i;
        }
    }
    // Update neighbors.  Important to do this after initializing the grid above
    for (size_t i = 0; i < gridSize; i++)
    {
        for (size_t j = 0; j < gridSize; j++)
        {
            updateNeightbors(grid[i][j]);
        }
    }

    randomizeStartEnd();

    createLog(": Graph::initMatrix()", MazeLog::FileLogger::e_logType::LOG_INFO);
}

void Graph::initGui()
{
    if (!debugFont.loadFromFile("Fonts/Vonique 64.ttf"))
        std::cout << "ERROR: Font failed to load!\n";

    debugText.setFont(debugFont);
    debugText.setCharacterSize(24);
    debugText.setFillColor(sf::Color::White);
    debugText.setPosition(10.f, 10.f);

    createLog(": Graph::initGui()", MazeLog::FileLogger::e_logType::LOG_INFO);
}

void Graph::initLogger()
{
    logger.initFile();
}

void Graph::initWindow()
{
    sf::VideoMode videoMode = sf::VideoMode(blockSize * gridSize, (blockSize * gridSize) + debugOffset);
    window = new sf::RenderWindow(videoMode, "Graph Maze", sf::Style::Close | sf::Style::Titlebar | sf::Style::Resize);
    window->setFramerateLimit(120);
    window->setVerticalSyncEnabled(false);

    createLog(": Graph::initWindow()", MazeLog::FileLogger::e_logType::LOG_INFO);
}

Graph::Graph(unsigned int size)
{
    gridSize = size;
    //Probably want to pass size down to initMatrix to create a proper sized matrix.

    initLogger();
    initGraph();
    initMatrix();
    initGui();
    initWindow();
}

Graph::~Graph()
{
    delete window;
    for (size_t i = 0; i < gridSize; i++)
    {
        for (size_t j = 0; j < gridSize; j++)
        {
            grid[i].erase(grid[i].begin());
        }
    }
}

void Graph::run()
{
    while (window->isOpen())
    {
        pollEvents();
        update();
        render();
    }
}

void Graph::update()
{
    updateGui();
}

void Graph::updateGui()
{
    std::stringstream ss;
    sf::Vector2i position = sf::Mouse::getPosition(*window);
    ss << "ROW, COL:" << "  " << static_cast<int>((position.y - debugOffset) / blockSize) << ", " << static_cast<int>(position.x / blockSize) << '\n' <<
        "Start:         " << getStart()->row << "  " << getStart()->col << '\n' <<
        "End:            " << getEnd()->row << "  " << getEnd()->col;

    debugText.setString(ss.str());
}

void Graph::render()
{
    //Always clear first
    window->clear();

    for (size_t i = 0; i < gridSize; i++)
    {
        for (size_t j = 0; j < gridSize; j++)
        {
            window->draw(grid[i][j]->shape);
        }
    }

    //Render GUI last
    renderGui(); 

    //Always display last
    window->display();
}

void Graph::renderGui()
{
    window->draw(debugText);
}

const Vertex * Graph::getStart() const
{
    if (start)
        return start;
    else
        return nullptr;
}

const Vertex * Graph::getEnd() const
{
    if (end)
        return end;
    else
        return nullptr;
}

void Graph::pollEvents()
{
    sf::Vector2i position = sf::Mouse::getPosition(*window);

    while (window->pollEvent(ev))
    {
        switch (ev.type)
        {
        case sf::Event::Closed:
            window->close();
            break;
        case sf::Event::KeyPressed:
            if (ev.key.code == sf::Keyboard::Escape)
                window->close();
            else if (ev.key.code == sf::Keyboard::S)
                setStartSquare();
            else if (ev.key.code == sf::Keyboard::E)
                setEndSquare();
            else if (ev.key.code == sf::Keyboard::Space)
                BFSexplore();
            else if (ev.key.code == sf::Keyboard::N)
                DFSexplore();
            else if (ev.key.code == sf::Keyboard::R)
                resetAllSquares();
            else if (ev.key.code == sf::Keyboard::V)
                colorVisited(position.x, position.y - debugOffset);
            else if (ev.key.code == sf::Keyboard::B)
                colorUnvisited(position.x, position.y - debugOffset);
            break;
        case sf::Event::MouseButtonReleased:
            if (ev.key.code == sf::Mouse::Left)
            {
                colorVisited(ev.mouseButton.x, ev.mouseButton.y - debugOffset);
            }
        }
    }
}

void Graph::colorVisited(const float& mouseXpos, const float& mouseYpos)
{

    if (mouseXpos / blockSize < gridSize && mouseYpos / blockSize < gridSize && mouseYpos  > 0)
    {
        
        if (grid[mouseYpos / blockSize][mouseXpos / blockSize]->isPath == true)
        {
            grid[mouseYpos / blockSize][mouseXpos / blockSize]->shape.setFillColor(sf::Color::White);
            grid[mouseYpos / blockSize][mouseXpos / blockSize]->isPath = false;
        }
        else if (grid[mouseYpos / blockSize][mouseXpos / blockSize] == this->start || grid[mouseYpos / blockSize][mouseXpos / blockSize] == this->end)
        {
            std::cout << "Error.  This block is already assigned!\n";
        }
        //else 
        //{
        //    grid[mouseYpos / blockSize][mouseXpos / blockSize]->shape.setFillColor(sf::Color::Black);
        //    grid[mouseYpos / blockSize][mouseXpos / blockSize]->isPath = true;
        //}
    }

    
}

void Graph::colorUnvisited(const float & mouseXpos, const float & mouseYpos)
{
    if (mouseXpos / blockSize < gridSize && mouseYpos / blockSize < gridSize && mouseYpos  > 0)
    {
        if (grid[mouseYpos / blockSize][mouseXpos / blockSize]->isPath == false)
        {
            grid[mouseYpos / blockSize][mouseXpos / blockSize]->shape.setFillColor(sf::Color::Black);
            grid[mouseYpos / blockSize][mouseXpos / blockSize]->isPath = true;
        }
        else if (grid[mouseYpos / blockSize][mouseXpos / blockSize] == this->start || grid[mouseYpos / blockSize][mouseXpos / blockSize] == this->end)
        {
            std::cout << "Error.  This block is already assigned!\n";
        }
    }
}

void Graph::updateNeightbors(Vertex * vertex)
{
    if (this->gridSize < 2) // grid size 1 or less should do nothings
    {
        std::cout << "This grid is too small" << std::endl;
        return;
    }
    else if (this->gridSize == 2) // grid size 2
    {
        if (vertex == grid[0][0]) // top left
        {
            vertex->top = nullptr;
            vertex->left = nullptr;
            vertex->bottom = grid[vertex->row + 1][vertex->col];
            vertex->right = grid[vertex->row][vertex->col + 1];
        }
        else if (vertex == grid[0][1]) // top right
        {
            vertex->top = nullptr;
            vertex->right = nullptr;
            vertex->bottom = grid[vertex->row + 1][vertex->col];
            vertex->left = grid[vertex->row][vertex->col - 1];
        }
        else if (vertex = grid[1][0]) // bot left
        {
            vertex->bottom = nullptr;
            vertex->left = nullptr;
            vertex->top = grid[vertex->row - 1][vertex->col];
            vertex->right = grid[vertex->row][vertex->col + 1];
        }
        else // bot right
        {
            vertex->bottom = nullptr;
            vertex->right = nullptr;
            vertex->top = grid[vertex->row - 1][vertex->col];
            vertex->left = grid[vertex->row][vertex->col - 1];
        }
    }
    else // grid size 3x3 and greater can follow this logic
    {
        if (vertex->row < 1)
        {
            if (vertex->col < 1) // top left corner
            {
                vertex->top = nullptr;
                vertex->left = nullptr;
                vertex->bottom = grid[vertex->row + 1][vertex->col];
                vertex->right = grid[vertex->row][vertex->col + 1];
            }
            else if (vertex->col >= 1 && vertex->col < gridSize - 1) // top row until top right corner
            {
                vertex->top = nullptr;
                vertex->bottom = grid[vertex->row + 1][vertex->col];
                vertex->left = grid[vertex->row][vertex->col - 1];
                vertex->right = grid[vertex->row][vertex->col + 1];
            }
            else
            {
                vertex->top = nullptr;
                vertex->right = nullptr;
                vertex->bottom = grid[vertex->row + 1][vertex->col]; // top right corner
                vertex->left = grid[vertex->row][vertex->col - 1];
            }
        }
        else if (vertex->row >= 1 && vertex->row < gridSize - 1) // Just Imagine..
        {
            if (vertex->col < 1)
            {
                vertex->left = nullptr;
                vertex->top = grid[vertex->row - 1][vertex->col];
                vertex->bottom = grid[vertex->row + 1][vertex->col];
                vertex->right = grid[vertex->row][vertex->col + 1];
            }
            else if (vertex->col >= 1 && vertex->col < gridSize - 1) // Middle
            {
                vertex->top = grid[vertex->row - 1][vertex->col];
                vertex->bottom = grid[vertex->row + 1][vertex->col];
                vertex->left = grid[vertex->row][vertex->col - 1];
                vertex->right = grid[vertex->row][vertex->col + 1];
            }
            else
            {
                vertex->right = nullptr;
                vertex->top = grid[vertex->row - 1][vertex->col];
                vertex->bottom = grid[vertex->row + 1][vertex->col];
                vertex->left = grid[vertex->row][vertex->col - 1];
            }
        }
        else
        {
            if (vertex->col < 1) // bottom left corner
            {
                vertex->bottom = nullptr;
                vertex->left = nullptr;
                vertex->top = grid[vertex->row - 1][vertex->col];
                vertex->right = grid[vertex->row][vertex->col + 1];
            }
            else if (vertex->col >= 1 && vertex->col < gridSize - 1) // bot row until top right corner
            {
                vertex->bottom = nullptr;
                vertex->top = grid[vertex->row - 1][vertex->col];
                vertex->left = grid[vertex->row][vertex->col - 1];
                vertex->right = grid[vertex->row][vertex->col + 1];
            }
            else // bot right corner
            {
                vertex->bottom = nullptr;
                vertex->right = nullptr;
                vertex->left = grid[vertex->row][vertex->col - 1];
                vertex->top = grid[vertex->row - 1][vertex->col];
            }
        }
    }
}

void Graph::setStartSquare()
{
    sf::Vector2i position = sf::Mouse::getPosition(*window);
    position.y -= debugOffset; // offset for debug window at top
    if (position.x / blockSize < gridSize && position.x > 0 && position.y / blockSize < gridSize && position.y > 0)
    {
        if (!start)
        {
            grid[position.y / blockSize][position.x / blockSize]->shape.setFillColor(sf::Color::Green);
            grid[position.y / blockSize][position.x / blockSize]->isPath = false;
            grid[position.y / blockSize][position.x / blockSize]->visited = true;

            this->start = grid[position.y / blockSize][position.x / blockSize];
        }
        else
        {
            if (grid[position.y / blockSize][position.x / blockSize] != this->start)
            {
                grid[position.y / blockSize][position.x / blockSize]->shape.setFillColor(sf::Color::Green);
                grid[position.y / blockSize][position.x / blockSize]->isPath = false;
                grid[position.y / blockSize][position.x / blockSize]->visited = true;

                this->start->shape.setFillColor(sf::Color::Black);
                this->start->isPath = true;
                this->start->visited = false;

                this->start = grid[position.y / blockSize][position.x / blockSize]; // Finally set start to the newly set start
                
            }
        }
    }

    createLog(": Graph::setStartSquare()", MazeLog::FileLogger::e_logType::LOG_INFO);
}

void Graph::setEndSquare()
{
    sf::Vector2i position = sf::Mouse::getPosition(*window);
    position.y -= debugOffset; // offset for debug window at top
    if (position.x / blockSize < gridSize && position.x > 0 && position.y / blockSize < gridSize && position.y > 0)
    {
        if (!end)
        {
            grid[position.y / blockSize][position.x / blockSize]->shape.setFillColor(sf::Color::Red);
            grid[position.y / blockSize][position.x / blockSize]->isPath = false;

            this->end = grid[position.y / blockSize][position.x / blockSize];
        }
        else
        {
            if (grid[position.y / blockSize][position.x / blockSize] != this->end)
            {
                grid[position.y / blockSize][position.x / blockSize]->shape.setFillColor(sf::Color::Red);
                grid[position.y / blockSize][position.x / blockSize]->isPath = false;

                this->end->shape.setFillColor(sf::Color::Black);
                this->end->isPath = true;

                this->end = grid[position.y / blockSize][position.x / blockSize];
            }
        }
    }

    createLog(": Graph::setEndSquare()", MazeLog::FileLogger::e_logType::LOG_INFO);
}

void Graph::resetAllSquares()
{
    for (size_t i = 0; i < gridSize; i++)
    {
        for (size_t j = 0; j < gridSize; j++)
        {
            grid[i][j]->visited = false;
            grid[i][j]->isPath = true;
            grid[i][j]->shape.setFillColor(sf::Color::Black);
        }
    }
    randomizeStartEnd();
    
    // Logger.  Passing message as rvalue ref and log type
    createLog(": Graph::resetAllSquares()", MazeLog::FileLogger::e_logType::LOG_INFO);
}

void Graph::randomizeStartEnd()
{
    int randRow = rand() % (gridSize / 2);
    int randCol = rand() % (gridSize / 2);
    start = grid[randRow][randCol];
    grid[randRow][randCol]->shape.setFillColor(sf::Color::Green);
    grid[randRow][randCol]->isPath = false;
    grid[randRow][randCol]->visited = true;

    //Initialize End
    int randRow2 = (rand() % (gridSize / 2)) + (gridSize / 2);
    int randCol2 = (rand() % (gridSize / 2)) + (gridSize / 2);
    end = grid[randRow2][randCol2];
    grid[randRow2][randCol2]->shape.setFillColor(sf::Color::Red);
    grid[randRow2][randCol2]->isPath = false;

    createLog(": Graph::randomizeStartEnd()", MazeLog::FileLogger::e_logType::LOG_INFO);
}

const void Graph::createLog(const std::string&& logLine, MazeLog::FileLogger::e_logType logType)
{
    char *ctime_no_newline;
    time_t tm = time(0);
    ctime_no_newline = strtok(ctime(&tm), "\n");
    std::string stringLog(ctime_no_newline);
    stringLog.append(logLine);
    logger << logType << stringLog;
}

void Graph::BFSexplore()
{
    if (!start)
        std::cout << "ERROR: Set Start Square First!\n";
    else
        this->bfsQueue.emplace(start);

    while (!bfsQueue.empty())
    {
        Vertex * currentNode = bfsQueue.front();
        bfsQueue.pop();

        SearchBFSNeighbors(currentNode);

    }



    createLog(": Graph::BFSexplore()", MazeLog::FileLogger::e_logType::LOG_INFO);
}

void Graph::makeVisited(Vertex * vertex)
{
    vertex->shape.setFillColor(sf::Color::Blue);
    vertex->visited = true;

    //std::chrono::milliseconds dura(100);
    //std::this_thread::sleep_for(dura);
}

void Graph::SearchBFSNeighbors(Vertex * currentNode)
{
    std::vector<Vertex *> listNeighbors = { 
        currentNode->left, 
        currentNode->bottom, 
        currentNode->right, 
        currentNode->top
    };

    for (size_t i = 0; i < listNeighbors.size(); i++) 
    {
        if (listNeighbors[i] && listNeighbors[i]->visited != true && listNeighbors[i]->isPath == true)
        { 
            makeVisited(listNeighbors[i]); 
            bfsQueue.emplace(listNeighbors[i]);

            /*
                Memoizing as using a double for loop is inefficient

                1. Need to create an empty vector
                2. When a new square is added, we need to place that into a vector
                3. Draw items in a vector so it is O(n) instead of O(n^2)
            */

            bfsColorMemoization.reserve(gridSize);
            bfsColorMemoization.emplace_back(listNeighbors[i]);

        }
    }
    for (size_t i = 0; i < bfsColorMemoization.size(); i++)
    {
        window->draw(bfsColorMemoization[i]->shape);
    }

    window->display();

}

void Graph::DFSexplore()
{
    if (!start)
        std::cout << "Error: Set Start Square First!\n";
    else
        this->dfsStack.emplace(start);

    DFSrecurse(start, dfsStack);
    createLog(": Graph::DFSexplore()", MazeLog::FileLogger::e_logType::LOG_INFO);
}

void Graph::DFSrecurse(Vertex * currentNode, std::stack<Vertex*> stack)
{
    std::vector<Vertex *> listNeighbors = {
        currentNode->left,
        currentNode->bottom,
        currentNode->right,
        currentNode->top
    };

    for (size_t i = 0; i < listNeighbors.size(); i++)
    {
        if (listNeighbors[i] && !listNeighbors[i]->visited && listNeighbors[i]->isPath)
        {
            makeVisited(listNeighbors[i]);
            dfsStack.emplace(listNeighbors[i]);
            dfsColorMemoization.reserve(gridSize);
            dfsColorMemoization.emplace_back(listNeighbors[i]);
            for (size_t i = 0; i < dfsColorMemoization.size(); i++)
            {
                window->draw(dfsColorMemoization[i]->shape);
            }

            window->display();
            DFSrecurse(listNeighbors[i], dfsStack);
        }
    }
}