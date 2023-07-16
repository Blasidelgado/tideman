#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Max number of candidates
#define MAX 9

// preferences[i][j] is number of voters who prefer i over j
int preferences[MAX][MAX];

// locked[i][j] means i is locked in over j
bool locked[MAX][MAX];

// Each pair has a winner, loser
typedef struct
{
    int winner;
    int loser;
}
pair;

// Array of candidates
char *candidates[MAX];
pair pairs[MAX * (MAX - 1) / 2];

int pair_count;
int candidate_count;

// Function prototypes
bool vote(int rank, char *name, int ranks[]);
void record_preferences(int ranks[]);
void add_pairs(void);
void sort_pairs(void);
void lock_pairs(void);
void print_winner(void);

// Custom functions
bool makes_circle(int cycle_start, int loser);
void swap_int(int *x, int *y);
void swap_pair(pair *x, pair *y);


int main(int argc, char *argv[])
{
    // Check for invalid usage
    if (argc < 2)
    {
        printf("Usage: tideman [candidate ...]\n");
        return 1;
    }

    // Populate array of candidates
    candidate_count = argc - 1;
    if (candidate_count > MAX)
    {
        printf("Maximum number of candidates is %i\n", MAX);
        return 2;
    }
    for (int i = 0; i < candidate_count; i++)
    {
        candidates[i] = argv[i + 1];
    }

    // Clear graph of locked in pairs
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = 0; j < candidate_count; j++)
        {
            locked[i][j] = false;
        }
    }

    pair_count = 0;
    int voter_count = 0;
    do
    {
        printf("Number of voters: ");
        scanf("%i", &voter_count);
    }
    while (voter_count < 1);

    // Query for votes
    for (int i = 0; i < voter_count; i++)
    {
        // ranks[i] is voter's ith preference
        int ranks[candidate_count];

        // Query for each rank
        for (int j = 0; j < candidate_count; j++)
        {
            char name[31];
            printf("Rank %i: ", j + 1);
            scanf("%30s", name);

            if (!vote(j, name, ranks))
            {
                printf("Invalid vote.\n");
                return 3;
            }
        }

        record_preferences(ranks);

        printf("\n");
    }

    add_pairs();
    sort_pairs();
    lock_pairs();
    print_winner();
    return 0;
}

// Update ranks given a new vote
bool vote(int rank, char *name, int ranks[])
{
    for (int i = 0; i < candidate_count; i++)
    {
        if (strcmp(name, candidates[i]) == 0) // Candidate is eligible
        {
            ranks[rank] = i; // Update voter's preference
            return true;
        }
    }
    return false;
}

// Update preferences given one voter's ranks
void record_preferences(int ranks[])
{
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = i + 1; j < candidate_count; j++)
        {
            preferences[ranks[i]][ranks[j]]++; // Start for the most preferred and update his/her remaining matchups
        }
    }
    return;
}

// Record pairs of candidates where one is preferred over the other
void add_pairs(void)
{
    int a, b;
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = i + 1; j < candidate_count; j++)
        {
            a = preferences[i][j];
            b = preferences[j][i];
            if (a == b) // If tie in face-to-face, do not create a pair
            {
                continue;
            }
            if (a > b) // Case candidate i is preferred over j
            {
                pairs[pair_count].winner = i;
                pairs[pair_count].loser = j;
            }
            else    // Case candidate j is preferred over i
            {
                pairs[pair_count].winner = j;
                pairs[pair_count].loser = i;
            }
            pair_count++;
        }
    }
    return;
}

// Sort pairs in decreasing order by strength of victory
void sort_pairs(void)
{
    int a, b, index = 0;
    int strengths[pair_count];
    pair p;

    for (int i = 0; i < pair_count; i++)
    {
        p = pairs[i];
        a = preferences[p.winner][p.loser];
        b = preferences[p.loser][p.winner];
        strengths[index] = a - b; // Add the strength of the victory
        index++;
    }

    // Implement bubble sorting in strengths and in pairs to sort both decreasingly.
    for (int j = 0; j < pair_count; j++)
    {
        int swaps = 0;
        for (int k = 0; k < pair_count - 1; k++)
        {
            if (strengths[k] < strengths[k + 1])
            {
                swap_int(&strengths[k], &strengths[k + 1]);
                swap_pair(&pairs[k], &pairs[k + 1]);
                swaps++;
            }
        }
        if (swaps == 0)
        {
            break;
        }
    }
    return;
}

// Lock pairs into the candidate graph in order, without creating cycles
void lock_pairs(void)
{
    for (int i = 0; i < pair_count; i++)
    {
        if (!makes_circle(pairs[i].winner, pairs[i].loser))
        {
            // Lock the pair unless it makes a circle
            locked[pairs[i].winner][pairs[i].loser] = true;
        }
    }
}

// Print the winner of the election
void print_winner(void)
{
    char winner[31];
    bool is_source;
    for (int i = 0; i < pair_count; i++)
    {
        is_source = true;
        for (int j = 0; j < pair_count; j++)
        {
            if (locked[j][i]) // If candidate resulted defeated in a face-to-face
            {
                is_source = false; // Then is not source
                break;  // Search for the next one
            }
        }
        if (is_source) // Candidate is the source therefore the winner
        {
            strcpy(winner, candidates[i]);
            break;
        }
    }
    printf("%s\n", winner);
    return;
}

// Recursive function to check if entry makes a circle
bool makes_circle(int cycle_start, int loser)
{
    if (loser == cycle_start)
    {
        // If the current loser is the cycle start
        // The entry makes a circle
        return true;
    }
    for (int i = 0; i < candidate_count; i++)
    {
        if (locked[loser][i]) // Check all face-to-face's the loser has won
        {
            if (makes_circle(cycle_start, i)) // Search recursively if the loser is the cycle start
            {
                return true;
            }
        }
    }
    return false;
}

// Helper function to swap strengths victories
void swap_int(int *x, int *y)
{
    int temp = *x;
    *x = *y;
    *y = temp;
}

// Helper function to swap pairs
void swap_pair(pair *x, pair *y)
{
    pair temp = *x;
    *x = *y;
    *y = temp;
}
