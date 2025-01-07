# Maze-V1
 Maze Game V1 Project

Main menu: 
START DIFFICULTY EXIT VOLUME
Controls: Player controls are defined as the Arrow Keys (UP, DOWN, LEFT, RIGHT)
Volume: (Left arrow = -) (Right arrow = +)

Maze Generation algorithm: 
The maze is generated using DFS(Depth-First=Search) Algorithm. 
First, we select a path in the maze (for the sake of the example, let's choose a path according to some rule we lay out ahead of time) 
and we follow it until we hit a dead end or reach the finishing point of the maze.

 Aim & Objectives:
Complete the maze in a given amount of time based on player's chosen difficulty (Easy(120 Seconds), Medium(60Seconds), Hard(30Seconds)).
Eat the watermelons to increase game score status.
Complete level maze to increase level status.

The Maze class initializes a grid of cellCount x cellCount dimensions, where each cell starts as a wall (true in grid array).
Generate() is the entry point for creating the maze. It:
1.	Clears existing walls.
2.	Resets the grid by marking all cells as walls.
3.	Invokes the recursive DFS-based maze generator starting at (0, 0). 


 
Recursive Maze Generation (GenerateMazeDFS)
Use a stack to store the cells that need to be processed.
This function works recursively and uses randomized directions to ensure variability in maze patterns:
Step-by-Step Logic:
1.	Mark Current Cell: The current cell (x, y) is marked as free (false).
2.	Randomize Directions: The algorithm defines four possible movements:
•	Up: (0, -1)
•	Down: (0, 1)
•	Left: (-1, 0)
•	Right: (1, 0) These directions are shuffled randomly using std::shuffle to introduce randomness in the maze structure.
3.	Visit Neighbours:
Move 2 steps in a chosen direction (nx, ny = x + dir.x * 2, y + dir.y * 2).
If (nx, ny) is within bounds and still a wall (grid[ny][nx] == true), carve a passage:
•	Remove the wall between the current cell (x, y) and (nx, ny) by marking the intermediate cell grid[y + dir.y][x + dir.x] = false.
•	Recursively call GenerateMazeDFS(nx, ny) to explore the new cell.
4.	The recursion ends when all neighbours have been visited.
 

Backtracking:
•	If there are no valid moves, backtrack by popping the stack.
•	Continue until all cells are visited.
 

Boundary and Path Rules:
•	Odd-indexed cells are paths, and even-indexed cells remain walls. This ensures walls between paths.  

Post-Processing
Once the maze is generated:
•	Convert Walls to Rectangles: For visualization, wall cells are converted into Rectangle objects, stored in walls.
•	Ensure Entry/Exit Points: The start (0, 0) and end (cellCount-1, cellCount-1) are cleared (false).
Visualization
•	Walls are drawn as maroon-coloured rectangles.
•	Start and end positions are highlighted in green and red, respectively.
 

