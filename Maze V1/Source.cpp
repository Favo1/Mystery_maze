// MR F E GABRIEL-OLUWATOBI
#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>
#include <vector>

using namespace std;

Color green = {173, 204, 96 , 255};
Color darkGreen = {43, 51, 24, 255};

int cellSize = 30;
int cellCount = 25;
int offset = 75;

double lastUpdateTime = 0;

bool ElementInDeque(Vector2 element, deque<Vector2> deque) 
{
	for (unsigned int i = 0; i < deque.size(); i++) 
	{
		if (Vector2Equals(deque[i], element)) 
		{
			return true;
		}
		return false;
	}
}
bool eventTriggered(double interval)
{
	double currentTime = GetTime();
		if(currentTime - lastUpdateTime >= interval)
		{
			lastUpdateTime = currentTime;
			return true;
		}
		return false;
}

// This is my class for the pc
class Snake
{
public:
	deque<Vector2> body = {Vector2{6,9}, Vector2{5,9}, Vector2{4,9} };
	Vector2 direction = {1, 0};
	bool addSegment = false;

	void Draw()
	{
		for (unsigned int i = 0; i < body.size(); i++)
		{
			float x = body[i].x;
			float y = body[i].y;
			Rectangle segment = Rectangle{offset + x * cellSize, offset + y * cellSize, float(cellSize), float(cellSize) };
			DrawRectangleRounded(segment, 0.5, 6, darkGreen);
		}
	}
	void Update()
	{
		body.push_front(Vector2Add(body[0], direction));
		if (addSegment == true)
		{
			addSegment = false;
		}
		else
		{
			body.pop_back();
		}
	}

	void Reset()
	{
		body = {Vector2{6,9}, Vector2{5,9}, Vector2{4,9}};
		direction = {1, 0};
	}
};
//Food class
class Food {

public:
	Vector2 position;
	Texture2D texture;

	Food(deque<Vector2> snakeBody) {
		Image image = LoadImage("Graphics/applex32.png");
		texture = LoadTextureFromImage(image);
		UnloadImage(image);
		position = GenerateRandomPos(snakeBody);
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

	Vector2 GenerateRandomPos(deque<Vector2> snakeBody)
	{
		float x = GetRandomValue(0, cellCount - 1);
		float y = GetRandomValue(0, cellCount - 1);
		Vector2 position = {x, y};
		while (ElementInDeque(position, snakeBody))
		{
			position = GenerateRandomCell();
		}
		return position;
		
	}
};
//Game class
class Game 
{
public:
	Snake snake = Snake();
	Food food = Food(snake.body);
	bool running = true;

	void Draw()
	{
		food.Draw();
		snake.Draw();
	}

	void Update()
	{
		if(running)
		{
			snake.Update();
			CheckCollisionWithFood();
			CheckCollisionWithEdges();
			CheckCollisionWithTail();
		}
		
	}
	
	void CheckCollisionWithFood()
	{
		if(Vector2Equals(snake.body[0], food.position))
		{
			
			food.position = food.GenerateRandomPos(snake.body);
			snake.addSegment = true;
		}
	}

	void CheckCollisionWithEdges()
	{
		if(snake.body[0].x == cellCount || snake.body[0].x == -1)
		{
			GameOver();
		}
		if (snake.body[0].y == cellCount || snake.body[0].y == -1)
		{
			GameOver();
		}
	}
	void GameOver()
	{
		snake.Reset();
		food.position = food.GenerateRandomPos(snake.body);
		running = false;
	}

	void CheckCollisionWithTail()
	{
		deque<Vector2> headlessBody = snake.body;
		headlessBody.pop_front();
		if(ElementInDeque(snake.body[0], headlessBody))
		{
			GameOver();
		}
	}

	void GenerateRandomMaze(vector<Rectangle>& maze, int gridWidth, int gridHeight)
	{
		

		srand((unsigned int)time(0)); //Seed te raddom number generator

		//Create a random maze by filling cells with walls
		for (int x = 0; x< gridWidth; x++)
		{
			for(int y = 0; y < gridHeight; y++)
			{
				if (rand() % 4 == 0) // 25% chance of placing a wall
				{
					Rectangle wall = { x * cellSize, y * cellSize, cellSize, cellSize };
					maze.push_back(wall);
				}
			}
		}
	//Clear the center to ensure the snake has room to spawn and move
		int centerX = gridWidth / 2;
		int centerY = gridHeight / 2;
		maze.erase(remove_if(maze.begin(), maze.end(),
			[centerX, centerY](const Rectangle& wall) {
				return (wall.x >= (centerX - 1) * cellSize && wall.x <= (centerX + 1) * cellSize &&
					wall.y >= (centerY - 1) * cellSize && wall.y <= (centerY + 1) * cellSize);
			}),
		maze.end());

	}
};

int main() {
	cout << "Starting the game... " << endl;
	InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Mystery Maze");
	SetTargetFPS(60);

	Game game = Game();

	//Initialize the maze
	vector<Rectangle> maze;
	int gridWidth = 25 * 30 / 2;
	int gridHeight = 25* 30 / 2;
	game.GenerateRandomMaze(maze, gridWidth, gridHeight);

	while (WindowShouldClose() == false)
	{
		BeginDrawing();

		if (eventTriggered(0.2))
		{
			game.Update();

		}
		// Controls for our sanke
		if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1)
		{
			game.snake.direction = { 0, -1 };
			game.running = true;
		}
		if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != 1)
		{
			game.snake.direction = { 0, 1 };
			game.running = true;
		}
		if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1)
		{
			game.snake.direction = { -1, 0 };
			game.running = true;
		}
		if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != 1)
		{
			game.snake.direction = { 1, 0 };
			game.running = true;
		}
		//Drawing the classes
		ClearBackground(green);
		DrawRectangleLinesEx(Rectangle{ (float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10 }, 5, darkGreen);
		game.Draw();

		// Draw the maze walls
		for (const auto& wall : maze) {
			DrawRectangleRec(wall, darkGreen);
		}
		EndDrawing();
	}

	CloseWindow();
	return 0;
















}