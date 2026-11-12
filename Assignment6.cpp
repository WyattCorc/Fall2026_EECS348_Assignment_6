/*
Program Name: EECS 348 Assignment 5 - Recursive Sudoku Solver (Modified from Assignment 4)
Description: This program reads 5 Sudoku puzzle files (puzzle1.txt - puzzle5.txt), solves them 
             using recursive backtracking, displays the results to the console, and writes the 
             solutions for each puzzle to corresponding output files (solution1.txt - solution5.txt).
Inputs: puzzle1.txt, puzzle2.txt, puzzle3.txt, puzzle4.txt, puzzle5.txt
Outputs: Console output and solution1.txt, solution2.txt, solution3.txt, solution4.txt, solution5.txt
Collaborators: Wyatt Corcoran (Original Assignment 4 Author)
Other Sources: Google Gemini, ChatGPT
Author: Wyatt Corcoran
Created: 10/15/2026
Revised by: Minuka Cooray, Revised by Wyatt C
Revision Date: 10/29/2026
Compile on Cycle: g++ -std=c++11 -Wall -Wextra -o assignment5.exe assignment5.cpp
Run: ./assignment5.exe
*/

#include <iostream>//Manages I/O functionality
#include <fstream>//Reading and writing .txt files
#include <vector>//Implements lists
#include <string>//Allows strings to be used in C++
#include <set>//Sets, similar to lists, can only contain unique elems

//NOTICE - ALL ORIGINAL CODE BY GEMINI & WYATT CORCORAN, revisions for Assignment 5 made by Minuka Cooray are clarified

/*
Class Grid has functions to instantiate the 9x9 grid, load puzzles from files, check legal numbers
for empty cells, recursively solve the board via backtracking, and output solutions to both console and files.
*/
class Grid {//Defines class Grid
private://Begins private member section of Grid class
    char board[9][9]; //Creates an instance of the 2D array with max 9 rows, 9 cols.

    //Written by Wyatt to merge a defunct "Solver" class with the grid class
    bool findEmptyCell(int& r, int& c) const{//Iterates till it finds the first empty space
        for (r = 0; r < 9; ++r) {//iterates 0,8
            for (c = 0; c < 9; ++c) {//iterates 0,8
                if(board[r][c] == '_')//Returns true if an empty space exists
                return true;//Found an empty cell returns true
            }
        }
        return false;//returns false if the whole grid is full
    }

    //Original by Gemini rewritten by Wyatt
    //Function that uses sets to determine if a block of 9 is legal
     std::vector<char> SectionFilter(int r, int c) const { //output is a vector with chars 
        std::set<char> taken;//Creates a set to find what nums have already been taken in a given:
        for (int i = 0; i < 9; ++i) {//iterates 0-8
            taken.insert(board[r][i]);//row
            taken.insert(board[i][c]);//col
        }
        //Original by Gemini rewritten by Wyatt
        //This block determines if the 3x3 boxes are valid
        int startRow = (r / 3) * 3;//Finds which of the 9, 3x3 boxes you are in
        int startCol = (c / 3) * 3;//Calculates the starting column index of the 3x3 box
        for (int rowOffset = 0; rowOffset < 3; ++rowOffset) {//Iterates through the row of the box
            for (int colOffset = 0; colOffset < 3; ++colOffset) {//Iterates through the col of that box
                taken.insert(board[startRow + rowOffset][startCol + colOffset]);//Appends found values to the filter list
            }
        }       //Original by Gemini rewritten by Wyatt
                //Generates a list containing all of the Unused values
                std::vector<char> possible_vals;//Outputs a vector
        for (char digit = '1'; digit <= '9'; ++digit) {//Iterates through 1-9
            if (taken.find(digit) == taken.end()) {//Compares it to the set
                possible_vals.push_back(digit);//Add the value if it isnt in the set
            }
        }
        return possible_vals;//returns the list of available values
     }

    /*
    Block Summary: Initial Grid Validation Function
    Authored by: Combination of Gemini and Minuka Cooray
    This helper function verifies that the initial loaded board does not contain any duplicate digits
    in any row, column, or 3x3 subgrid before starting the recursive search (prevents invalid boards
    from hanging or falsely reporting solutions).
    */
    bool isInitialBoardValid() const { //Defines function to check if starting puzzle follows Sudoku rules
        for (int r = 0; r < 9; ++r) { //Loops through all 9 rows of the grid
            for (int c = 0; c < 9; ++c) { //Loops through all 9 columns of the grid
                char val = board[r][c]; //Stores the character at the current row and column
                if (val != '_') { //Checks if the current cell is pre-filled with a number
                    if (val < '1' || val > '9') return false; //Returns false if character is not a valid digit 1-9
                    for (int i = 0; i < 9; ++i) { //Loops from 0 to 8 to check the current row and column
                        if (i != c && board[r][i] == val) return false; //Returns false if duplicate found in same row
                        if (i != r && board[i][c] == val) return false; //Returns false if duplicate found in same column
                    } //Ends the row and column duplicate check loop
                    int startRow = (r / 3) * 3; //Calculates top row index of the current 3x3 subgrid
                    int startCol = (c / 3) * 3; //Calculates left column index of the current 3x3 subgrid
                    for (int ro = 0; ro < 3; ++ro) { //Iterates through the 3 rows of the subgrid
                        for (int co = 0; co < 3; ++co) { //Iterates through the 3 columns of the subgrid
                            int curR = startRow + ro; //Computes actual board row index inside subgrid
                            int curC = startCol + co; //Computes actual board column index inside subgrid
                            if ((curR != r || curC != c) && board[curR][curC] == val) { //Checks if another cell in box has same digit
                                return false; //Returns false because a 3x3 subgrid conflict exists
                            } //Ends subgrid conflict conditional check
                        } //Ends subgrid column loop
                    } //Ends subgrid row loop
                } //Ends non-empty cell check
            } //Ends column iteration loop
        } //Ends row iteration loop
        return true; //Returns true if no rule violations were found anywhere on the initial board
    } //Ends isInitialBoardValid function

     /*
     Block Summary: Recursive Backtracking Solver with File Output
     Authored by: Original by Gemini & Wyatt Corcoran, modified by Minuka Cooray (with Gemini assistance)
     Finds empty cells, tests valid digits, and when a full board is reached, increments solutionCount,
     prints the solution to the console, and writes the formatted 9x9 grid to the output file stream.
     */
      void solveRecursive(int& solutionCount, std::ofstream& outFile) { //Solution count and output file stream passed by reference
        int r, c; //Declares row and column integer variables to store empty cell coordinates
        //Base case for the recursion is a legal full grid
        if (!findEmptyCell(r, c)) {//If there are no empty cells
            solutionCount++;//Add a solution to the count
            std::cout << "Solution " << solutionCount << ":\n";//Print which solution number this is
            gridprint(); //calls the gridprint to display on console
            gridprintToFile(outFile); //Writes the solved board in input format to the solutionN.txt file
            return;//exit current recursive call
        } //Ends base case check
        std::vector<char> possible_vals = SectionFilter(r, c);//Find legal values
        for (char val : possible_vals) {//attempt a plugin for each legal value
            board[r][c] = val;//Plugs in the value at this point 
            solveRecursive(solutionCount, outFile);//Recur back through maintaining solution count and output file stream
            board[r][c] = '_';//This does the backtrack replacing the index with nothing if the calls return to here
        } //Ends candidate value loop
    } //Ends solveRecursive method

public://Begins public member section of Grid class
    //Initialize an empty grid (Original by Gemini & Wyatt Corcoran)
    Grid() {//Make a grid
        for (int r = 0; r < 9; ++r) {//Iterate row
            for (int c = 0; c < 9; ++c) {//Iterate col
                board[r][c] = '_';//Replace all 81 indices with '_'
            }
        }
    }

    //Rewritten by Wyatt
     bool loadFromFile(const std::string& filename) {//Uses the fstream lib to read the files char by char
        std::ifstream file(filename);//Grabs filename
        if (!file.is_open()) return false;//No file exists returns false, causes error to be printed from main
        for (int r = 0; r < 9; ++r) {//Iterates row
            for (int c = 0; c < 9; ++c) {//Iterates col
                if (!(file >> board[r][c])) return false;//Item by item replaces with equivalent from file if it cant, returns error
            }
        }
        return true;//returns true when the read succeeds
    }

    /*
    Block Summary: Puzzle Solve Controller and Solution File Generator
    Authored by: Original by Wyatt Corcoran, revised by Minuka Cooray (with Gemini assistance)
    Opens the corresponding solutionN.txt file, validates the initial puzzle, initiates the recursive solver,
    and writes either "No solution found." or "Solutions found: n" to the output file.
    */
    void solve(const std::string& filename, const std::string& outFilename) {//Takes input filename and output solution filename
        std::cout << "Puzzle file: " << filename << "\n";//Prints the original for comparison sake
        gridprint();//prints the original

        std::ofstream outFile(outFilename); //Opens the target solutionN.txt file for writing
        if (!outFile.is_open()) { //Checks if the output file failed to open
            std::cout << "Error opening output file: " << outFilename << "\n"; //Prints error message if file creation fails
            return; //Exits the function early if output file cannot be opened
        } //Ends output file open check

        int solutionCount = 0;//initializes solutions at 0
        if (isInitialBoardValid()) { //Checks that the starting puzzle has no immediate rule conflicts
            solveRecursive(solutionCount, outFile);//Begins recursion and streams found solutions to outFile
        } //Ends initial board validation check

        if (solutionCount == 0) {//No solution found, returns no solution
            std::cout << "No solution found.\n\n";//Prints no solution found to console
            outFile << "No solution found.\n"; //Writes the single line "No solution found." to solutionN.txt
        } else { //Executes if one or more valid solutions were found
            std::cout << "Solutions found: " << solutionCount << "\n\n"; //Prints total solution count to console
            outFile << "Solutions found: " << solutionCount << "\n"; //Writes "Solutions found: n" at the end of solutionN.txt
        } //Ends solution count conditional block
        outFile.close(); //Closes the solutionN.txt output file stream
    } //Ends solve function

    // Print function that displays output cleanly
    //section rewritten by Wyatt to fit to the output standards in solution1.txt
    void gridprint() const {//This iterates through row and col, printing off the result,
        for (int r = 0; r < 9; ++r) {//iterate by row
            for (int c = 0; c < 9; ++c) {//iterate by col
                std::cout << board[r][c] << ' ';//print at index
            }
            std::cout << '\n';//Break to represent rows properly
        }
        std::cout << '\n';//Break when the 9x9 print is done
    }

    /*
    Block Summary: File Grid Printer Function
    Authored by: Minuka Cooray (with Gemini assistance)
    Writes a solved 9x9 Sudoku grid to the provided ofstream in the exact same format as the input
    puzzle files (space-separated values on 9 lines, followed by a blank line).
    */
    void gridprintToFile(std::ofstream& outFile) const { //Takes active output file stream by reference
        for (int r = 0; r < 9; ++r) { //Iterates through rows 0 to 8
            for (int c = 0; c < 9; ++c) { //Iterates through columns 0 to 8
                outFile << board[r][c]; //Writes the character at current board index to the file
                if (c < 8) { //Checks if current column is not the last column in the row
                    outFile << ' '; //Writes a separating space between characters on the same row
                } //Ends space separator conditional
            } //Ends column loop
            outFile << '\n'; //Writes a newline character at the end of each row
        } //Ends row loop
        outFile << '\n'; //Writes an extra newline to cleanly separate multiple solutions in the file
    } //Ends gridprintToFile function
};//Ends class Grid definition

/*
Main loops through the 5 puzzle files (puzzle1.txt - puzzle5.txt), generates the matching output
filename (solution1.txt - solution5.txt), loads each puzzle into the Grid class, and writes the results.
Original by Wyatt Corcoran, modified by Minuka Cooray to map each puzzleN.txt to solutionN.txt.
*/
int main() {//Main entry point of the program
    //Hard assigned the 5 puzzles
    std::string files[] = {"puzzle1.txt", "puzzle2.txt", "puzzle3.txt", "puzzle4.txt", "puzzle5.txt"};//Array of input filenames
    //Hard assigned the 5 matching solution output files
    std::string outFiles[] = {"solution1.txt", "solution2.txt", "solution3.txt", "solution4.txt", "solution5.txt"};//Array of output filenames

    //Iterates through each file index from 0 to 4
    for (int i = 0; i < 5; ++i) {//Loops through all 5 puzzles using index i
        std::string filename = files[i];//Gets current input puzzle filename
        std::string outFilename = outFiles[i];//Gets matching solutionN.txt filename
        Grid grid;//Makes a new grid
        if (grid.loadFromFile(filename)) { //Loads the starting values from the file
            grid.solve(filename, outFilename);//Runs the recursive solve and writes to solutionN.txt
        } else {//if FALSE is output during the loadFromFile funct you get this error printed
            std::cout << "Could not open or read file: " << filename << "\n\n";//error printed here
        }//Ends file load conditional
    }//Ends puzzle file loop
    
    return 0; //Finishes running successfully 
}//Ends main function