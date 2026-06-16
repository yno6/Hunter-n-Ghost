#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include "defs.h"
#include "helpers.h"
#include "hunter.h"


//========================= HUNTER COLLECTION FUNCTIONS =========================//

void huntercollection_init(struct House* house) {
    for (int i = 0; i < MAX_ROOM_OCCUPANCY * 2; i++) {
        house->hunters[i] = NULL;
    }
}

bool huntercollection_append(struct House* house, struct Hunter* hunter) {
    for (int i = 0; i < MAX_ROOM_OCCUPANCY * 2; i++) {
        if (house->hunters[i] == NULL) {
            house->hunters[i] = hunter;
            return true;
        }
    }
    return false;
}

void huntercollection_cleanup(struct House* house) {
    for (int i = 0; i < MAX_ROOM_OCCUPANCY * 2; i++) {
        if (house->hunters[i]) {
            hunter_cleanup(house->hunters[i]);
            free(house->hunters[i]);
            house->hunters[i] = NULL;
        }
    }
}


//============================== HUNTER FUNCTIONS ==============================//

//initialize a hunter given its name, id and house struct 
void hunter_init(struct Hunter* hunter, const char* name, int id, struct House* house) {
    //hunter's name and id initialization as per user input
    strncpy(hunter->name, name, MAX_HUNTER_NAME - 1);
    hunter->name[MAX_HUNTER_NAME - 1] = '\0';
    hunter->id = id;

    //initialize all the other hunter fields
    hunter->current_room = house->starting_room;    //hunter starts in van
    hunter->shared_case_file = &house->case_file;   //share casefile
    hunter->fear = 0;                               //initial zero fear 
    hunter->boredom = 0;                            //initial boredome is zero
    hunter->exited = false;                         //hunter is still present, so not exited 
    hunter->exit_reason = LR_EVIDENCE;              //default exit reason
    hunter->return_to_van = false;                  

    //randomnly pick a number/device and assign it to the hunter
    const enum EvidenceType* devices = NULL; 
    int device_count = get_all_evidence_types(&devices); //get count (should always be seven)
    hunter->current_device = devices[rand_int_threadsafe(0, device_count - 1)]; 
    
    //initialize the breadcrumb stack (roomstack)
    roomstack_init(&hunter->rooms_visited);

    //add the hunter to van, if space available
    struct Room* van = house->starting_room;
    sem_wait(&van->mutex);
    if (van->hunter_count < MAX_ROOM_OCCUPANCY) {
        van->hunters[van->hunter_count] = hunter;
        van->hunter_count++;
    }
    sem_post(&van->mutex);

    log_hunter_init(id, van->name, name, hunter->current_device);
}

//clean up hunter/remove from room stacks and free up memory
void hunter_cleanup(struct Hunter* hunter) {
    roomstack_cleanup(&hunter->rooms_visited);
}


//-------------- Helper Functions for hunter exit & device swap --------------//

void hunter_exit(struct Hunter* hunter, enum LogReason reason) {
    //remove hunter from its current room safely
    room_remove_hunter(hunter->current_room, hunter);

    hunter->exited = true;
    hunter->exit_reason = reason;

    //log exit / final state 
    log_exit(hunter->id, hunter->boredom, hunter->fear, hunter->current_room->name, hunter->current_device, reason);
}

void hunter_swap_device(struct Hunter* hunter) {

    const enum EvidenceType* devices = NULL;
    int device_count = get_all_evidence_types(&devices);

    enum EvidenceType old_device = hunter->current_device;
    enum EvidenceType new_device;

    //keep picking a new device randomnly, until it's different from the old/current one
    do {
        new_device = devices[rand_int_threadsafe(0, device_count - 1)];
    } while (new_device == old_device);

    //update current device info and log the swap
    hunter->current_device = new_device;
    log_swap(hunter->id, hunter->boredom, hunter->fear, old_device, new_device);
}



//============================== HUNTER TAKE TURN ==============================//

void hunter_take_turn(struct Hunter* hunter) {
    struct Room* room = hunter->current_room;

    //R-17 update stats (ghost check)
    sem_wait(&room->mutex);
    if (room->ghost && !room->ghost->exited) {  //if ghost is present
        hunter->boredom = 0;  //set boredom to zero
        hunter->fear++;       //increase fear by 1
    } 
    else {//ghost not present in the room 
        hunter->boredom++;    //icrease boredom by 1
    }
    sem_post(&room->mutex);

    //------------------------------------------------------------------------------------------------
    //R-18 Van/Exit Room Check

    if (room->is_exit) {//if the hunter is in the van
        
        //clear the hunter's breadcrumb stack (RoomStack)
        roomstack_cleanup(&hunter->rooms_visited);

        //check if casefile is solved (enough evidence to represent a valid ghost, so exit the simulation)
        sem_wait(&hunter->shared_case_file->mutex);
        if (hunter->shared_case_file->solved) {
            sem_post(&hunter->shared_case_file->mutex);
            hunter_exit(hunter, LR_EVIDENCE);
               return; 
        }
        sem_post(&hunter->shared_case_file->mutex);

        //swap device, only if hunter is still exploring
        hunter_swap_device(hunter);
        hunter->return_to_van = false;
    }

    //------------------------------------------------------------------------------------------------
    // R-19 Fear/Boredom exit
    if (hunter->boredom >= ENTITY_BOREDOM_MAX) {
        hunter_exit(hunter, LR_BORED);
        return;
    }
    if (hunter->fear >= HUNTER_FEAR_MAX) {
        hunter_exit(hunter, LR_AFRAID);
        return;
    }

    //------------------------------------------------------------------------------------------------
    //R-20 Attempt to gather evidence
    
    bool found_evidence = false;
    sem_wait(&room->mutex);

    //if room has evidence/matching evidence found
    if (room_has_evidence(room, hunter->current_device)) {
        //clear evidence from room
        room_clear_evidence(room, hunter->current_device);
        sem_post(&room->mutex);
        
        //add evidence to the shared casefile
        sem_wait(&hunter->shared_case_file->mutex);
        // evidence_set(&hunter->shared_case_file->collected, hunter->current_device);
        hunter->shared_case_file->collected |= (1 << hunter->current_device);
        sem_post(&hunter->shared_case_file->mutex);
        
        //log evidence
        log_evidence(hunter->id, hunter->boredom, hunter->fear, room->name, hunter->current_device);

        //now hunter can return to van, unless already there
        if (!room->is_exit) hunter->return_to_van = true; 
        
        found_evidence = true;
        log_return_to_van(hunter->id, hunter->boredom, hunter->fear, room->name, hunter->current_device, true);

    }
    else {//else, no evidence found, unlock room 
        sem_post(&room->mutex);
    }

    //10% chance to return to van
    if (!found_evidence && rand_int_threadsafe(0, 100) < 10) {
        hunter->return_to_van = true;
    }


    //------------------------------------------------------------------------------------------------
    // R-21 Movement (using the Breadcrumb Stack)
    struct Room* target_room = NULL;

    // R-21.1 If hunter is returning to van & stack not empty, pop next room
    if (hunter->return_to_van && hunter->rooms_visited.head != NULL) {
        target_room = roomstack_pop(&hunter->rooms_visited);
    } 
    else {
        // R-21.2 Otherwise exploring, pick random connection from current room
        sem_wait(&room->mutex);
        int count = room->connection_count;
        if (count > 0) {
            int rand_index = rand_int_threadsafe(0, count - 1);
            target_room = room->connections[rand_index];
        }
        sem_post(&room->mutex);
    }

    // R-21.3 if target is valid and different from current, try to move
    if (target_room && target_room != room) {
       
        sem_wait(&target_room->mutex);
        if (target_room->hunter_count < MAX_ROOM_OCCUPANCY) {  // Not full
            sem_post(&target_room->mutex);

            //remove hunter from current room
            room_remove_hunter(room, hunter);  

            //add hunter to the target room
            sem_wait(&target_room->mutex);
            target_room->hunters[target_room->hunter_count] = hunter; 
            target_room->hunter_count++;
            hunter->current_room = target_room;
            sem_post(&target_room->mutex);

            //log successful move
            log_move(hunter->id, hunter->boredom, hunter->fear, room->name, target_room->name, hunter->current_device);

            //if exploring (hunter has not returned to van or exited), push new room onto breadcrumb stack
            if (!hunter->return_to_van && !target_room->is_exit) {
                roomstack_push(&hunter->rooms_visited, target_room);
            }
        } else {
            //otherwise, target room full, stay in the current room
            sem_post(&target_room->mutex);
        }
    }
}


void* hunter_thread(void* arg) {
    struct Hunter* hunter = (struct Hunter*)arg;

    while (!hunter->exited) {
        hunter_take_turn(hunter);
    }
    
    return NULL;
} 

