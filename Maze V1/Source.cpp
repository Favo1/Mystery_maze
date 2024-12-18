// MR F E GABRIEL-OLUWATOBI
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
#define CELL_SIZE 25
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
        Vector2 startPosition = { 0, 0 };
        Vector2 endPosition = { (float)(cellCount - 1), (float)(cellCount - 1) }; //Bottom-right corner;
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
                        (float)cellSize,
                            });
                        cout << "Added wall at: (" << x << ", " << y << ")" << endl;
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

            // Clear the center for the snake to spawn
            int centerX = cellCount / 2;
            int centerY = cellCount / 2;
            grid[centerY][centerX] = false;
            

            
        }

        bool IsWall( Vector2 pos)
        {
            return grid[(int)pos.y][(int)pos.x];
        }

        void Draw()
        {
            for (const auto& wall: walls)
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

    /*Food(){}*/

    Food(const deque<Vector2>& snakeBody, const Maze& maze)
    {
        Image image = LoadImage("Graphics/apple.png");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        position = GenerateRandomPos(snakeBody, maze);
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

    Vector2 GenerateRandomPos(const deque<Vector2>& snakeBody, const Maze& maze)
    {
        Vector2 position = GenerateRandomCell();
        while (ElementInDeque(position, snakeBody))
        {
            position = GenerateRandomCell();
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
    bool running = true;
    int score = 0;
    Sound eatSound;
    Sound wallSound;
    Sound uiSound;
    Sound menuSound;
    //Multiple foods function

     void InitializeFoods(int foodCount)
     {
         foods.clear();
         for(int i = 0; i < foodCount; i++)
         {
             foods.push_back(Food(snake.body, maze));
         }
     }
     
    Game() : maze(), food(snake.body, maze)
    {
        InitAudioDevice();
        menuSound = LoadSound("Sounds/mini1111.wav");
        uiSound = LoadSound("Sounds/UI_Select.wav");
        eatSound = LoadSound("Sounds/eat.wav");
        wallSound = LoadSound("Sounds/spring.wav");
        
        maze.Generate();
        InitializeFoods(3);
    }

    ~Game()
    {
        UnloadSound(menuSound);
        UnloadSound(uiSound);
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        CloseAudioDevice();
        
        for(const auto& food : foods)
        {
            UnloadTexture(food.texture);
        }
    }

    void Draw()
    {
       
        food.Draw();
        snake.Draw();
        maze.Draw();
    }

    void Update()
    {
        if (running)
        {
            snake.Update();
            CheckCollisionWithFood();
            CheckCollisionWithEdges();
            CheckCollisionWithTail();
        }
    }

    void CheckCollisionWithFood()
    {
        if (Vector2Equals(snake.body[0], food.position))
        {
            food.position = food.GenerateRandomPos(snake.body, maze);
            snake.addSegment = true;
            score ++;
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
        food.position = food.GenerateRandomPos(snake.body, maze);
		running = false;
        score = 0;
        PlaySound(wallSound);
	}



	void gameMenu()
	{
		enum MenuOption { START, EXIT, NUM_OPTIONS };
		const char* menuOptions[NUM_OPTIONS] = { "Start Game", "Exit" };
		int selectedOption = 0;
        float menuVolume = 0.5f; // Initial volume (50%)
        float uiVolume = 0.5f;

        PlaySound(menuSound);
        SetSoundVolume(menuSound, menuVolume);
        SetSoundVolume(uiSound, uiVolume);
		while (!WindowShouldClose()) {
			// Menu controls
            
            if (IsKeyPressed(KEY_ESCAPE)) {
                CloseWindow();
                exit(0);//exit the program 
            }
			if (IsKeyPressed(KEY_DOWN)) {
				selectedOption = (selectedOption + 1) % NUM_OPTIONS; // Move down
                PlaySound(uiSound);
			}
			if (IsKeyPressed(KEY_UP)) {
				selectedOption = (selectedOption - 1 + NUM_OPTIONS) % NUM_OPTIONS; // Move up
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
			if (IsKeyPressed(KEY_ENTER)) {
                PlaySound(uiSound);
				if (selectedOption == START) {
                    PlaySound(uiSound);
                    UnloadSound(menuSound);
					break; // Exit the menu to start the game
				}
				else if (selectedOption == EXIT) {
                    PlaySound(uiSound);
					CloseWindow(); // Exit the game
					exit(0);       // End the program
				}
               
			}
            
			// Drawing the menu
			BeginDrawing();
			ClearBackground(darkgray);
			DrawText("Main Menu", screenWidth / 2 - MeasureText("Main Menu", 40) / 2, 50, 60, BLACK);

			for (int i = 0; i < NUM_OPTIONS; i++) {
                
				Color color = (i == selectedOption) ? RED : BLACK; // Highlight selected option
				DrawText(menuOptions[i], screenWidth / 2 - MeasureText(menuOptions[i], 20) / 2, 150 + i * 40, 40, color);
			}
            // Display volume
            DrawText("Volume:", screenWidth / 2 - 100, 300, 30, BLACK);
            DrawText(TextFormat("%i%%", (int)(menuVolume * 100)), screenWidth / 2 + 50, 300, 30, RED);
			EndDrawing();
		}
	}


};

	int main()
	{	
		cout << "Starting the game... " << endl;
		InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Mystery Maze");
		SetTargetFPS(60); // Make sure game is running at 60FPS

		Game game = Game();
	
		// Show the game menu
		game.gameMenu();

		//Main Game loop
		while (WindowShouldClose() == false)
		{
			
			BeginDrawing();
			
			if (eventTriggered(0.2, lastUpdateTime))
			{
				game.Update();
			}

			// Controls for our sanke
			if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1)
			{
				game.snake.direction = { 0, -1 };
				game.running = true;
			}
			if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1)
			{
				game.snake.direction = { 0, 1 };
				game.running = true;
			}
			if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1)
			{
				game.snake.direction = { -1, 0 };
				game.running = true;
			}
			if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1)
			{
				game.snake.direction = { 1, 0 };
				game.running = true;
			}
			//Drawing the classes
			ClearBackground(green);


			DrawRectangleLinesEx(Rectangle{ (float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10 }, 5, darkGreen);
            DrawText("Mystery Maze", offset - 5, 20, 40, darkGreen);
            DrawText(TextFormat("%i", game.score), offset - 5, offset + cellSize * cellCount + 10, 40, darkGreen);
            game.Draw();

			EndDrawing();
		}
		CloseWindow();
		return 0;
	}