#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// NETWORK PARAMETERS //
typedef struct
{
    int foo; // TODO: Implement
} NetworkParameters;

// STAT SHEET //
typedef struct
{
    int foo; // TODO: Implement
} StatSheet;

// SCOUT //
typedef struct
{
    NetworkParameters parameters;
    StatSheet stats;
} Scout;

// POPULATION //
typedef struct
{
    Scout scouts[64];
} Population;

// NOVEL HISTORY
typedef struct
{
    Scout *scouts;
    size_t scout_count;
    size_t array_length;
} NovelHistory;

// ITERATE TRAINING //
void iterate_training(Population *population, NovelHistory *novel_history)
{
    /*
    GENERATE NEW world
    FOR EACH scout IN population:
        REPEAT 1000 TIMES:
            EVALUATE scout's network
            DO scout's action IN world
            UPDATE world
            UPDATE scout's stats

    FOR EACH scout IN population:
        ASSIGN novelty_score TO scout

    PRESERVE TOP 16 scouts
    DELETE BOTTOM 16 scouts
    DELETE 16 scouts at random

    REPEAT 32 TIMES:
        SELECT RANDOM original_scout
        CREATE new_scout WITH MUTATIONS
        ADD new_scout TO population
    */
}

// MAIN FUNCTION //
#define CMD_CHAR_LIMIT 256

int main(int argc, char const *argv[])
{
    char cmd_buffer[CMD_CHAR_LIMIT];

    Population population;

    NovelHistory novel_history;
    novel_history.scouts = (Scout *)malloc(sizeof(Scout) * 8);
    novel_history.scout_count = 0;
    novel_history.array_length = 8;

    while (true)
    {
        printf("> ");
        fgets(cmd_buffer, CMD_CHAR_LIMIT, stdin);
        for (size_t i = 0; i < CMD_CHAR_LIMIT; i++)
        {
            if (cmd_buffer[i] == '\n')
            {
                cmd_buffer[i] = '\0';
                break;
            }
        }

        // COMMAND: help
        // List the commands available in this program with information about how to use them
        if (strcmp(cmd_buffer, "help") == 0)
        {
            // TODO: Write help command
            printf("Sorry, I haven't written this yet..\n");
        }

        // COMMAND: spawn
        // Create a new population of scouts
        else if (strcmp(cmd_buffer, "spawn") == 0)
        {
            // TODO: Implement
        }

        // COMMAND: train <iterations>
        // Run a learning iteration
        else if (strcmp(cmd_buffer, "train") == 0)
        {
            // TODO: Repeat this as many times as dictated by the command argument
            iterate_training(&population, &novel_history);
        }

        // COMMAND: info
        // Get information about the current population
        else if (strcmp(cmd_buffer, "info") == 0)
        {
            // TODO: Implement
        }

        // COMMAND: quit
        // Stop the program
        else if (strcmp(cmd_buffer, "quit") == 0)
        {
            break;
        }

        else
        {
            printf("Command not recognised.\n");
        }

        printf("\n");
    }
}