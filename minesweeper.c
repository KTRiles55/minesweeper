// Kenneth Riles
// This project will run a minesweeper game

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#pragma warning(disable : 4996)


const int MAX_TOKENS = 50;
const int MAXLINELEN = 400;

struct cell
{
    int position;
    int adjcount;
    int mined;
    int covered;
    int flagged;
};
typedef struct cell cell;

cell** board = NULL;
int rows;
int cols;
int mines;
int boardCreated = 0;
int win = 0;

int neighborCount = 8;
int rowNeighbors[] = { -1, -1, 0, 1, 1, 1, 0, -1 };
int colNeighbors[] = { 0, 1, 1, 1, 0, -1, -1, -1 };


int get_random(int range)
{
    return ((int)floor((float)range * rand() / RAND_MAX)) % range;
}

void getLine(char string[], int maxLength);

void tokenizeLine(char string[], char tokens[][MAX_TOKENS], int tokenCount);

int process_command(char tokens[][MAX_TOKENS], int tokenCount);

int rungame();

char* fixLetterCase(char* newCommand);

void command_help();

void command_new(int r, int c, int m);

void command_show();

void command_flag(int r, int c);

void command_unflag(int r, int c);

int command_uncover(int r, int c);

void uncover_recursive(int r, int c);

void init_cell(cell* c, int p);

void display_cell(cell* c);

void distributeMines(int r, int c, int m);

void calculateAdjacencyCounts(int r, int c);  

int checkWinCondition();

int checkMissingParameters(char command[], int rVal, int cVal);


int main()
{
    printf("\nHello!\nYou are playing the game of Minesweeper!\n\n"
           "If you are new to using this program,\n"
           "please enter the command \"help\" for more information!\n\n");
    rungame();
}



void getLine(char string[], int maxLength)
{
    fgets(string, maxLength, stdin);
    int length = strlen(string);
    string[length - 1] = '\0';
}



void tokenizeLine(char string[], char tokens[][MAX_TOKENS], int tokenCount)
{
    char stringcpy[MAXLINELEN];
    strcpy(stringcpy, string);
    char* firstToken = strtok(stringcpy, " ");

    while (firstToken != NULL)
    {
        strcpy(tokens[tokenCount], firstToken);
        tokenCount++;
        firstToken = strtok(NULL, " ");
    }
}



// Identifies command based on user input
int process_command(char tokens[][MAX_TOKENS], int tokenCount)
{
    char* command = tokens[0];
    printf(command);
    command = fixLetterCase(command);
    int rVal = atoi(tokens[1]);
    int cVal = atoi(tokens[2]);

    if (strcmp(command, "new") == 0)
    {
        printf("\n>> Entered NEW command <<\n\n");

        if (checkMissingParameters(command, rVal, cVal) > 0) {
            return -1;
        }

        if (atoi(tokens[3]) > 0) {
            rows = rVal;
            cols = cVal;
            mines = atoi(tokens[3]);

            if (boardCreated < 1)
            {
                boardCreated = 1;
            }

            else
            {
                free(board);
            }

            command_new(rows, cols, mines);
        }

        else {
            printf("Command will not work correctly.\n"
                  "Please enter NEW command in following format:\n" 
                  "** new [# of rows] [# of cols] [# of mines] **\n");
        }

    }

    else if (strcmp(command, "show") == 0)
    {
        printf("\n>> Entered SHOW command <<\n\n");
        command_show();
    }

    else if (strcmp(command, "help") == 0)
    {
        printf("\n>> Entered HELP command <<\n\n");
        command_help();
    }

    else if (strcmp(command, "quit") == 0)
    {
        printf("\n>> Entered QUIT command <<\nTerminating program...\n\n");
        return 0;
    }

    else if (strcmp(command, "flag") == 0)
    {
        printf("\n>> Entered FLAG command <<\n\n");
        if (checkMissingParameters(command, rVal, cVal) > 0) {
            return -1;
        }

        if (board == NULL)
        {
            printf("Board wasn't created yet.\nThere are no cells to flag!\n\n");
            return -1;
        }

        command_flag(rVal, cVal);
    }

    else if (strcmp(command, "unflag") == 0)
    {
        printf("\n>> Entered UNFLAG command <<\n\n");
        if (checkMissingParameters(command, rVal, cVal) > 0) {
            return -1;
        }

        if (board == NULL)
        {
            printf("Board wasn't created yet.\nThere are no cells to unflag!\n\n");
            return -1;
        }

        command_unflag(rVal, cVal);
    }

    else if (strcmp(command, "uncover") == 0)
    {
        printf("\n>> Entered UNCOVER command <<\n\n");
        if (checkMissingParameters(command, rVal, cVal) > 0) {
            return -1;
        }

        if (board == NULL)
        {
            printf("Board wasn't created yet.\nThere are no cells to uncover!\n\n");
            return -1;
        }

        int winning_condition = command_uncover(rVal, cVal);
        if (winning_condition == -1)
        {
            printf("You uncovered a mine!\nYou lose!\n");
            return 0;
        }

        else if (winning_condition == 1)
        {
            win = 1;
            printf("You have flagged all mines and uncovered all cells!\n You win!\n");
        }
    }

    else
    {
        printf("\n> Unknown command. <\n> Please try entering again! <\n\n");
        return -1;
    }

    return 1;
}



char* fixLetterCase(char* newCommand)
{
    int length = sizeof(newCommand) / sizeof(newCommand[0]);
    for (int i = 0; i < length; ++i)
    {
        unsigned char c = newCommand[i];
        if (((int)c < 91) && ((int)c > 64))
        {
            int dec = (int)c + 32;
            c = (char)dec;
            newCommand[i] = c;
        }
    }

    return newCommand;
}



// Creates new board dynamically
void command_new(int r, int c, int m)
{
    board = (cell**)malloc(sizeof(cell*) * r);
    for (int i = 0; i < r; ++i)
    {
        board[i] = (cell*)malloc(sizeof(cell) * c);
    }

    for (int i = 0; i < r; ++i)
    {
        for (int j = 0; j < c; ++j)
        {
            init_cell(&board[i][j], i * c + j);
        }
    }

    distributeMines(r, c, m);
    calculateAdjacencyCounts(r, c);
}



// Displays board of cells
void command_show()
{
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            display_cell(&board[i][j]);
        }
        printf("\n");
    }
}


// Displays each available command and how each works
void command_help()
{
    printf("Available commands:\n"
           "__________________________________________________________________________________________________________\n\n"
           "1) new [# of rows] [# of cols] [* of mines] - Creates a new table containing rows, columns and mines.\n\n"
           "2) show - Displays covered/uncovered table.\n\n"
           "3) flag [row #] [col #] - Places flag on selected cell in position.\n\n"
           "4) unflag [row #] [col #] - Removes flag from selected cell in position.\n\n"
           "5) uncover [row #] [col #] - Uncovers a cell in selected position.\n\n"
           "6) quit - Terminates the program.\n"
           );
}

// Initializes each cell upon creation
void init_cell(cell* c, int p)
{
    c->position = p;
    c->adjcount = 0;
    c->mined = 0;
    c->covered = 1;
    c->flagged = 0;
}



void display_cell(cell* c)
{
    if (c->covered == 1)
    {
        printf("%2s", "/");
    }

    else if (c->mined == 1)
    {
        printf("%2s", "*");
    }

    else if (c->adjcount == 0)
    {
        printf("%2s", ".");
    }

    else
    {
        printf("%2d", c->adjcount);
    }
}



// Randomly displaces mines
void distributeMines(int r, int c, int m)
{
    int random_row;
    int random_col;
    int i = 1;
    while (i <= m)
    {
        random_row = get_random(r);
        random_col = get_random(c);
        if (board[random_row][random_col].mined == 0)
        {
            board[random_row][random_col].mined = 1;
            i++;
        }
    }
}


// Calculates number of neighbors that contain mines for each cell
void calculateAdjacencyCounts(int r, int c)
{
    int mineCount;

    for (int i = 0; i < r; ++i)
    {
        for (int j = 0; j < c; ++j)
        {
            mineCount = 0;
            for (int d = 0; d < neighborCount; ++d)
            {
                int rn = i + rowNeighbors[d];
                int cn = j + colNeighbors[d];
                

                if ((rn >= 0) && (rn < r) && (cn >= 0) && (cn < c))
                {
                    if (board[rn][cn].mined == 1)
                    {
                        mineCount++;
                    }
                }
            }

            board[i][j].adjcount = mineCount;
        }
    }
}



// flags a cell
void command_flag(int r, int c)
{
    if (board[r][c].flagged == 0)
    {
        board[r][c].flagged = 1;
    }

    else
    {
        printf("Cell has already been flagged!\n\n");
    }
}


// removes a flag from a cell
void command_unflag(int r, int c)
{
    if (board[r][c].flagged == 1)
    {
        board[r][c].flagged = 0;
    }

    else
    {
        printf("Cell was already unflagged!\n\n");
    }
}


// uncovers a cell, with assist from helper function for empty cells
int command_uncover(int r, int c)
{
    uncover_recursive(r, c);

    if (board[r][c].mined == 1)
    {
        return -1; 
    }

    else 
    {
        int win_check = checkWinCondition();
        if (win_check == 1)
        {
            return 1;
        }
    }

    return 0;
}


// helper for uncover command when dealing with empty cells
void uncover_recursive(int r, int c)
{
    if (board[r][c].covered == 1)
    {
        board[r][c].covered = 0;
        
    }

    if (board[r][c].adjcount == 0) 
    {
        for (int d = 0; d < neighborCount; ++d)
        {
            int rn = r + rowNeighbors[d];
            int cn = c + colNeighbors[d];

            if ((rn >= 0) && (rn < r) && (cn >= 0) && (cn < c))
            {
                if (board[rn][cn].covered == 1)
                {
                    uncover_recursive(rn, cn);
                }
            }
        }
    }
}


int rungame()
{
    int new_game = 1;
    while (new_game > 0)
    {
        srand(time(0));
        char line[80];
        char tokens[10][MAX_TOKENS];
        int tokenCount = 0;

        int command_validity_checkNum = -1;

        while (command_validity_checkNum < 0)
        {
            printf("\n>>");
            // reads input from user
            getLine(line, MAXLINELEN);

            // tokenizes input string
            tokenizeLine(line, tokens, tokenCount);
            command_validity_checkNum = process_command(tokens, tokenCount);
        }

        if (command_validity_checkNum == 0) {
            return -1;
        }

        if (win == 1)
        {
            new_game = 0;
            // free memory space from each cell on the board
            free(board);
        }
    }
    char response[50];
    int replayGame = 1;
    while (replayGame > 0)
    {
        printf("Do you want to play again (Yes / No)? ");
        getLine(response, MAXLINELEN);
        fixLetterCase(response);
        if (strcmp(response, "yes") == 0)
        {
            win = 0;
            boardCreated = 0;
            rungame();
        }

        else if (strcmp(response, "no") == 0)
        {
            printf("\nClosing program...\n");
            replayGame = 0;
        }

        else
        {
            printf("\n\n> Invalid response <\nPlease enter either \"yes\" or \"no\"\n\n");
        }
    }

    return new_game;
}


int checkWinCondition()
{
    int flagged_mines = 0;
    int covered_cells = 0;
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            if ((board[i][j].mined == 1) && (board[i][j].flagged == 1))
            {
                flagged_mines++;
            }

            if ((board[i][j].covered == 1) && (board[i][j].mined == 0))
            {
                covered_cells++;
            }
        }
    }

    if ((flagged_mines == mines) && (covered_cells == 0))
    {
        return 1;
    }

    return 0;
}


int checkMissingParameters(char command[], int rVal, int cVal) {
    if ((rVal == 0) || (cVal == 0)) {
        printf("%s command must take 2 parameters in the form:\n"
               "** %s [# of rows] [# of cols] **\n", command, command);
        return 1;
    }

    return 0;
}
