#include <iostream>
#include <raylib.h>
#include <deque>
#include <vector>
#include <raymath.h>
#include <ctime>
#include <algorithm>
#include <random>
using namespace std;

Color green = { 173, 204, 96 , 255 };
Color darkGreen = { 43, 51, 24, 255 };
Color maroon = { 190, 33, 55, 255 };
Color darkgray = { 80, 80, 80, 255 };

// Main game variables
int cellSize = 30;
int cellCount = 25;
int offset = 75;

// Maze variables
#define CELL_SIZE 25;
const int screenWidth = 750;
const int screenHeight = 750;
double lastUpdateTime = 0;

bool ElementInDeque(Vector2 element, const deque<Vector2>& deque)
{
    for (size_t i = 0; i < deque.size(); i++)
    {
        if (Vector2Equals(deque[i], element))
        {
            return true;
        }
    }
    return false;
}

bool eventTriggered(double interval, double& lastUpdateTime)
{
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval)
    {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

// Snake class
class Snake
{
public:
    deque<Vector2> body = { Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9} };
    Vector2 direction = { 1, 0 };
    Vector2 checkpoint = { 0, 0 }; //Default checkpoint (starting position)
    bool addSegment = false;

    void Draw()
    {
        for (size_t i = 0; i < body.size(); i++)
        {
            float x = body[i].x;
            float y = body[i].y;
            Rectangle segment = { offset + x * cellSize, offset + y * cellSize, (float)cellSize, (float)cellSize };
            DrawRectangleRounded(segment, 0.5, 6, darkGreen);
        }
    }

    void Update()
    {
        body.push_front(Vector2Add(body[0], direction));
        if (!addSegment)
        {
            body.pop_back();
        }
        addSegment = false;
    }

    void Reset()
    {
        body = { Vector2{0, 0}, Vector2{0, 0}, Vector2{0, 0} };
        direction = { 1, 0 };
    }
};

// Maze class
class Maze
{
public:
    vector<Rectangle> walls;
    vector<vector<bool>> grid; //2D grid to store wall (true = wall, fasle = free)

    //Set start & end destinations for maze
    Vector2 startPosition = { 0, 0 }; // Top-Left corner
    Vector2 endPosition = { (float)(cellCount - 1), (float)(cellCount - 1) }; //Bottom-right corner
    Maze() { grid.resize(cellCount, vector<bool>(cellCount, true)); }

    void Generate()
    {
        walls.clear();
        grid.assign(cellCount, vector<bool>(cellCount, true)); //Reset grid

        // Depth First Search Maze Generation
        srand(time(0));
        GenerateMazeDFS(0, 0);


        //Convert grid walls to rectangle format
        for (int y = 0; y < cellCount; ++y)
        {
            for (int x = 0; x < cellCount; ++x)
            {
                if (grid[y][x])
                {
                    walls.push_back(Rectangle{
                    (float)offset + x * cellSize,
                    (float)offset + y * cellSize,
                    (float)cellSize,
                    (float)cellSize
                        });
                    /*cout << "Added wall at: (" << x << ", " << y << ")" << endl;*/
                }
            }
        };
        grid[0][0] = grid[cellCount - 1][cellCount - 1] = false; //Clear start and end positions


    }

    void GenerateMazeDFS(int x, int y)
    {
        grid[y][x] = false; //Mark current cell as free

        // Randomized direction: UP, DOWN , LEFT, RIGHT
        vector<Vector2> directions = { {0, -1 }, { 0,1 }, { -1, 0 }, { 1,0 } };

        //Shuffle direction to ensure randomness
        static random_device rd;
        static default_random_engine rng(rd());
        shuffle(directions.begin(), directions.end(), rng);

        for (const auto& dir : directions)
        {
            int nx = x + dir.x * 2; //Move 2 cells in direction
            int ny = y + dir.y * 2;

            if (nx >= 0 && ny >= 0 && nx < cellCount && ny < cellCount && grid[ny][nx])
            {
                grid[y + dir.y][x + dir.x] = false; //  Remove wall between current and next cell
                GenerateMazeDFS(nx, ny);
            }

        }
    }

    bool IsWall(Vector2 pos) const
    {
        return grid[(int)pos.y][(int)pos.x];
    }

    void Draw()
    {
        for (const auto& wall : walls)
        {
            DrawRectangleRec(wall, MAROON);

        }

        // Draw the start & end position
        DrawRectangle(offset + startPosition.x * cellSize, offset + startPosition.y * cellSize, cellSize, cellSize, GREEN);
        DrawRectangle(offset + endPosition.x * cellSize, offset + endPosition.y * cellSize, cellSize, cellSize, RED);
    }
};

// Food class
class Food
{
public:
    Vector2 position;
    Texture2D texture;

    Food(const deque<Vector2>& snakeBody, const Maze& maze) : position({ -1, -1 })
    {
        Image image = LoadImage("Graphics/watermelon.png");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        /*position = GenerateRandomPos(snakeBody, maze);*/
    }

    ~Food()
    {
        UnloadTexture(texture);
    }

    void Draw()
    {
        DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
    }

    Vector2 GenerateRandomCell()
    {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{ x, y };

    }
        bool IsValidPositioin(Vector2 pos) const {
            return pos.x >= 0 && pos.x < cellCount && pos.y >= 0 && pos.y < cellCount;
        
    }

    Vector2 GenerateRandomPos(const deque<Vector2>& snakeBody, const Maze& maze, const vector<Food>& existingFoods)
    {
        Vector2 position = GenerateRandomCell();
        bool isValidPosition = false;

        while (!isValidPosition)
        {
            position = GenerateRandomCell();

            // Ensure position is not on the snake, not a wall, and not overlapping with existing foods
            if (!ElementInDeque(position, snakeBody) &&
                !maze.IsWall(position) &&
                find_if(existingFoods.begin(), existingFoods.end(), [&](const Food& food) {
                    return Vector2Equals(food.position, position);
                    }) == existingFoods.end())
            {
                isValidPosition = true;
            }
        }
        return position;
    }
};

//Key class
class Key
{
public:
    Vector2 position;
    Texture2D texture;
    Vector2 checkpoint = { 0,0 }; //Default checkpoint (starting position)



    Key(const deque<Vector2>& snakeBody, const Maze& maze) : position({ -1, -1 })
    {
        Image image = LoadImage("Graphics/key.png");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
    }

    ~Key()
    {
        UnloadTexture(texture);
    }

    void Draw()
    {
        DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
    }

    Vector2 GenerateRandomCell()
    {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{ x, y };

    }
        bool IsValidPositioin(Vector2 pos) const {
            return pos.x >= 0 && pos.x < cellCount && pos.y >= 0 && pos.y < cellCount;
    }

    Vector2 GenerateRandomPos(const deque<Vector2>& snakeBody, const Maze& maze, const vector<Key>& existingKeys)
    {
        Vector2 position = GenerateRandomCell();
        bool isValidPosition = false;

        while (!isValidPosition)
        {
            position = GenerateRandomCell();

            // Ensure position is not on the snake, not a wall, and not overlapping with existing foods
            if (!ElementInDeque(position, snakeBody) &&
                !maze.IsWall(position) &&
                find_if(existingKeys.begin(), existingKeys.end(), [&](const Key& key) {
                    return Vector2Equals(key.position, position);
                    }) == existingKeys.end())
            {
                isValidPosition = true;
            }
        }
        return position;
    }
};

// Game class
class Game
{
public:
    Snake snake;
    vector<Food> foods;
    Food food;
    Maze maze;
    vector<Key> keys;
    Key key;

    Sound eatSound;
    Sound wallSound;
    Sound uiSound;
    Sound menuSound;
    Sound gameOverSound;
    Sound completeSound;

    bool running = true;
    int difficulty;
    int level = 1;
    int score = 0;
    int currentScore = score;
    float timer = 60.f;
    float timeLimit = timer;
    bool timerActive = true;
    bool isLevelComplete = false;
    bool isLevelUncomplete = false;
    bool showCompletionMessage = false;


    Game(int diff) : difficulty(diff), maze(), food(snake.body, maze),
        key(snake.body, maze)
    {
        InitAudioDevice();

        //Game sounds
        menuSound = LoadSound("Sounds/mini1111.wav");
        uiSound = LoadSound("Sounds/ui_select.wav");
        eatSound = LoadSound("Sounds/eat.wav");
        wallSound = LoadSound("Sounds/spring.wav");
        gameOverSound = LoadSound("Sounds/weird.wav");
        completeSound = LoadSound("Sounds/complete.wav");

        //Classes Methods
        maze.Generate();
    }

    ~Game()
    {
        UnloadTexture(food.texture);
        for (const auto& food : foods)
        {
            UnloadTexture(food.texture);
        }

        UnloadSound(menuSound);
        UnloadSound(uiSound);
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        UnloadSound(gameOverSound);
        UnloadSound(completeSound);
        CloseAudioDevice();
    }

    //Multiple foods function
    void InitializeFoods(int foodCount)
    {
        foods.clear();
        for (int i = 0; i < foodCount; i++)
        {
            Food newFood(snake.body, maze);
            newFood.position = newFood.GenerateRandomPos(snake.body, maze, foods);
            foods.push_back(newFood);
            cout << "Food " << i << " generated at position: (" << newFood.position.x << ", " << newFood.position.y << ")" << endl;
        }
    }

    //New level
    void StartNewLevel()
    {
        level++;
        timer = timeLimit;        // Reset timer
        maze.Generate();          // Generate a new maze
        InitializeFoods(level + 3); // Add more food with higher levels
        snake.Reset();            // Reset snake position
        food.position = food.GenerateRandomPos(snake.body, maze, foods);
        key.position = key.GenerateRandomPos(snake.body, maze, keys);
    }

    void Draw()
    {
        maze.Draw();
        food.Draw();
        snake.Draw();
        key.Draw();
        DrawCompletionMessage();
        DrawOverMessage();
        DrawTimer();
    }

    void Update()
    {
        if (running)
        {
            snake.Update();
            CheckCollisionWithFood();
            CheckCollisionWithEdges();
            CheckCollisionWithTail();
            CheckCollisionWithKey();
        }

        //Handle level completion
        if (isLevelComplete)
        {
            StartNewLevel();
            isLevelComplete = false; //Reset completion status
        }
    }

    void UpdateTimer(float deltaTime)
    {
        if (timer > 0 && running && !isLevelComplete)
        {
            timer -= deltaTime; //Subtract time passed

            if (timer <= 0)
            {
                timer = 0; //Prevent negative timer
            }
        }
    }

    void CheckLevelCompletion() {

        if (Vector2Equals(snake.body[0], maze.endPosition)) {
            running = false;
            isLevelComplete = true; // Mark the level as complete
            PlaySound(completeSound);
            
        }
    }

    void CheckLevelIncompletion() {

        if (timer == 0 && !running) {
            isLevelUncomplete = true; // Mark the level as incomplete
        }
    }

    void DrawTimer()
    {
        int baseTimerWidth = 200;
        int baseTimerHeight = 20;

        // Adjust timer dimensions based on difficulty
        int timerWidth = baseTimerWidth - (difficulty * 50); //Widht of the timer bar
        int timerHeight = baseTimerHeight; //Height of the timer bar


        int timerX = screenWidth - timerWidth - timerWidth - 20; // Centered horizontally
        int timerY = 10; //position near the top of the screen

        // Claculate the percentage of the time remaining
        float timeRatio = timer / timeLimit;
        int currentWidth = static_cast<int>(timeRatio * timerWidth);

        //Draw the background bar (full width)
        DrawRectangle(timerX, timerY, timerWidth, timerHeight, GRAY);

        //Draw the timer bar based on remainig time
        DrawRectangle(timerX, timerY, currentWidth, timerHeight, RED);

        //Display the time remaining as text
        DrawText(TextFormat("Time: %.1f", timer),
            timerX + timerWidth / 2 - 40, timerY + 25, 20, BLACK);

    }

    //Draw completion message
    void DrawCompletionMessage() {
        if (isLevelComplete) {
            DrawText("LEVEL COMPLETE!", screenWidth / 2 - 150, screenHeight / 2, 80, GREEN);
            PlaySound(completeSound);
        }
    }
    //Draw game over message
    void DrawOverMessage() {
        if (isLevelUncomplete) {
            DrawText("GAME OVER!", screenWidth / 2 - 150, screenHeight / 2, 80, BLACK);
            PlaySound(gameOverSound);
            StopSound(gameOverSound);
        }
    }

    void CheckCollisionWithFood()
    {
        if (Vector2Equals(snake.body[0], food.position))
        {
            food.position = food.GenerateRandomPos(snake.body, maze, foods);
            snake.addSegment = true;
            score++;
            PlaySound(eatSound);
        }
    }

    void CheckCollisionWithKey()
    {
        if (Vector2Equals(snake.body[0], key.position))
        {
            //Set checkpoint to current head position
            key.position = key.GenerateRandomPos(snake.body, maze, keys);
            snake.checkpoint = snake.body[0];
            PlaySound(eatSound);
        }
    }

    void CheckCollisionWithEdges()
    {
        if (snake.body[0].x < 0 || snake.body[0].x >= cellCount ||
            snake.body[0].y < 0 || snake.body[0].y >= cellCount)
        {
            GameOver();
        }

        Rectangle snakeHead = { offset + snake.body[0].x * cellSize, offset + snake.body[0].y * cellSize, (float)cellSize, (float)cellSize };
        for (const auto& wall : maze.walls)
        {
            /*Rectangle snakeHead = { offset + snake.body[0].x * cellSize, offset + snake.body[0].y * cellSize, (float)cellSize, (float)cellSize };*/
            if (CheckCollisionRecs(snakeHead, wall))
            {
                food.position = food.GenerateRandomPos(snake.body, maze, foods);
                key.position = key.GenerateRandomPos(snake.body, maze, keys);
                GameOver();
            }
        }
    }

    void CheckCollisionWithTail()
    {
        deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();
        if (ElementInDeque(snake.body[0], headlessBody))
        {
            GameOver();
        }
    }

    void GameOver()
    {
        snake.Reset();
        snake.body = { snake.checkpoint, Vector2Add(snake.checkpoint, Vector2{ 0, 0}), Vector2Add(snake.checkpoint, Vector2{0,0}) };
        snake.direction;
        running = false;
        currentScore = currentScore;
        PlaySound(wallSound);
    }

    void gameMenu()
    {
        enum MenuOption { START, DIFFICULTY, EXIT, NUM_OPTIONS };
        const char* menuOptions[NUM_OPTIONS] = { "Start Game", "Difficulty", "Exit" };
        const char* difficulties[3] = { "Easy", "Medium", "Hard" };
        int difficulty = 0; // Default: Easy (0=Easy, 1=Medium, 2=Hard)
        int selectedOption = 0;
        float menuVolume = 0.5f; // Initial volume (50%)

        PlaySound(menuSound);
        SetSoundVolume(menuSound, menuVolume);
        while (!WindowShouldClose()) {
            if (!IsSoundPlaying(menuSound)) {
                PlaySound(menuSound);
            }

            // Menu controls
            if (IsKeyPressed(KEY_ESCAPE)) {
                CloseWindow();
                exit(0);
            }
            if (IsKeyPressed(KEY_DOWN)) {
                selectedOption = (selectedOption + 1) % NUM_OPTIONS;
                PlaySound(uiSound);
            }
            if (IsKeyPressed(KEY_UP)) {
                selectedOption = (selectedOption - 1 + NUM_OPTIONS) % NUM_OPTIONS;
                PlaySound(uiSound);
            }
            if (IsKeyPressed(KEY_LEFT)) {
                menuVolume -= 0.1f; // Decrease volume
                if (menuVolume < 0.0f) menuVolume = 0.0f;
                SetSoundVolume(menuSound, menuVolume);
                PlaySound(uiSound); // Feedback sound
            }
            if (IsKeyPressed(KEY_RIGHT)) {
                menuVolume += 0.1f; // Increase volume
                if (menuVolume > 1.0f) menuVolume = 1.0f;
                SetSoundVolume(menuSound, menuVolume);
                PlaySound(uiSound); // Feedback sound
            }

            if (selectedOption == DIFFICULTY && IsKeyPressed(KEY_ENTER)) {
                difficulty = (difficulty + 1) % 3; // Cycle through difficulties
                PlaySound(uiSound);
            }
            if (IsKeyPressed(KEY_ENTER)) {
                PlaySound(uiSound);
                if (selectedOption == START) {
                    UnloadSound(menuSound);
                    this->ApplyDifficulty(difficulty);
                    break; // Exit the menu to start the game
                } else if (selectedOption == EXIT) {
                    CloseWindow();
                    exit(0);
                }
            }

            // Drawing the menu
            BeginDrawing();
            ClearBackground(darkgray);
            DrawText("Main Menu", screenWidth / 2 - MeasureText("Main Menu", 40) / 2, 50, 60, BLACK);
            for (int i = 0; i < NUM_OPTIONS; i++) {
                Color color = (i == selectedOption) ? RED : BLACK;
                DrawText(menuOptions[i], screenWidth / 2 - MeasureText(menuOptions[i], 20) / 2, 150 + i * 40, 40, color);
            }

            // Display volume
            DrawText("Volume:", screenWidth / 2 - 100, 300, 30, BLACK);
            DrawText(TextFormat("%i%%", (int)(menuVolume * 100)), screenWidth / 2 + 50, 300, 30, RED);

            //Display difficulty
            DrawText("Difficulty:", screenWidth / 2 - 100, 400, 30, BLACK);
            DrawText(difficulties[difficulty], screenWidth / 2 + 50, 400, 30, RED);
            EndDrawing();
        }
    }

    void ApplyDifficulty(int difficulty) {
        switch (difficulty) {
        case 0: // Easy
            timer = 40.0f;
            cellCount = 15;
            InitializeFoods(3);
            break;
        case 1: // Medium
            timer = 60.0f;
            cellCount = 25;
            InitializeFoods(5);
            break;
        case 2: // Hard
            timer = 80.0f;
            cellCount = 35;
            InitializeFoods(7);
            break;
        default:
            0;
            break;
        }
        maze = Maze();
        /*snake.Reset();*/
        SetWindowSize(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount);
        maze.Generate();
        snake.Reset();
    }
};

int main()
{
    cout << "Starting the game... " << endl;
    InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Mystery Maze");
    SetTargetFPS(60); // Make sure game is running at 60FPS

    Game game = Game(1);

    // Show the game menu
    game.gameMenu();

    //Main Game loop
    while (WindowShouldClose() == false)
    {

        BeginDrawing(); // Begin drawing

        if (eventTriggered(0.2, lastUpdateTime))
        {
            game.Update();
        }

        float deltaTime = GetFrameTime(); // Get time since last frame
        game.UpdateTimer(deltaTime); // Update the timer


        if (IsKeyPressed(KEY_M)) {
            game.running = false;
            game.gameMenu();
        }

        // Controls for snake
        if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1 && game.timer > 0)
        {
            game.snake.direction = { 0, -1 };
            game.running = true;
        }
        if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1 && game.timer > 0)
        {
            game.snake.direction = { 0, 1 };
            game.running = true;
        }
        if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1 && game.timer > 0)
        {
            game.snake.direction = { -1, 0 };
            game.running = true;
        }
        if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1 && game.timer > 0)
        {
            game.snake.direction = { 1, 0 };
            game.running = true;
        }


        //Check for level completion
        game.CheckLevelCompletion();
        game.CheckLevelIncompletion();

        // Draw the level completion message if applicable
        if (game.isLevelComplete) {
            game.DrawCompletionMessage();
            /*game.NewLevel();*/
        }
        if (game.isLevelUncomplete) {
            game.DrawOverMessage();
        }


        //Drawing the classes
        ClearBackground(green);


        DrawRectangleLinesEx(Rectangle{ (float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10 }, 5, darkGreen);
        DrawText("Mystery Maze", offset - 5, 20, 40, darkGreen);
        DrawText(TextFormat("%i", game.score), offset - 5, offset + cellSize * cellCount + 10, 40, darkGreen);
        DrawText(TextFormat("Level: %d", game.level), offset + 610, offset + cellSize * cellCount + 10, 40, darkGreen);
        game.Draw();

        EndDrawing();
    }
    CloseWindow();
    return 0;
}