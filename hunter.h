#ifndef HUNTER_H
#define HUNTER_H

#include "defs.h"
#include "room.h"
#include <stdbool.h>


/**
 * @brief Initialize the hunter collection for the house.
 * @param[in,out] house House containing hunters.
 */
void huntercollection_init(struct House* house);

/**
 * @brief Append a hunter to the house collection.
 * @param[in,out] house House to add hunter to.
 * @param[in] hunter Hunter to append.
 * @return True if successfully added, false if collection full.
 */
bool huntercollection_append(struct House* house, struct Hunter* hunter);

/**
 * @brief Cleanup all hunters in a house, freeing memory.
 * @param[in,out] house House containing hunters.
 */
void huntercollection_cleanup(struct House* house);


/**
 * @brief Initialize a hunter with name, ID, and house.
 * @param[in,out] hunter Hunter to initialize.
 * @param[in] name Hunter name string.
 * @param[in] id Hunter ID number.
 * @param[in] house Pointer to the house structure.
 */
void hunter_init(struct Hunter* hunter, const char* name, int id, struct House* house);

/**
 * @brief Cleanup a hunter, freeing resources.
 * @param[in,out] hunter Hunter to cleanup.
 */
void hunter_cleanup(struct Hunter* hunter);

/**
 * @brief Mark a hunter as exited from the house.
 * @param[in,out] hunter Hunter to exit.
 * @param[in] reason Reason for exiting.
 */
void hunter_exit(struct Hunter* hunter, enum LogReason reason);

/**
 * @brief Main thread function for hunter behavior.
 * @param[in] arg Pointer to hunter struct.
 * @return NULL
 */
void* hunter_thread(void* arg);

#endif