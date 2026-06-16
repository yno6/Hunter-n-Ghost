#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "defs.h"
#include "helpers.h"
#include <semaphore.h>
#include "room.h"


//Initializing the ghost
void ghost_init(struct Ghost* ghost, struct House* house) {
    //12.1 assign a random room from the number of rooms available for the ghost to start
    ghost->current_room = &house->rooms[rand_int_threadsafe(1, house->room_count)];

    //12.2 randomnly assign the ghost a ghost type
    const enum GhostType* ghost_types;
    int count = get_all_ghost_types(&ghost_types);
    ghost->type = ghost_types[rand_int_threadsafe(0, count)];

    //12.3 ghost is assigned the default ID defined in defs.h
    ghost->id = DEFAULT_GHOST_ID;

    //initialize boredome and exit flag
    ghost->boredom = 0;
    ghost->exited = false;

    //register the ghost in the current room
    ghost->current_room->ghost = ghost; 

    //log the ghost initialization
    log_ghost_init(ghost->id, ghost->current_room->name, ghost->type);
}


//Helper method to pick evidence
//Pick a single evidence type from a ghost's evidence mask
static enum EvidenceType pick_evidence(enum GhostType type) {
    const enum EvidenceType* all_evidence;
    int count = get_all_evidence_types(&all_evidence);

    //count how many valid evidence types this ghost has
    int valid_count = 0;
    for (int i = 0; i < count; i++) {
        if (type & all_evidence[i]) valid_count++;
    }

    if (valid_count == 0) return EV_EMF; // fallback

    //pick a random index among the valid evidence
    int random_index = rand_int_threadsafe(0, valid_count);

    //return the evidence corresponding to that index
    for (int i = 0; i < count; i++) {
        if (type & all_evidence[i]) {
            if (random_index-- == 0) return all_evidence[i];
        }
    }

    return EV_EMF; // fallback, should never hit
}

void ghost_take_turn(struct Ghost* ghost) {
    struct Room* room = ghost->current_room;

    //locking current room semaphore to safely access data 
    sem_wait(&room->mutex); 

    //12.13 checking if hunters are in the current room 
    if(room->hunter_count > 0) {
        ghost->boredom = 0; //reset boredom to 0, hunters in room, ghost cannot move
    }
    else {
        ghost->boredom++; //increase boredom by 1, no hunters in room
    }

    //12.14 if boredom exceed max boredom then ghost exits simulation 
    if(ghost->boredom > ENTITY_BOREDOM_MAX) {
        log_ghost_exit(ghost->id, ghost->boredom, room->name); //log the exit
        ghost->exited = true; //exit ghost
        room->ghost = NULL;
        sem_post(&room->mutex); //unlock current room
        return; 
    }
    sem_post(&room->mutex);

    //12.15 randomly take action (haunt, move, or idle)
    int action = rand_int_threadsafe(0,3);

    switch(action) {
        case 0: //haunt (add evidence into the room to help identify the ghost)
            enum EvidenceType e = pick_evidence(ghost->type);
            sem_wait(&room->mutex);
            room_add_evidence(room, e);  //using helper method
            sem_post(&room->mutex);
            log_ghost_evidence(ghost->id, ghost->boredom, room->name, e);
            break;
    
        case 1: //move (only if no hunters in the room)
            sem_wait(&room->mutex); //lock room 

            int count = room->connection_count; //room connection count 

            //cannot move if hunters present or no connections
            if (room->hunter_count > 0 || count == 0) {
                sem_post(&room->mutex);
                log_ghost_idle(ghost->id, ghost->boredom, room->name);
                break; 
            }

            int next_idx = rand_int_threadsafe(0, count);
            struct Room* next_room = room->connections[next_idx];

            //lock next room
            sem_wait(&next_room->mutex);

            //double-check hunter didnt enter while locking 
            if(room->hunter_count == 0 && next_room->hunter_count < MAX_ROOM_OCCUPANCY) {
                room->ghost = NULL;
                ghost->current_room = next_room; 
                next_room->ghost = ghost; 

                log_ghost_move(ghost->id, ghost->boredom, room->name, next_room->name);
            }

            //unlock, since no move (possibly a hunter in the room)
            sem_post(&next_room->mutex);
            sem_post(&room->mutex);
            log_ghost_idle(ghost->id, ghost->boredom, room->name); 
            break;

        case 2: //idling/do nothing
            log_ghost_idle(ghost->id, ghost->boredom, room->name);
            break;
    }
}

void* ghost_thread(void* arg) {
    struct Ghost* ghost = (struct Ghost*)arg;
    
    while (!ghost->exited) {
        ghost_take_turn(ghost);
        // usleep(100000); 
    }
    
    return NULL;
}

