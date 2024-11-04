#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define CMD_CHAR_LIMIT 256

int main(int argc, char const *argv[])
{
    char cmd_buffer[CMD_CHAR_LIMIT];

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

        // COMMAND: spawn <size>
        // Create a new population of scouts
        else if (strcmp(cmd_buffer, "spawn") == 0)
        {
            // TODO: Implement
        }

        // COMMAND: train <iterations>
        // Run a learning iteration
        else if (strcmp(cmd_buffer, "train") == 0)
        {
            // TODO: Implement
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