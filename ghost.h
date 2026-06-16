#ifndef GHOST_H
#define GHOST_H

#include "defs.h"
#include "room.h"

/**
 * @brief Initialize a ghost with its type and house.
 * @param[in,out] ghost Ghost structure to initialize.
 * @param[in] house House where ghost exists.
 */
void ghost_init(struct Ghost* ghost, struct House* house);

/**
 * @brief Perform one turn of ghost behavior (move, haunt, etc.).
 * @param[in,out] ghost Ghost performing action.
 */
void ghost_take_turn(struct Ghost* ghost);

/**
 * @brief Main thread function for ghost behavior.
 * @param[in] arg Pointer to ghost struct.
 * @return NULL
 */
void* ghost_thread(void* arg);

#endif