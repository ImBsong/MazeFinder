#include "graph.h"

void Graph::initGraph(float blockSize)
{
    debugOffset = 240.f; // Create an offset size for debug text at top of screen.  120.f was a while
    this->blockSize = blockSize;
    createLog(": Graph::initGraph()", MazeLog::FileLogger::e_logType::LOG_INFO);
    pathVec.reserve(gridSize*gridSize);
    mazeDivideCounter = 0;
    endFound = false;
}

// Init Individual Blocks
Vertex::Vertex(float xpos, float ypos, float blockSize)
{
    visited = false;
    isPath = true;
    pathDistance = 0;
    explosionHole = false;
    haveNeighbors = false;

    top = nullptr;
    left = nullptr;
    bottom = nullptr;
    right = nullptr;

    // astar
    aStarParent = nullptr;
    g_cost = INT_MAX;
    h_cost = INT_MAX;
    f_cost = INT_MAX;

    // shape
    shape.setFillColor(sf::Color::Black);
    shape.setOutlineThickness(1.f);
    shape.setOutlineColor(sf::Color::White);
    shape.setPosition(xpos, ypos);
    shape.setSize(sf::Vector2f(blockSize, blockSize));

}

void Graph::initMatrix()
{
    // Initialize the grid with vertexes
    grid.reserve(gridSize); 
    for (size_t i = 0; i < gridSize; i++)
    {
        std::vector<Vertex*> temp;
        temp.reserve(gridSize);
        grid.emplace_back(temp);
        for (size_t j = 0; j < gridSize; j++)
        {
            grid[i].emplace_back(new Vertex( j * blockSize, (i * blockSize) + debugOffset, blockSize)); // x, y + offset, blocksize
            grid[i][j]->col = j;
            grid[i][j]->row = i;
        }
    }

    updateNeighborsRecursive(grid[0][0]); // You can start anywhere! But starting at top left

    //setDefaultStartEnd();
    randomizeStartEnd();

    createLog(": Graph::initMatrix()", MazeLog::FileLogger::e_logType::LOG_INFO);
}

void Graph::initGui()
{
    if (!debugFont.loadFromFile("Fonts/Vonique 64.ttf"))
        std::cout << "ERROR: Font failed to load!\n";

    // Grid Info Text
    debugTextGridInfo.setFont(debugFont);
    debugTextGridInfo.setCharacterSize(20);
    debugTextGridInfo.setFillColor(sf::Color::White);
    debugTextGridInfo.setPosition(10.f, 10.f);

    // Hotkey Info Text
    debugTextHotKeyInfo.setFont(debugFont);
    debugTextHotKeyInfo.setCharacterSize(20);
    debugTextHotKeyInfo.setFillColor(sf::Color::White);
    debugTextHotKeyInfo.setPosition((gridSize * blockSize) - 200.f, 10.f); // Anchor it from the right by -X

    debugPathDistance.setFont(debugFont);
    debugPathDistance.setCharacterSize(20);
    debugPathDistance.setFillColor(sf::Color::White);
    debugPathDistance.setPosition(10.f, 120.f);

    createLog(": Graph::initGui()", MazeLog::FileLogger::e_logType::LOG_INFO);
}

void Graph::initSound()
{
    // Music
    if (!this->music.openFromFile("Sounds/Blues.ogg"))
        std::cout << "ERROR: Music did not load!\n";

    music.setVolume(7);
    music.play();
    music.setLoop(true);
}

void Graph::initLogger()
{
    logger.initFile();
}

void Graph::initWindow()
{
    sf::VideoMode videoMode = sf::VideoMode(blockSize * gridSize, (blockSize * gridSize) + debugOffset);
    window = new sf::RenderWindow(videoMode, "Graph Maze", sf::Style::Close | sf::Style::Titlebar | sf::Style::Resize);
    window->setFramerateLimit(240);
    window->setVerticalSyncEnabled(false);

    createLog(": Graph::initWindow()", MazeLog::FileLogger::e_logType::LOG_INFO);
}

Graph::Graph(unsigned int size, float blockSize)
{
    gridSize = size;
    //Probably want to pass size down to initMatrix to create a proper sized matrix.

    initLogger();
    initGraph(blockSize);
    initMatrix();
    initWindow();
    initGui();
    initSound();
    initOutside();
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
        pollEvents(); // Listens for any events (eg: inputs)
        update(); // Updates the game
        render(); // Renders
    }
}

void Graph::update()
{
    updateGui();
}

void Graph::updateGui()
{
    // Grid Info 
    std::stringstream ssGridInfo;
    sf::Vector2i position = sf::Mouse::getPosition(*window);

    unsigned int row = (position.y >= debugOffset 
        && position.y <= debugOffset + (gridSize * blockSize)
        && position.x > 0 
        && position.x < (gridSize * blockSize)) // If y is higher than offset, put 0.
        ? (position.y - debugOffset) / blockSize : 0; 
    unsigned int col = (position.y >= debugOffset 
        && position.y <= debugOffset + (gridSize * blockSize)
        && position.x > 0 
        && position.x < (gridSize * blockSize)) // If y is higher than offset, put 0.
        ? (position.x / blockSize) : 0; 
    ssGridInfo << "ROW, COL:  " << row << ", " << col << '\n' <<
        '\n' <<
        "Start:         " << getStart()->row << "  " << getStart()->col << '\n' <<
        "End:            " << getEnd()->row << "  " << getEnd()->col;

    debugTextGridInfo.setString(ssGridInfo.str());

    // Hotkey Info 
    // Listen, I am not a UI artist... Spacing is hard
    std::stringstream ssHotkeyInfo;
    ssHotkeyInfo <<
        "Set Start:             S\n" <<
        "Set End:                E\n" <<
        "Disable Path:      V\n" <<
        "Enable Path:      C\n" <<
        "Reset Grid:           R\n" <<
        "DFS Search:        D\n" <<
        "BFS Search:        B\n" <<
        "a * Search:         a\n" <<
        "Create Maze:     M";

    debugTextHotKeyInfo.setString(ssHotkeyInfo.str());

    // Path Distance Info
    std::stringstream ssPathDistance;
    ssPathDistance << "Path Length: " << end->pathDistance;
    debugPathDistance.setString(ssPathDistance.str());
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
    window->draw(debugTextGridInfo);
    window->draw(debugTextHotKeyInfo);
    window->draw(debugPathDistance);
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
            if (ev.key.code == sf::Keyboard::Escape)        // Close
                window->close();
            else if (ev.key.code == sf::Keyboard::S)        // Set Start
                setStartSquare();
            else if (ev.key.code == sf::Keyboard::E)        // Set End
                setEndSquare();
            else if (ev.key.code == sf::Keyboard::B)        // BFS Explore
                BFSexplore();
            else if (ev.key.code == sf::Keyboard::D)        // DFS Explore
                DFSexplore();
            else if (ev.key.code == sf::Keyboard::R)        // Reset
                resetAllSquares();
            else if (ev.key.code == sf::Keyboard::V)        // Make Visited
                colorVisited(position.x, position.y - debugOffset);
            else if (ev.key.code == sf::Keyboard::C)        // Make Unvisited
                colorUnvisited(position.x, position.y - debugOffset);
            else if (ev.key.code == sf::Keyboard::M)        // Make Maze
                mazeCreator();
            else if (ev.key.code == sf::Keyboard::A)        // A* path search
                aStarExplore();
            break;
        case sf::Event::MouseButtonReleased:                // MAKE Visited, just for mouse prac
            if (ev.key.code == sf::Mouse::Left)
            {
                colorVisited(ev.mouseButton.x, ev.mouseButton.y - debugOffset);
            }
        }
    }
}

void Graph::initOutside()
{
    for (unsigned int i = 0; i < gridSize; ++i) {
        makeVisited(0, i);
        makeVisited(i, 0);
        makeVisited(gridSize - 1, i);
        makeVisited(i, gridSize - 1);
        render();
    }
}

void Graph::makeVisited(unsigned int row, unsigned int col)
{
    if (grid[row][col] != start && grid[row][col] != end) {
        if (grid[row][col]->isPath == true)
        {
            grid[row][col]->shape.setFillColor(sf::Color::White);
            grid[row][col]->isPath = false;
        }
    }
}

void Graph::makeUnvisited(unsigned int row, unsigned int col)
{
    if (grid[row][col] != start && grid[row][col] != end) {
        if (grid[row][col]->isPath == false)
        {
            grid[row][col]->shape.setFillColor(sf::Color::Black);
            grid[row][col]->isPath = true;
            setExplosionHole(row, col);
        }
    }
}


void Graph::colorVisited(const float& mouseXpos, const float& mouseYpos)
{
    unsigned int row = mouseYpos / blockSize;
    unsigned int col = mouseXpos / blockSize;

    if (col < gridSize && row < gridSize && mouseYpos  > 0)
    {
        makeVisited(row, col);
    }
}

void Graph::colorUnvisited(const float & mouseXpos, const float & mouseYpos)
{
    unsigned int row = mouseYpos / blockSize;
    unsigned int col = mouseXpos / blockSize;

    if (col < gridSize && row < gridSize && mouseYpos  > 0)
    {
        makeUnvisited(row, col);
    }
}

bool Graph::isValidNeighbor(unsigned int row, unsigned int col) const
{
    if (row >= 0 && row < gridSize &&
        col >= 0 && col < gridSize)
        return true;
    return false;
}

bool Graph::isNeighborsSet(unsigned int row, unsigned int col) const
{
    if (!isValidNeighbor(row, col))
        return false;
    else {
        if (grid[row][col]->haveNeighbors)
            return false;
    }
    return true;
}

// Second time, using recursive
void Graph::updateNeighborsRecursive(Vertex * vertex)
{
    // First set all the valid neighbors
    if (isValidNeighbor(vertex->row - 1, vertex->col)) //Top
        vertex->top = grid[vertex->row - 1][vertex->col];
    if (isValidNeighbor(vertex->row, vertex->col - 1)) //Left
        vertex->left = grid[vertex->row][vertex->col - 1];
    if (isValidNeighbor(vertex->row + 1, vertex->col)) //Bottom
        vertex->bottom = grid[vertex->row + 1][vertex->col];
    if (isValidNeighbor(vertex->row, vertex->col + 1)) //Right
        vertex->right = grid[vertex->row][vertex->col + 1];
    
    // This vertex/node now has neighbors set
    vertex->haveNeighbors = true;

    // Now recursively check which node to traverse to
    if (isNeighborsSet(vertex->row - 1, vertex->col)) // Checking Top
        updateNeighborsRecursive(vertex->top);
    if (isNeighborsSet(vertex->row, vertex->col - 1)) // Checking Left
        updateNeighborsRecursive(vertex->left);
    if (isNeighborsSet(vertex->row + 1, vertex->col)) // Checking Bot
        updateNeighborsRecursive(vertex->bottom);
    if (isNeighborsSet(vertex->row, vertex->col + 1)) // Checking Right
        updateNeighborsRecursive(vertex->right);
}

void Graph::setStartSquare()
{
    sf::Vector2i position = sf::Mouse::getPosition(*window);
    position.y -= debugOffset; // offset for debug window at top
    if (position.x / blockSize < gridSize && position.x > 0 && position.y / blockSize < gridSize && position.y > 0)
    {
        if (grid[position.y / blockSize][position.x / blockSize] != this->start)
        {
            Vertex * temp = start;
            
            this->start = grid[position.y / blockSize][position.x / blockSize]; 
            start->shape.setFillColor(sf::Color::Green);
            start->isPath = false;
            start->visited = true;

            temp->shape.setFillColor(sf::Color::Black);
            temp->isPath = true;
            temp->visited = false;
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
        if (grid[position.y / blockSize][position.x / blockSize] != this->end)
        {
            Vertex * temp = end;
            this->end = grid[position.y / blockSize][position.x / blockSize];

            end->shape.setFillColor(sf::Color::Red);

            temp->shape.setFillColor(sf::Color::Black);
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
            grid[i][j]->pathDistance = 0;
            grid[i][j]->explosionHole = false;
            grid[i][j]->aStarParent = nullptr;
            grid[i][j]->g_cost = INT_MAX;
            grid[i][j]->h_cost = INT_MAX;
            grid[i][j]->f_cost = INT_MAX;
        }
    }

    while (!pathStack.empty())
        pathStack.pop();
    while (!bfsQueue.empty())
        bfsQueue.pop();
    while (!dfsStack.empty())
        dfsStack.pop();
    while (!priorityQueue.empty())
        priorityQueue.pop_back();
    closedMap.clear();
    pathVec.clear();
    randomizeStartEnd();
    initOutside();
    endFound = false;

    // Logger.  Passing message as rvalue ref and log type
    createLog(": Graph::resetAllSquares()", MazeLog::FileLogger::e_logType::LOG_INFO);
}

void Graph::setDefaultStartEnd()
{
    start = grid[1][1];
    start->shape.setFillColor(sf::Color::Green);
    start->isPath = false;
    start->visited = true;

    end = grid[gridSize - 2][gridSize - 2];
    end->shape.setFillColor(sf::Color::Red);
}

void Graph::randomizeStartEnd()
{
    //Initialize Start
    int randRow = rand() % (gridSize / 2) + 1;
    int randCol = rand() % (gridSize / 2) + 1;
    start = grid[randRow][randCol];
    start->shape.setFillColor(sf::Color::Green);
    start->isPath = false;
    start->visited = true;

    //Initialize End
    int randRow2 = (rand() % (gridSize / 2)) + (gridSize / 2) - 1;
    int randCol2 = (rand() % (gridSize / 2)) + (gridSize / 2) - 1;
    end = grid[randRow2][randCol2];
    end->shape.setFillColor(sf::Color::Red);

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

unsigned int Graph::getPathDistance(const unsigned int & row, const unsigned int & col)
{
    if (col < gridSize && col >= 0 && row < gridSize && row >= 0) {
        return grid[row][col]->pathDistance;
    }
    return 0;
}

void Graph::colorPath(Vertex * vertex)
{
    if (vertex != start && vertex != end)
        vertex->shape.setFillColor(sf::Color::Color(255, 140, 0, 255));
}

void Graph::createPath(Vertex * node)
{
    /*
        * Use Recursion
        * Corner case when end is not found (end == 0)
        * Look for a neighbor that has -1 path distance
        * Stop when you find start
    */
    if (node->pathDistance == 0) {
        return;
    }
    else {
        if (node == start)
            return;

        std::vector<Vertex *> listNeighbors = {
            node->top,
            node->left,
            node->bottom,
            node->right
        };

        for (size_t i = 0; i < listNeighbors.size(); ++i) {
            if (listNeighbors[i]) {
                if (listNeighbors[i]->pathDistance == node->pathDistance - 1) {
                    pathStack.emplace(listNeighbors[i]);
                    createPath(listNeighbors[i]);
                    break;
                }
            }
        }
    }
}
 
void Graph::displayPath(std::stack<Vertex*> pathStack)
{
    while (!pathStack.empty()) {
        pathVec.emplace_back(pathStack.top());
        colorPath(pathVec.back());
        pathStack.pop();
        render();
    }
}

void Graph::aStarExplore()
{
    heapInsert(start);

    // Loop.  We will break when current node is end node
    while (true) {
        Vertex * currentNode = heapExtractMin();
        closedMap[cantor(currentNode->row, currentNode->col)] = currentNode;

        // Condition to break loop
        if (currentNode == end)
        {
            createAStarPath(end);
            break;
        }

        std::vector<Vertex *> listNeighbors = {
            currentNode->top,
            currentNode->left,
            currentNode->bottom,
            currentNode->right
        };
        
        for (size_t i = 0; i < listNeighbors.size(); ++i) {
            // If neighbor is not valid, go to next iteration
            
            if (!listNeighbors[i]->isPath)
                continue;

            // If neighbor is already in our closed list, go to next iteration
            if (closedMap.find(cantor(listNeighbors[i]->row, listNeighbors[i]->col)) != closedMap.end())
                continue;

            unsigned int temp_g_cost = currentNode->g_cost + 1; // No diagonals, so this "1" is constant

            // If temp g cost is less (means shorter path from start) OR it is already in priority queue.  EITHER CONDITION enters this
            // So i need to edit the temp g cost thing... To show that even tho g cost is same, this particular path is sub-optimal
            // vertex->g_cost = absDiff(vertex->row, start->row) + absDiff(vertex->col, start->col);
            if (temp_g_cost < listNeighbors[i]->g_cost || std::find(priorityQueue.begin(), priorityQueue.end(), listNeighbors[i]) == priorityQueue.end())
            {
                // Not in open (priority queue)
                listNeighbors[i]->aStarParent = currentNode;
                updateCosts(listNeighbors[i]);

                heapInsert(listNeighbors[i]);
                makeVisited(listNeighbors[i]);
                render();
            }
        }
    }
}

void Graph::BFSexplore()
{
    if (!start)
        std::cout << "ERROR: Set Start Square First!\n";
    else
        this->bfsQueue.emplace(start);

    while (!bfsQueue.empty() && !endFound)
    {
        Vertex * currentNode = bfsQueue.front();
        bfsQueue.pop();

        SearchBFSNeighbors(currentNode);
    } 

    createLog(": Graph::BFSexplore()", MazeLog::FileLogger::e_logType::LOG_INFO);

}

void Graph::makeVisited(Vertex * vertex)
{
    if (vertex != end)
        vertex->shape.setFillColor(sf::Color::Blue);
    vertex->visited = true;
}

void Graph::SearchBFSNeighbors(Vertex * currentNode)
{
    std::vector<Vertex *> listNeighbors = { 
        currentNode->left, 
        currentNode->bottom, 
        currentNode->right, 
        currentNode->top
    };

    for (size_t i = 0; i < listNeighbors.size(); ++i) 
    {
        // Check that neighbor exists and it's not visited and is valid path
        if (listNeighbors[i] && !listNeighbors[i]->visited && listNeighbors[i]->isPath)
        {
            makeVisited(listNeighbors[i]);
            listNeighbors[i]->pathDistance = currentNode->pathDistance + 1;
            bfsQueue.emplace(listNeighbors[i]);

            if (listNeighbors[i] == end) {
                endFound = true;
                break;
            }
        }
    }
    render();

    createPath(end);
    displayPath(pathStack);
}

// Careful here, we're swapping the elements of priority queue
void Graph::swap(unsigned int index1, unsigned int index2)
{
    if (priorityQueue.size() == 0)
        return;
    Vertex * temp = priorityQueue[index1];
    priorityQueue[index1] = priorityQueue[index2];
    priorityQueue[index2] = temp;
}

void Graph::heapInsert(Vertex * vertex)
{
    priorityQueue.emplace_back(vertex);
    unsigned int index = priorityQueue.size() - 1;

    // This loop climbs up to the top!
    while (index != 0 && priorityQueue[index]->f_cost < priorityQueue[getParent(index)]->f_cost)
    {
        swap(index, getParent(index));
        index = getParent(index);
    }
}

void Graph::MinHeapify(unsigned int index)
{
    unsigned int leftChildIndex = getLeftChild(index);
    unsigned int rightChildIndex = getRightChild(index);
    unsigned int smallest = index;

    // Get smaller of left and right.  Need to check against size so you don't get vector subscript out of range
    if (leftChildIndex < priorityQueue.size() && priorityQueue[leftChildIndex]->f_cost < priorityQueue[index]->f_cost)
        smallest = leftChildIndex;

    if (rightChildIndex < priorityQueue.size() && priorityQueue[rightChildIndex]->f_cost < priorityQueue[index]->f_cost)
        smallest = rightChildIndex;

    // Now do a loop ONLY if one of the children is indeed smaller
    if (smallest != index)
    {
        swap(index, smallest);
        MinHeapify(smallest);
    }
    
}

Vertex * Graph::heapExtractMin()
{
    if (priorityQueue.size() <= 0)
        return nullptr;
    else if (priorityQueue.size() == 1) {
        Vertex * temp = priorityQueue[0];
        priorityQueue.pop_back();
        return temp;
    }
    else {
        Vertex * temp = priorityQueue[0]; // Get last element
        priorityQueue[0] = priorityQueue[priorityQueue.size() - 1];
        priorityQueue.pop_back();
        
        // This sorts
        MinHeapify(0);

        // For all the ones sorted with fcost, we need to prioritize ones with lowest hcost
        for (int i = 1; i < priorityQueue.size(); ++i)
        {
            if (priorityQueue[i]->h_cost < priorityQueue[0]->h_cost)
                swap(i, 0);
        }

        return temp;
    }
}

unsigned int Graph::absDiff(const unsigned int & valueOne, const unsigned int & valueTwo)
{
    int result = valueOne - valueTwo;
    return result = (result < 0) ? result * (-1) : result;
}

void Graph::updateCosts(Vertex * vertex)
{
    // G cost is distance from start
    vertex->g_cost = absDiff(vertex->row, start->row) + absDiff(vertex->col, start->col);

    // H cost is distance from end
    vertex->h_cost = absDiff(vertex->row, end->row) + absDiff(vertex->col, end->col);

    // F cost is G + H
    vertex->f_cost = vertex->g_cost + vertex->h_cost;
}

// TODO
// When there is no path, cantor algo fails
// Path distance value does not show
void Graph::createAStarPath(Vertex * temp)
{
    while (temp->aStarParent) {
        pathStack.push(temp->aStarParent);
        temp = temp->aStarParent;
    }

    unsigned int counter = 0;

    while (!pathStack.empty())
    {
        colorPath(pathStack.top());
        pathStack.pop();
        render();
        ++counter;
    }
    end->pathDistance = counter;
    
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

            render();
            DFSrecurse(listNeighbors[i], dfsStack);
        }
    }
}

void Graph::mazeCreator()
{
    mazeCreatorRecursive(grid[0][0], grid[gridSize - 1][gridSize - 1]);
}

unsigned int Graph::randMazeVal(unsigned int length) const
{
    unsigned int randomVal = rand() % 2; // 0, 1
    unsigned int val;
    if (randomVal == 0)
        return (length / 2);
    else 
        return (length / 2) + 1;
}

unsigned int Graph::holeMaker(unsigned int RangeOne, unsigned int RangeTwo)
{
    // Example: Between 25 and 63 >   25 + ( std::rand() % ( 63 - 25 + 1 ) )
    // I am using 25 + 1 + ( std::rand() % ( 63 - 25) ) to not include 25 and 63
    return RangeOne + 1 + (rand() % (RangeTwo - RangeOne - 1));
}

void Graph::drawQuadrantLines(Vertex * topLeft, Vertex * botRight, unsigned int midHorizantal, unsigned int midVertical)
{ 
    // Make line top to bottom.  COL Doesn't change!
    for (unsigned int i = topLeft->row + 1; i < botRight->row; ++i) {
        if (grid[i][midHorizantal]->explosionHole == false)
            makeVisited(i, midHorizantal);
        render();
    }

    // Make line left to right.  ROW Doesn't change!
    for (unsigned int i = topLeft->col + 1; i < botRight->col; ++i) {
        if (grid[midVertical][i]->explosionHole == false)
            makeVisited(midVertical, i);
        render();
    }

    switch (mazeDivideCounter) {
    case 0: // Hole on top, right, bottom.  Ignore Left
        makeUnvisited(holeMaker(topLeft->row, midVertical), midHorizantal); // Make a hole on top divider
        makeUnvisited(holeMaker(midVertical, botRight->row), midHorizantal); // Make a hole on bot divider
        makeUnvisited(midVertical, holeMaker(midHorizantal, botRight->col)); // Make a hole on right divider
        ++mazeDivideCounter;
        render();
        break;
    case 1: // Ignore Top
        makeUnvisited(midVertical, holeMaker(topLeft->col, midHorizantal)); // Make a hole on left divider
        makeUnvisited(holeMaker(midVertical, botRight->row), midHorizantal); // Make a hole on bot divider
        makeUnvisited(midVertical, holeMaker(midHorizantal, botRight->col)); // Make a hole on right divider
        ++mazeDivideCounter;
        render();
        break;
    case 2: // Ignore Right
        makeUnvisited(holeMaker(topLeft->row, midVertical), midHorizantal); // Make a hole on top divider
        makeUnvisited(midVertical, holeMaker(topLeft->col, midHorizantal)); // Make a hole on left divider
        makeUnvisited(holeMaker(midVertical, botRight->row), midHorizantal); // Make a hole on bot divider
        ++mazeDivideCounter;
        render();
        break;
    case 3: // Ignore Bot
        makeUnvisited(holeMaker(topLeft->row, midVertical), midHorizantal); // Make a hole on top divider
        makeUnvisited(midVertical, holeMaker(topLeft->col, midHorizantal)); // Make a hole on left divider
        makeUnvisited(midVertical, holeMaker(midHorizantal, botRight->col)); // Make a hole on right divider
        mazeDivideCounter = 0;
        render();
        break;
    }
}

void Graph::mazeCreatorRecursive(Vertex * topLeft, Vertex * botRight)
{
    /*
        1. Divide into quadrants, with some randomness (Just a 0 or 1 offset)
        2. Put 1 space in 3 out of the 4 lines
        3. recurse quadtree. Top left, top right, bot left, bot right
        4. Exit condition is if the area is X grids or less
    */
    
    unsigned int midHorizantal = topLeft->col + randMazeVal(botRight->col - topLeft->col); // To Help Create Lines
    unsigned int midVertical = topLeft->row + randMazeVal(botRight->row - topLeft->row);
    int areaSize = ((topLeft->col - botRight->col) * (topLeft->row - botRight->row));
    if (areaSize < 30) // If quadrant is less than x size
        return;
    else {
        drawQuadrantLines(topLeft, botRight, midHorizantal, midVertical); // Creates Lines with Spaces on 3/4 sections
        mazeCreatorRecursive(topLeft, grid[midVertical][midHorizantal]); // Top left quadrant
        mazeCreatorRecursive(grid[topLeft->row][midHorizantal], grid[midVertical][botRight->col]); // Top right quadrant
        mazeCreatorRecursive(grid[midVertical][topLeft->col], grid[botRight->row][midHorizantal]); // Bot left quadrant
        mazeCreatorRecursive(grid[midVertical][midHorizantal], botRight); // Bot right quadrant
    }
}

void Graph::setExplosionHole(unsigned int row, unsigned int col)
{
    grid[row][col]->explosionHole = true;
    grid[row+1][col]->explosionHole = true;
    grid[row-1][col]->explosionHole = true;
    grid[row][col+1]->explosionHole = true;
    grid[row][col-1]->explosionHole = true;
}