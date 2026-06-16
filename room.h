#ifndef ROOM_H
#define ROOM_H

#include <stdio.h>      
#include <stdlib.h>   
#include <string.h>
#include "room.h"
#include "defs.h"
#include <stdbool.h>

/**
 * @brief Initialize a room structure.
 * @param[in,out] room Pointer to the room to initialize.
 * @param[in] name Room name (string).
 * @param[in] is_exit True if this room is an exit.
 */void room_init(struct Room* room, const char* name, bool is_exit);
 
 /**
 * @brief Connect two rooms bidirectionally.
 * @param[in,out] room1 First room.
 * @param[in,out] room2 Second room.
 */
void room_connect(struct Room* room1, struct Room* room2);


/**
 * @brief Add a hunter to a room.
 * @param[in,out] room Room to add the hunter to.
 * @param[in] hunter Hunter to add.
 * @return True if hunter was successfully added, false if room is full, so hunter not added.
 */
bool room_add_hunter(struct Room* room, struct Hunter* hunter);

/**
 * @brief Remove a hunter from a room.
 * @param[in,out] room Room to remove the hunter from.
 * @param[in] hunter Hunter to remove.
 */
void room_remove_hunter(struct Room* room, struct Hunter* hunter);


/**
 * @brief Add evidence to a room.
 * @param[in,out] room Room where evidence is added.
 * @param[in] evidence Evidence type to add.
 */
void room_add_evidence(struct Room* room, enum EvidenceType evidence);

/**
 * @brief Remove evidence from a room.
 * @param[in,out] room Room to clear evidence from.
 * @param[in] evidence Evidence type to remove.
 */
void room_clear_evidence(struct Room* room, enum EvidenceType evidence);

/**
 * @brief Check if a room contains a specific evidence type.
 * @param[in] room Room to check.
 * @param[in] evidence Evidence type to check for.
 * @return True if room has the evidence, false otherwise.
 */
bool room_has_evidence(struct Room* room, enum EvidenceType evidence);


/**
 * @brief Initialize a RoomStack structure.
 * @param[in,out] stack Stack to initialize.
 */
void roomstack_init(struct RoomStack* stack);

/**
 * @brief Push a room onto a RoomStack.
 * @param[in,out] stack Stack to push onto.
 * @param[in] room Room to push.
 */
void roomstack_push(struct RoomStack* stack, struct Room* room);

/**
 * @brief Pop a room from a RoomStack.
 * @param[in,out] stack Stack to pop from.
 * @return Pointer to the room popped, or NULL if stack is empty.
 */
struct Room* roomstack_pop(struct RoomStack* stack);

/**
 * @brief Cleanup a RoomStack, freeing any allocated memory.
 * @param[in,out] stack Stack to clean up.
 */
void roomstack_cleanup(struct RoomStack* stack);

#endif