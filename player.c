// player.c
#include "player.h"
#include "wordle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define MAX_WORDS 15000
#define WORD_LENGTH 5
#define SOLUTION_LENGTH 2500

static char wordList[MAX_WORDS][WORD_LENGTH + 1];
static int wordCount = 0;
static char solutionList[SOLUTION_LENGTH][WORD_LENGTH + 1];
static int solutionCount = 0;

static char possibleWords[SOLUTION_LENGTH][WORD_LENGTH + 1];
static int possibleWordCount = 0;
static int initialized = 0;
static char previousGuess[WORD_LENGTH + 1];

static void loadWords()
{
    if (wordCount > 0)
        return;
    FILE *file = fopen("wordList.txt", "r");
    if (file == NULL)
    {
        perror("Failed to open wordList.txt");
        exit(EXIT_FAILURE);
    }
    char buffer[WORD_LENGTH + 2];
    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {
        buffer[strcspn(buffer, "\r\n")] = '\0';
        if (strlen(buffer) == WORD_LENGTH)
        {
            strcpy(wordList[wordCount], buffer);
            wordCount++;
            if (wordCount >= MAX_WORDS)
            {
                break;
            }
        }
    }
    fclose(file);
}

static void loadSolutions()
{
    if (solutionCount > 0)
        return;
    FILE *file = fopen("solutionList.txt", "r");
    if (file == NULL)
    {
        perror("Failed to open solutionList.txt");
        exit(EXIT_FAILURE);
    }
    char buffer[WORD_LENGTH + 2];
    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {
        buffer[strcspn(buffer, "\r\n")] = '\0';
        if (strlen(buffer) == WORD_LENGTH)
        {
            strcpy(solutionList[solutionCount], buffer);
            solutionCount++;
            if (solutionCount >= SOLUTION_LENGTH)
            {
                break;
            }
        }
    }
    fclose(file);
}

void reset_player()
{
    possibleWordCount = 0;
    initialized = 0;
    previousGuess[0] = '\0';
    // No need to reset wordList and solutionList or their counts since they are constant after loading
}

char *player_AI(const char lastResult[WORD_LENGTH + 1])
{
    if (!initialized)
    {
        // Load words and solutions
        loadWords();
        loadSolutions();

        // Initialize possibleWords with all possible solutions
        possibleWordCount = solutionCount;
        for (int i = 0; i < solutionCount; i++)
        {
            strcpy(possibleWords[i], solutionList[i]);
        }
        initialized = 1;

        // Return initial guess
        strcpy(previousGuess, "slate"); // Good starting word
        char *guess = (char *)malloc((WORD_LENGTH + 1) * sizeof(char));
        strcpy(guess, previousGuess);
        return guess;
    }

    // Update possibleWords based on lastResult and previousGuess
    int newPossibleWordCount = 0;
    char newPossibleWords[SOLUTION_LENGTH][WORD_LENGTH + 1];

    for (int i = 0; i < possibleWordCount; i++)
    {
        char *word = possibleWords[i];
        char *result = checkWord(word, previousGuess);
        if (strcmp(result, lastResult) == 0)
        {
            strcpy(newPossibleWords[newPossibleWordCount], word);
            newPossibleWordCount++;
        }
        free(result);
    }

    // Update the list of possible words
    possibleWordCount = newPossibleWordCount;
    memcpy(possibleWords, newPossibleWords, possibleWordCount * (WORD_LENGTH + 1));

    if (possibleWordCount == 0)
    {
        // No possible words remaining
        fprintf(stderr, "No possible words remaining.\n");
        exit(1);
    }

    if (possibleWordCount == 1)
    {
        // Only one possible word left
        strcpy(previousGuess, possibleWords[0]);
        char *guess = (char *)malloc((WORD_LENGTH + 1) * sizeof(char));
        strcpy(guess, previousGuess);
        return guess;
    }

    // Select next guess that minimizes expected number of possible words
    double bestExpectedRemaining = 1e9;
    char bestGuess[WORD_LENGTH + 1];

    // Consider all words in wordList as possible guesses
    for (int g = 0; g < possibleWordCount; g++)
    {
        char *G = possibleWords[g];
        int feedbackCounts[243] = {0};

        for (int s = 0; s < possibleWordCount; s++)
        {
            if (s == g)
                continue;
            else
            {
                char *S = possibleWords[s];
                char *feedback = checkWord(S, G);

                // Map feedback to index
                int index = 0;
                for (int i = 0; i < WORD_LENGTH; i++)
                {
                    index *= 3;
                    if (feedback[i] == 'G')
                    {
                        index += 2;
                    }
                    else if (feedback[i] == 'Y')
                    {
                        index += 1;
                    }
                }

                feedbackCounts[index]++;
                free(feedback);
            }

            // Compute expected remaining words
            double expectedRemaining = 0.0;
            for (int i = 0; i < 243; i++)
            {
                if (feedbackCounts[i] > 0)
                {
                    double p = (double)feedbackCounts[i] / possibleWordCount;
                    expectedRemaining += p * log2(p);
                }
            }

            if (expectedRemaining < bestExpectedRemaining)
            {
                bestExpectedRemaining = expectedRemaining;
                strcpy(bestGuess, G);
            }
        }
    }

    // Return the best guess
    strcpy(previousGuess, bestGuess);
    char *guess = (char *)malloc((WORD_LENGTH + 1) * sizeof(char));
    strcpy(guess, previousGuess);
    return guess;
}
