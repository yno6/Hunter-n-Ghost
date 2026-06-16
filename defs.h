#ifndef DEFS_H
#define DEFS_H

#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>


#define MAX_ROOM_NAME 64
#define MAX_HUNTER_NAME 64
#define MAX_ROOMS 24
#define MAX_ROOM_OCCUPANCY 8
#define MAX_CONNECTIONS 8
#define ENTITY_BOREDOM_MAX 15
#define HUNTER_FEAR_MAX 15
#define DEFAULT_GHOST_ID 68057

typedef unsigned char EvidenceByte; // Just giving a helpful name to unsigned char for evidence bitmasks
struct Hunter; //forward declaring hunter for the room struct

enum LogReason {
    LR_EVIDENCE = 0,
    LR_BORED = 1,
    LR_AFRAID = 2
};

enum EvidenceType {
    EV_EMF          = 1 << 0,
    EV_ORBS         = 1 << 1,
    EV_RADIO        = 1 << 2,
    EV_TEMPERATURE  = 1 << 3,
    EV_FINGERPRINTS = 1 << 4,
    EV_WRITING      = 1 << 5,
    EV_INFRARED     = 1 << 6,
};

enum GhostType {
    GH_POLTERGEIST  = EV_FINGERPRINTS | EV_TEMPERATURE | EV_WRITING,
    GH_THE_MIMIC    = EV_FINGERPRINTS | EV_TEMPERATURE | EV_RADIO,
    GH_HANTU        = EV_FINGERPRINTS | EV_TEMPERATURE | EV_ORBS,
    GH_JINN         = EV_FINGERPRINTS | EV_TEMPERATURE | EV_EMF,
    GH_PHANTOM      = EV_FINGERPRINTS | EV_INFRARED    | EV_RADIO,
    GH_BANSHEE      = EV_FINGERPRINTS | EV_INFRARED    | EV_ORBS,
    GH_GORYO        = EV_FINGERPRINTS | EV_INFRARED    | EV_EMF,
    GH_BULLIES      = EV_FINGERPRINTS | EV_WRITING     | EV_RADIO,
    GH_MYLING       = EV_FINGERPRINTS | EV_WRITING     | EV_EMF,
    GH_OBAKE        = EV_FINGERPRINTS | EV_ORBS        | EV_EMF,
    GH_YUREI        = EV_TEMPERATURE  | EV_INFRARED    | EV_ORBS,
    GH_ONI          = EV_TEMPERATURE  | EV_INFRARED    | EV_EMF,
    GH_MOROI        = EV_TEMPERATURE  | EV_WRITING     | EV_RADIO,
    GH_REVENANT     = EV_TEMPERATURE  | EV_WRITING     | EV_ORBS,
    GH_SHADE        = EV_TEMPERATURE  | EV_WRITING     | EV_EMF,
    GH_ONRYO        = EV_TEMPERATURE  | EV_RADIO       | EV_ORBS,
    GH_THE_TWINS    = EV_TEMPERATURE  | EV_RADIO       | EV_EMF,
    GH_DEOGEN       = EV_INFRARED     | EV_WRITING     | EV_RADIO,
    GH_THAYE        = EV_INFRARED     | EV_WRITING     | EV_ORBS,
    GH_YOKAI        = EV_INFRARED     | EV_RADIO       | EV_ORBS,
    GH_WRAITH       = EV_INFRARED     | EV_RADIO       | EV_EMF,
    GH_RAIJU        = EV_INFRARED     | EV_ORBS        | EV_EMF,
    GH_MARE         = EV_WRITING      | EV_RADIO       | EV_ORBS,
    GH_SPIRIT       = EV_WRITING      | EV_RADIO       | EV_EMF,
};

struct CaseFile {
    EvidenceByte collected; // Union of all of the evidence bits collected between all hunters
    bool         solved;    // True when >=3 unique bits set
    sem_t        mutex;     // Used for synchronizing both fields when multithreading
};

// Implement here based on the requirements, should all be allocated to the House structure
struct Room {
    char name[MAX_ROOM_NAME];                   //room name 
    struct Room* connections[MAX_CONNECTIONS];  //pointers to connected rooms
    int connection_count;                       //number of connections
    struct Ghost* ghost;                        //pointer to ghost in the room (NULL if ghost is not in the room)
    struct Hunter* hunters[MAX_ROOM_OCCUPANCY]; //array of hunters currently in room
    int hunter_count;                           //number of hunters in the room
    bool is_exit;                               //boolean for if the room is the exit
    EvidenceByte evidence;                      //evidence currently in the room
    sem_t mutex;                                //semaphore for multithreading
};

// Room Node for linked list
struct RoomNode {
    struct Room* room;
    struct RoomNode* next;
};

// RoomStack (stack of rooms, singly linked list)
struct RoomStack {
    struct RoomNode* head;
};

// Implement here based on the requirements, should be allocated to the House structure
struct Ghost {
    int id;                     //ghost's id 
    enum GhostType type;        //represents the type of ghost & the evidence required to catch it
    struct Room* current_room;  //pointer to current room ghost is in
    int boredom;                //boredom level of ghost 
    bool exited;                //boolean flag to keep track if ghost has exited or not
};

// Can be either stack or heap allocated
struct House {
    struct Room rooms[MAX_ROOMS];   //fixed array of rooms
    int room_count;                 //number of rooms in the house currently
    struct Room* starting_room;     //pointer to the starting room 
    struct Hunter* hunters[MAX_ROOM_OCCUPANCY * 2];         //dynamic array of hunters
    struct CaseFile case_file;      //shared case file structure for collected evidence
    struct Ghost ghost;             //the ghost data
};


//Allocated to the heap
struct Hunter {
    char name[MAX_HUNTER_NAME];         //hunter's name
    int id;                             //hunter's id
    struct Room* current_room;          //pointer to the current room
    struct CaseFile* shared_case_file;  //pointer to the shared case file struct
    enum EvidenceType current_device;   //the current device in use to detect evidence
    struct RoomStack rooms_visited;     //stack of the rooms visisted since the last time they were in the starting room/exit
    int fear;                           //hunter's fear level
    int boredom;                        //hunter's boredom level
    enum LogReason exit_reason;         //the reason hunter exited the simulation (if they exited)
    bool exited;                        //boolean to check if hunter exited
    bool return_to_van;
};

/* The provided `house_populate_rooms()` function requires the following functions.
   You are free to rename them and change their parameters and modify house_populate_rooms()
   as needed as long as the house has the correct rooms and connections after calling it.
*/

void room_init(struct Room* room, const char* name, bool is_exit);
void room_connect(struct Room* a, struct Room* b); // Bidirectional connection

#endif // DEFS_H
