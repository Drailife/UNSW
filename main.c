//
// DO NOT CHANGE THIS FILE
//
// You do not submit this file. This file is not marked.
// If you think you need to change this file you have
// misunderstood the assignment - ask in the course forum.
//
// Assignment 2 24T1 COMP1511: CS Dungeon
// main.c
//
// Version 1.0.0: Release
//
// This file allows you to interactively test the functions you
// implement in cs_dungeon.c

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cs_dungeon.h"

// You do not need to use any of the defines in this file!
#define MAX_LINE 2048

// Win requirements
#define MIN_WIN 5
#define MAX_WIN 50

// Complete commands
#define HELP '?'
#define QUIT 'q'

// Stage 1
#define APPEND_DUNGEON 'a'
#define PRINT_MAP 'p'
#define ADD_FINAL_BOSS 'f'
#define CHECK_PLAYER_STATS 's'

// Stage 2
#define INSERT_DUNGEON 'i'
#define PRINT_CURRENT_DUNGEON 'd'
#define CLASS_POWER 'P'

// Stage 3
#define ADD_ITEM 't'
#define COLLECT_ITEM 'c'
#define USE_ITEM 'u'

// Stage 4
#define TELEPORT 'T'
#define BOSS_FIGHT 'b'

// Command Returns
#define EXIT 0
#define READ_COMMAND 2

// Setup and Gameplay Distinguishers
#define SETUP 1
#define GAMEPLAY 2

// Helper Functions
void welcome_message(void);
struct map *setup_map(void);
void setup_player(struct map *map);
struct map *command_loop(struct map *map, int game_status);
static int get_command(char *command, int max_command_length);
static int run_command_setup(struct map *map, char *line);
static int run_command_gameplay(struct map *map, char *line);
void handle_final_boss(struct map *map);

// Do: Completed
static void do_print_help_setup(void);
static void do_print_help_gameplay(void);

// Do: Stage 1
static void do_append_dungeon(struct map *map, char *line);
static void do_print_map(struct map *map);
static void do_final_boss(struct map *map, char *line);
static void do_player_stats(struct map *map);

// Do: Stage 2
static void do_insert_dungeon(struct map *map, char *line);
static void do_print_dungeon(struct map *map);
static void do_move_player(struct map *map, char *line);
static void do_fight(struct map *map, char *line);
static int do_end_turn(struct map *map);
static void do_class_power(struct map *map);

// Do: Stage 3
static void do_add_item(struct map *map, char *line);
static void do_collect_item(struct map *map, char *line);
static void do_use_item(struct map *map, char *line);

// Do: Stage 4
static void do_teleport(struct map *map);
static void do_boss_fight(struct map *map);

int main(void)
{
    welcome_message();
    struct map *map = setup_map();
    setup_player(map);

    printf("\n\n----------Setup Phase----------\n\n");
    map = command_loop(map, SETUP);

    if (map->entrance == NULL) {
        printf(
            "No dungeons have been added, so it's not possible to play "
            "the game! Exiting.\n");
        free_map(map);
        return 1;
    }

    handle_final_boss(map);
    do_print_map(map);

    printf("\n\n--------Gameplay Phase--------\n\n");
    map = command_loop(map, GAMEPLAY);

    printf("\nThanks for playing!\n");

    free_map(map);

    return 0;
}

// Prints welcome message
//
// Parameters:
//      nothing
//
// Returns:
//      nothing
//
void welcome_message(void)
{
    printf("Welcome to the 1511 Dungeon!\n");
    printf(
        "This is a game where you get to create your own dungeon map, "
        "battle monsters and collect items!\n");
}

// Scans input/output for creating the map, handles calling create_map
//
// Parameters:
//      nothing
//
// Returns:
//      pointer to the newly malloc'd struct map
//
struct map *setup_map(void)
{
    printf("Please enter the name of your map: ");
    char map_name[MAX_STR_LEN];
    fgets(map_name, MAX_STR_LEN, stdin);
    map_name[strcspn(map_name, "\n")] = '\0';

    printf("Please enter the amount of points required to win: ");
    int win_requirement;
    scanf("%d", &win_requirement);
    while (win_requirement < MIN_WIN || win_requirement > MAX_WIN) {
        printf("Please enter a win requirement between %d and %d: ", MIN_WIN,
               MAX_WIN);
        scanf("%d", &win_requirement);
    }

    // Remove whitespace from buffer
    int whitespace_ch;
    while ((whitespace_ch = getchar()) != '\n' && whitespace_ch != EOF) {
    }

    struct map *map = create_map(map_name, win_requirement);

    return map;
}

// Scans input/output for creating the player, handles calling create_player
//
// Parameters:
//      pointer to the dungeon map
//
// Returns:
//      nothing
//
void setup_player(struct map *map)
{
    // Get player name
    printf("Please enter the player's name: ");
    char player_name[MAX_STR_LEN];
    fgets(player_name, MAX_STR_LEN, stdin);
    player_name[strcspn(player_name, "\n")] = '\0';

    // Get player class until valid class is entered
    printf("Player Class Options:\n\tFighter\n\tWizard\n");
    printf("Please enter player's chosen class type: ");
    char class_type[MAX_STR_LEN];
    fgets(class_type, MAX_STR_LEN, stdin);
    class_type[strcspn(class_type, "\n")] = '\0';
    while (strncmp(class_type, "Fighter", 8) != 0 &&
           strncmp(class_type, "Wizard", 7) != 0) {
        printf(
            "ERROR: Invalid class. Class '%s' does not exist. "
            "Enter again: ",
            class_type);
        fgets(class_type, MAX_STR_LEN, stdin);
        class_type[strcspn(class_type, "\n")] = '\0';
    }

    map->player = create_player(player_name, class_type);
}

// Loops until CTRL + D, scanning in commands from the user
//
// Parameters:
//      map         - pointer to the dungeon map, containing list of dungeons,
//                    the player, and the points required to win.
//      game_status - value indicating whether or not the game has ended
//
// Returns:
//      a pointer to the dungeon map
//
struct map *command_loop(struct map *map, int game_status)
{
    int read_command = READ_COMMAND;
    char line[MAX_LINE];
    while (read_command) {
        printf("\nEnter Command: ");
        read_command = get_command(line, MAX_LINE);
        if (read_command && game_status == SETUP) {
            read_command = run_command_setup(map, line);
            if (read_command == EXIT) {
                return map;
            }
        } else if (read_command && game_status == GAMEPLAY) {
            read_command = run_command_gameplay(map, line);
            if (read_command != READ_COMMAND) {
                return map;
            }
        }

        // Exit on CTRL + D when in the setup phase
        if (read_command == 0 && game_status == SETUP) {
            printf("Thanks for playing!\n");
            free_map(map);
            exit(1);
        }
    }
    return map;
}

// Scans in a command line from stdin, removes the newline and any whitespace
//
// Parameters:
//      command             - where to store the scanned line
//      max_command_length  - maximum length of command line
//
// Returns:
//      int indicating if fgets was successful or not
//
static int get_command(char *command, int max_command_length)
{
    if (fgets(command, max_command_length, stdin) == NULL) {
        return 0;
    }

    // remove '\n' if present
    command[strcspn(command, "\n")] = '\0';

    // This will remove any leading whitespace from entered commands.
    // memmove copies MAX_LINE - leading_whitespace bytes from
    // command + leading_whitespace to command, meaning you are left with only
    // the command
    int leading_whitespace = 0;
    while (isspace(command[leading_whitespace])) {
        leading_whitespace++;
    }
    memmove(command, command + leading_whitespace,
            MAX_LINE - leading_whitespace);

    return 1;
}

// Checks what the entered command was, and calls the corresponding do function
// During Setup
//
// Parameters:
//      map         - pointer to the dungeon map, containing list of dungeons,
//                    the player, and the points required to win.
//      line        - the scanned in line from stdin containing the commands
//
// Returns:
//      an int indicating whether or not to keep reading commands
//
static int run_command_setup(struct map *map, char *line)
{
    char command = line[0];
    if (command == HELP) {
        do_print_help_setup();
    } else if (command == QUIT) {
        return EXIT;
    } else if (command == APPEND_DUNGEON) {
        do_append_dungeon(map, line);
    } else if (command == PRINT_MAP) {
        do_print_map(map);
    } else if (command == ADD_FINAL_BOSS) {
        do_final_boss(map, line);
    } else if (command == CHECK_PLAYER_STATS) {
        do_player_stats(map);
    } else if (command == INSERT_DUNGEON) {
        do_insert_dungeon(map, line);
    } else if (command == ADD_ITEM) {
        do_add_item(map, line);
    } else {
        printf("Invalid command '%c', use '?' to get help!\n", line[0]);
    }
    return READ_COMMAND;
}

// Checks what the entered command was, and calls the corresponding do
// During Gameplay
//
// Parameters:
//      map         - pointer to the dungeon map, containing list of dungeons,
//                    the player, and the points required to win.
//      line        - the scanned in line from stdin containing the commands
//
// Returns:
//      an int indicating whether or not to keep reading commands
//
static int run_command_gameplay(struct map *map, char *line)
{
    char command = line[0];
    if (command == HELP) {
        do_print_help_gameplay();
    } else if (command == QUIT) {
        return EXIT;
    } else if (command == PRINT_MAP) {
        do_print_map(map);
    } else if (command == CHECK_PLAYER_STATS) {
        do_player_stats(map);
    } else if (command == PRINT_CURRENT_DUNGEON) {
        do_print_dungeon(map);
    } else if (command == NEXT_DUNGEON || command == PREVIOUS_DUNGEON) {
        do_move_player(map, line);
    } else if (command == PHYSICAL_ATTACK || command == MAGICAL_ATTACK) {
        do_fight(map, line);
    } else if (command == CLASS_POWER) {
        do_class_power(map);
    } else if (command == COLLECT_ITEM) {
        do_collect_item(map, line);
    } else if (command == USE_ITEM) {
        do_use_item(map, line);
    } else if (command == TELEPORT) {
        do_teleport(map);
    } else if (command == BOSS_FIGHT) {
        do_boss_fight(map);
    } else {
        printf("Invalid command '%c', use '?' to get help!\n", line[0]);
    }

    int end_turn_return = do_end_turn(map);

    if (end_turn_return != CONTINUE_GAME) {
        return end_turn_return;
    }

    return READ_COMMAND;
}

// Handles printing and calling for the final_boss function in dungeon.c. This
// function is called multiple times until a valid final boss is inputted.
//
// Parameters:
//      map         - pointer to the dungeon map, containing list of dungeons,
//                    the player, and the points required to win.
//
// Returns:
//      nothing
//
void handle_final_boss(struct map *map)
{
    printf("Please enter the required item to defeat the final boss: ");
    int read_command = READ_COMMAND;
    char line[MAX_LINE];
    read_command = get_command(line, MAX_LINE);
    if (read_command) {
        do_final_boss(map, line);
    }
}

// Prints the help table, explaining the different commands available when
// setting up the game
//
// Parameters:
//      nothing
//
// Returns:
//      nothing
//
static void do_print_help_setup(void)
{
    printf(
        "=======================[  1511 Dungeon  ]=======================\n"
        "      =============[    Setup: Usage Info   ]===============    \n"
        "  ?                                                             \n"
        "    Show setup usage info                                       \n"
        "  q                                                             \n"
        "    Exit setup stage                                            \n"
        "  a [dungeon name] [monster type] [num_monsters]                \n"
        "    Append a dungeon to the end of the map's list of dungeons   \n"
        "  p                                                             \n"
        "    Prints the map's list of dungeons                           \n"
        "  s                                                             \n"
        "    Shows the player's current stats                            \n"
        "  i [position] [dungeon name] [monster type] [num_monsters]     \n"
        "    Inserts a dungeon to the specified position in the map      \n"
        "  t [dungeon position] [item type] [points]                     \n"
        "    Inserts an item into the specified dungeon's list of items  \n"
        "================================================================\n");
}

// Prints the help table, explaining the different commands available when
// playing the game
//
// Parameters:
//      nothing
//
// Returns:
//      nothing
//
static void do_print_help_gameplay(void)
{
    printf(
        "=======================[  1511 Dungeon  ]=======================\n"
        "      =============[  Gameplay: Usage Info  ]===============    \n"
        "  ?                                                             \n"
        "    Show gameplay usage info                                    \n"
        "  q                                                             \n"
        "    Exit gameplay                                               \n"
        "  p                                                             \n"
        "    Prints the map's list of dungeons                           \n"
        "  s                                                             \n"
        "    Shows the player's current stats                            \n"
        "  d                                                             \n"
        "    Prints the current dungeon's details                        \n"
        "  >                                                             \n"
        "    Move to the next dungeon in the map                         \n"
        "  <                                                             \n"
        "    Move to the previous dungeon in the map                     \n"
        "  !                                                             \n"
        "    Physically attack monsters in current dungeon               \n"
        "  #                                                             \n"
        "    Magically attack monsters in current dungeon                \n"
        "  P                                                             \n"
        "    Activate the player's class power                           \n"
        "  c [item number]                                               \n"
        "    Collect the specified item from current dungeon             \n"
        "  u [item number]                                               \n"
        "    Use the specified item from the player's inventory          \n"
        "  T                                                             \n"
        "    Teleport to the furthest dungeon                            \n"
        "  b                                                             \n"
        "    Fight the boss in the current dungeon                       \n"
        "================================================================\n");
}

////////////////////////////////////////////////////////////////////////////////
//                             Stage 1 Functions                              //
////////////////////////////////////////////////////////////////////////////////

// Handles calling and printing for append_dungeon in dungeon.c
//
// Parameters:
//      map         - pointer to the dungeon map, containing list of dungeons,
//                    the player, and the points required to win.
//      line        - the scanned in line from stdin containing the commands
//
// Returns:
//      nothing
//
static void do_append_dungeon(struct map *map, char *line)
{
    char command;
    char dungeon_name[MAX_STR_LEN];
    int monster_type;
    int num_monsters;

    // TODO DO NOT USE SCNAF %S
    int scanf_return = sscanf(line, "%c %31s %d %d", &command, dungeon_name,
                              &monster_type, &num_monsters);

    if (scanf_return != 4) {
        printf(
            "ERROR: Invalid command. The correct format is %c "
            "[dungeon name] [monster type] [num monsters] \n",
            command);
        return;
    }

    int append_dungeon_return =
        append_dungeon(map, dungeon_name, monster_type, num_monsters);

    if (append_dungeon_return == INVALID_NAME) {
        printf("ERROR: Invalid name. This name already exists in the map.\n");
    } else if (append_dungeon_return == INVALID_MONSTER) {
        printf(
            "ERROR: Invalid monster type. Monster type should be one of:\n"
            "\t1: SLIME\n"
            "\t2: GOBLIN\n"
            "\t3: SKELETON\n"
            "\t4: WOLF\n");
    } else if (append_dungeon_return == INVALID_AMOUNT) {
        printf(
            "ERROR: Invalid monster number. Monster number must be between "
            "%d and %d.\n",
            MIN_MONSTERS, MAX_MONSTERS);
    } else if (append_dungeon_return == VALID) {
        printf("%s has been added as a dungeon to the map!\n", dungeon_name);
    } else {
        printf("ERROR: Unknown return value.\n");
    }
}

// Handles calling and printing for print_map in dungeon.c
//
// Parameters:
//      map         - pointer to the dungeon map, containing list of dungeons,
//                    the player, and the points required to win.
//
// Returns:
//      nothing
//
static void do_print_map(struct map *map) { print_map(map); }

// Handles calling and printing for final_boss in dungeon.c
//
// Parameters:
//      map         - pointer to the dungeon map, containing list of dungeons,
//                    the player, and the points required to win.
//      line        - the scanned in line from stdin containing the commands
//
// Returns:
//      nothing
//
static void do_final_boss(struct map *map, char *line)
{
    int required_item;

    int scanf_return = sscanf(line, "%d", &required_item);

    if (scanf_return != 1) {
        printf(
            "ERROR: Invalid input. The correct format is "
            "[required item]\n");
        handle_final_boss(map);
        return;
    }

    int final_boss_return = final_boss(map, required_item);

    if (final_boss_return == INVALID_ITEM) {
        printf(
            "ERROR: Invalid item type. Item type should be one of:\n"
            "\t0: PHYSICAL WEAPON\n"
            "\t1: MAGICAL TOME\n"
            "\t2: ARMOR\n"
            "\t3: HEALTH POTION\n"
            "\t4: TREASURE\n");
        handle_final_boss(map);
    } else if (final_boss_return == VALID) {
        printf("The final boss has been added to %s!\n", map->name);
    } else {
        printf("ERROR: Unknown return value.\n");
    }
}

// Handles calling and printing for player_stats in dungeon.c
//
// Parameters:
//      map         - pointer to the dungeon map, containing list of dungeons,
//                    the player, and the points required to win.
//
// Returns:
//      nothing
//
static void do_player_stats(struct map *map) { player_stats(map); }

////////////////////////////////////////////////////////////////////////////////
//                             Stage 2 Functions                              //
////////////////////////////////////////////////////////////////////////////////

// Handles calling and printing for insert_dungeon in dungeon.c
//
// Parameters:
//      map         - pointer to the dungeon map, containing list of dungeons,
//                    the player, and the points required to win.
//      line        - the scanned in line from stdin containing the commands
//
// Returns:
//      nothing
//
static void do_insert_dungeon(struct map *map, char *line)
{
    char command;
    int position;
    char dungeon_name[MAX_STR_LEN];
    int monster_type;
    int num_monsters;

    // TODO DONT SCANF %S STINGS
    int scanf_return = sscanf(line, "%c %d %31s %d %d", &command, &position,
                              dungeon_name, &monster_type, &num_monsters);

    if (scanf_return != 5) {
        printf(
            "ERROR: Invalid command. The correct format is %c "
            "[position] [dungeon name] [monster type]"
            " [num monsters] \n",
            command);
        return;
    }

    int insert_dungeon_return =
        insert_dungeon(map, dungeon_name, monster_type, num_monsters, position);

    if (insert_dungeon_return == INVALID_POSITION) {
        printf("ERROR: Invalid position. Position must be 1 or greater.\n");
    } else if (insert_dungeon_return == INVALID_NAME) {
        printf("ERROR: Invalid name. This name already exists in the map.\n");
    } else if (insert_dungeon_return == INVALID_MONSTER) {
        printf(
            "ERROR: Invalid monster type. Monster type should be one of:\n"
            "\t1: SLIME\n"
            "\t3: GOBLIN\n"
            "\t4: SKELETON\n"
            "\t5: WOLF\n");
    } else if (insert_dungeon_return == INVALID_AMOUNT) {
        printf(
            "ERROR: Invalid monster number. Monster number must be between "
            "%d and %d.\n",
            MIN_MONSTERS, MAX_MONSTERS);
    } else if (insert_dungeon_return == VALID) {
        printf("%s has been added as a dungeon to the map!\n", dungeon_name);
    } else {
        printf("ERROR: Unknown return value.\n");
    }
}

// Handles calling and printing for print_dungeon in dungeon.c
//
// Parameters:
//      map         - pointer to the dungeon map, containing list of dungeons,
//                    the player, and the points required to win.
//
// Returns:
//      nothing
//
static void do_print_dungeon(struct map *map) { print_dungeon(map); }

// Handles calling and printing for move_player in dungeon.c
//
// Parameters:
//      map         - pointer to the dungeon map, containing list of dungeons,
//                    the player, and the points required to win.
//      line        - the scanned in line from stdin containing the commands
//
// Returns:
//      nothing
//
static void do_move_player(struct map *map, char *line)
{
    char command;

    int scanf_return = sscanf(line, "%c", &command);

    if (scanf_return != 1) {
        printf("ERROR: Invalid command. The correct format is %c\n", command);
        return;
    }

    int move_player_return = move_player(map, command);

    if (move_player_return == INVALID) {
        printf(
            "ERROR: Invalid move. There must be a dungeon next to the player"
            " to move into.\n");
    } else if (move_player_return == VALID && command == NEXT_DUNGEON) {
        printf("Moved into the next dungeon\n");
    } else if (move_player_return == VALID && command == PREVIOUS_DUNGEON) {
        printf("Moved into the previous dungeon\n");
    } else {
        printf("ERROR: Unknown return value.\n");
    }
}

// Handles calling and printing for fight in dungeon.c
//
// Parameters:
//      map         - pointer to the dungeon map, containing list of dungeons,
//                    the player, and the points required to win.
//      line        - the scanned in line from stdin containing the commands
//
// Returns:
//      nothing
//
static void do_fight(struct map *map, char *line)
{
    char command;

    int scanf_return = sscanf(line, "%c", &command);

    if (scanf_return != 1) {
        printf("ERROR: Invalid command. The correct format is %c\n", command);
        return;
    }

    int fight_return = fight(map, command);

    if (fight_return == INVALID) {
        printf("ERROR: There are no monsters to fight in this dungeon!\n");
    } else if (fight_return == VALID) {
        printf("A battle has raged!\n");
    } else {
        printf("ERROR: Unknown return value.\n");
    }
}

// Handles calling and printing for end_turn in dungeon.c
//
// Parameters:
//      map         - pointer to the dungeon map, containing list of dungeons,
//                    the player, and the points required to win.
//
// Returns:
//      an int indicating whether or not the game should continue, and
//      if the game has ended, whether the player has won or lost
//
static int do_end_turn(struct map *map)
{
    int end_turn_return = end_turn(map);

    if (end_turn_return == PLAYER_DEFEATED) {
        printf("%s has been defeated!\n", get_player_name(map->player));
    } else if (end_turn_return == WON_MONSTERS) {
        printf(
            "%s the %s has won by collecting %d points and defeating all "
            "monsters!\n",
            get_player_name(map->player), class_string(map->player),
            map->win_requirement);
    } else if (end_turn_return == WON_BOSS) {
        printf(
            "%s the %s has won by collecting %d points and defeating the "
            "final boss!\n",
            get_player_name(map->player), class_string(map->player),
            map->win_requirement);
    }
    return end_turn_return;
}

// Handles calling and printing for class_power in dungeon.c
//
// Parameters:
//      map         - pointer to the dungeon map, containing list of dungeons,
//                    the player, and the points required to win.
//
// Returns:
//      nothing
//
static void do_class_power(struct map *map)
{
    int class_power_return = class_power(map);

    if (class_power_return == INVALID) {
        printf("ERROR: %s has already used their class power.\n",
               get_player_name(map->player));
    } else if (class_power_return == VALID) {
        printf("%s used their class power! This can no longer be used.\n",
               get_player_name(map->player));
    } else {
        printf("ERROR: Unknown return value.\n");
    }
}

////////////////////////////////////////////////////////////////////////////////
//                             Stage 3 Functions                              //
////////////////////////////////////////////////////////////////////////////////

// Handles calling and printing for add_item in dungeon.c
//
// Parameters:
//      map         - pointer to the dungeon map, containing list of dungeons,
//                    the player, and the points required to win.
//      line        - the scanned in line from stdin containing the commands
//
// Returns:
//      nothing
//
static void do_add_item(struct map *map, char *line)
{
    char command;
    int dungeon_number;
    int item_type;
    int points;

    int scanf_return = sscanf(line, "%c %d %d %d", &command, &dungeon_number,
                              &item_type, &points);

    if (scanf_return != 4) {
        printf(
            "ERROR: Invalid command. The correct format is %c "
            "[dungeon number] [item type] [points]\n",
            command);
        return;
    }

    int add_item_return = add_item(map, dungeon_number, item_type, points);

    if (add_item_return == INVALID_DUNGEON) {
        printf(
            "ERROR: Invalid dungeon number. Dungeon number needs to "
            "correspond to a dungeon in the map.\n");
    } else if (add_item_return == INVALID_ITEM) {
        printf(
            "ERROR: Invalid item type. Item type should be one of:\n"
            "\t0: PHYSICAL WEAPON\n"
            "\t1: MAGICAL TOME\n"
            "\t2: ARMOR\n"
            "\t3: HEALTH POTION\n"
            "\t4: TREASURE\n");
    } else if (add_item_return == INVALID_POINTS) {
        printf(
            "ERROR: Invalid point value. Point value must be between %d "
            "and %d.\n",
            MIN_ITEM_POINT_VALUE, MAX_ITEM_POINT_VALUE);
    } else if (add_item_return == VALID) {
        printf("Item successfully added to the dungeon!\n");
    } else {
        printf("ERROR: Unknown return value.\n");
    }
}

// Handles calling and printing for collect_item in dungeon.c
//
// Parameters:
//      map         - pointer to the dungeon map, containing list of dungeons,
//                    the player, and the points required to win.
//      line        - the scanned in line from stdin containing the commands
//
// Returns:
//      nothing
//
static void do_collect_item(struct map *map, char *line)
{
    char command;
    int item_number;

    int scanf_return = sscanf(line, "%c %d", &command, &item_number);

    if (scanf_return != 2) {
        printf(
            "ERROR: Invalid command. The correct format is %c "
            "[item number]\n",
            command);
        return;
    }

    int collect_item_return = collect_item(map, item_number);

    if (collect_item_return == INVALID) {
        printf(
            "ERROR: Invalid item number. Item number needs to "
            "correspond to an item in the current dungeon.\n");
    } else if (collect_item_return == VALID) {
        printf("Item successfully added to %s's inventory!\n",
               get_player_name(map->player));
    } else {
        printf("ERROR: Unknown return value.\n");
    }
}

// Handles calling and printing for use_item in dungeon.c
//
// Parameters:
//      map         - pointer to the dungeon map, containing list of dungeons,
//                    the player, and the points required to win.
//      line        - the scanned in line from stdin containing the commands
//
// Returns:
//      nothing
//
static void do_use_item(struct map *map, char *line)
{
    char command;
    int item_number;

    int scanf_return = sscanf(line, "%c %d", &command, &item_number);

    if (scanf_return != 2) {
        printf(
            "ERROR: Invalid command. The correct format is %c "
            "[item number]\n",
            command);
        return;
    }

    int use_item_return = use_item(map, item_number);

    if (use_item_return == INVALID) {
        printf(
            "ERROR: Invalid item number. Item number needs to "
            "correspond to an item in %s's inventory.\n",
            get_player_name(map->player));
    } else if (use_item_return == VALID) {
        printf("Item successfully used and removed from %s's inventory!\n",
               get_player_name(map->player));
    } else {
        printf("ERROR: Unknown return value.\n");
    }
}

////////////////////////////////////////////////////////////////////////////////
//                             Stage 4 Functions                              //
////////////////////////////////////////////////////////////////////////////////

// Handles calling and printing for teleport in dungeon.c
//
// Parameters:
//      map         - pointer to the dungeon map, containing list of dungeons,
//                    the player, and the points required to win.
//
// Returns:
//      nothing
//
static void do_teleport(struct map *map)
{
    int teleport_return = teleport(map);

    if (teleport_return == INVALID) {
        printf("ERROR: There is no dungeon to teleport to.\n");
    } else if (teleport_return == VALID) {
        printf("%s has teleported!\n", get_player_name(map->player));
    } else {
        printf("ERROR: Unknown return value.\n");
    }
}

// Handles calling and printing for boss_fight in dungeon.c
//
// Parameters:
//      map         - pointer to the dungeon map, containing list of dungeons,
//                    the player, and the points required to win.
//
// Returns:
//      nothing
//
static void do_boss_fight(struct map *map)
{
    int boss_fight_return = boss_fight(map);

    if (boss_fight_return == NO_BOSS) {
        printf("ERROR: There is no boss in this dungeon!\n");
    } else if (boss_fight_return == NO_ITEM) {
        printf(
            "ERROR: %s does not have the required item to "
            "defeat the boss!\n",
            get_player_name(map->player));
    } else if (boss_fight_return == VALID) {
        printf("A battle has raged against the dungeon boss!\n");
    } else {
        printf("ERROR: Unknown return value.\n");
    }
}
