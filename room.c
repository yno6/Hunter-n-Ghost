#include <string.h>
#include <stdlib.h>
#include <semaphore.h>
#include "defs.h"
#include "helpers.h"
#include "room.h"


//========================= ROOM FUNCTIONS =========================//

//room initialization
void room_init(struct Room* room, const char* name, bool is_exit) {
    //copy the name string
    strncpy(room->name, name, MAX_ROOM_NAME - 1);
    room->name[MAX_ROOM_NAME - 1] = '\0';  //ensure null termination

    //set basic values
    room->is_exit = is_exit;
    room->connection_count = 0;
    room->ghost = NULL;
    room->hunter_count = 0;
    room->evidence = 0;

    sem_init(&room->mutex, 0, 1);

    //initialize connections and hunters arrays to NULL
    for (int i = 0; i < MAX_ROOM_OCCUPANCY; i++) {
        room->hunters[i] = NULL;
    }
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        room->connections[i] = NULL;
    }

}

//function to connect two rooms (both ways/bidirectional)
void room_connect(struct Room* a, struct Room* b) {
    //add  (A -to-> B) connection if space available (bidirectional)
    if (a->connection_count < MAX_CONNECTIONS) {
        a->connections[a->connection_count++] = b;
    }
    //add (B -to-> A) connection if space available (bidirectional)
    if (b->connection_count < MAX_CONNECTIONS) {
        b->connections[b->connection_count++] = a;
    }
}


//function to add a hunter to a specific room
bool room_add_hunter(struct Room* room, struct Hunter* hunter) {
    bool added = false;

    sem_wait(&room->mutex);

    if (room->hunter_count < MAX_ROOM_OCCUPANCY) {
        room->hunters[room->hunter_count] = hunter;
        room->hunter_count++;
        added = true;
    }

    sem_post(&room->mutex);
    return added;
}


//function to remove a hunter from a specific room (probably current room)
void room_remove_hunter(struct Room* room, struct Hunter* hunter) {
    sem_wait(&room->mutex);

    for (int i = 0; i < room->hunter_count; i++) {
        if (room->hunters[i] == hunter) {
            // Shift remaining hunters left to fill the gap
            for (int j = i; j < room->hunter_count - 1; j++) {
                room->hunters[j] = room->hunters[j + 1];
            }
            room->hunter_count--;
            break;
        }
    }
    sem_post(&room->mutex);
}


//========== EVIDENCE FUNCTIONS (CORE/FOR DIRECT HUNTER USE) ===========//

void evidence_set(EvidenceByte* evidence, enum EvidenceType type) {
    *evidence |= type;
}

void evidence_clear(EvidenceByte* evidence, enum EvidenceType type) {
    *evidence &= ~type;
}

bool evidence_get(EvidenceByte evidence, enum EvidenceType type) {
    return (evidence & type) != 0;
}



//====================== ROOM EVIDENCE FUNCTIONS ======================//

void room_add_evidence(struct Room* room, enum EvidenceType evidence) {
    //ghost adds evidence matching its type (bitwise OR)
    evidence_set(&room->evidence, evidence);
}

bool room_has_evidence(struct Room* room, enum EvidenceType device) {
    //hunter checks if room has matching evidence (bitwise AND)
    return evidence_get(room->evidence, device);
}

void room_clear_evidence(struct Room* room, enum EvidenceType evidence) {
    //hunter collects evidence, so clear bit from the room
    evidence_clear(&room->evidence, evidence);
}



//========================= ROOM STACK FUNCTIONS =========================//

//initialize a roomstack (empty/NULL)
void roomstack_init(struct RoomStack* stack) {
    stack->head = NULL;
}

//add/push a room into the roomStack 
void roomstack_push(struct RoomStack* stack, struct Room* room) {
    struct RoomNode* node = malloc(sizeof(struct RoomNode));
    node->room = room;
    node->next = stack->head;
    stack->head = node;
}

//pop/remove the top most room from the RoomStack
struct Room* roomstack_pop(struct RoomStack* stack) {
    if (!stack->head) return NULL;
    struct RoomNode* top = stack->head;
    struct Room* room = top->room;
    stack->head = top->next;
    free(top);
    return room;
}

//clear the roomstack, removing all the rooms, freeing all the space 
void roomstack_cleanup(struct RoomStack* stack) {
    while (stack->head) {
        struct RoomNode* temp = stack->head;
        stack->head = stack->head->next;
        free(temp);
    }
}