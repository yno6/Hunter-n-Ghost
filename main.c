#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include "defs.h"
#include "helpers.h"
#include "room.h"
#include "hunter.h"
#include "ghost.h"

int main() {
    srand((unsigned)time(NULL));

    //initialise house and ghost 
    struct House house;
    house_populate_rooms(&house);

    struct Ghost ghost;
    ghost_init(&ghost, &house);

    //initialize CaseFile semaphore
    sem_init(&house.case_file.mutex, 0, 1);
    house.case_file.collected = 0;
    house.case_file.solved = false;

    //initialize hunter collection
    huntercollection_init(&house);

    printf("\n=========================== Willow House Investigation ===========================\n");

    //interactive hunter input 
    char name[MAX_HUNTER_NAME];
    int id;
    int hunter_count = 0;

    while (1) {
        printf("Enter hunter name (max 63 characters) or 'done' to finish: ");
        if (scanf("%63s", name) != 1) break; //if no name (letters) was entered, then break/stop prompting user
        if (strcmp(name, "done") == 0) break; //if done was entered, then break/stop prompting user

        printf("Enter hunter ID (integer): ");
        if (scanf("%d", &id) != 1) {
            printf("Invalid ID. Try again.\n");
            continue;
        }

        struct Hunter* hunter = malloc(sizeof(struct Hunter));
        if (!hunter) continue;

        
        hunter_init(hunter, name, id, &house);

        if (huntercollection_append(&house, hunter)) {
            hunter_count++;
        } else {
            hunter_cleanup(hunter);
            free(hunter);
        }
    }


    //start ghost thread
    pthread_t ghostThread;
    pthread_create(&ghostThread, NULL, ghost_thread, &ghost);

    //start hunter thread
    pthread_t* hunter_threads = malloc(sizeof(pthread_t) * hunter_count);
    for (int i = 0; i < hunter_count; i++) {
        pthread_create(&hunter_threads[i], NULL, hunter_thread, house.hunters[i]);
    }

    //wait for threads to finishing before joining together
    pthread_join(ghostThread, NULL);
    for (int i = 0; i < hunter_count; i++) {
        pthread_join(hunter_threads[i], NULL);
    }

    //Display result
    printf("\n================= Investigation Results =================\n");

    //display reasons for each hunter's exit
    for (int i = 0; i < hunter_count; i++) {
        struct Hunter* h = house.hunters[i];
        printf("%s (%d) exited because of [%s] (bored = %d, fear = %d)\n", h->name, h->id, exit_reason_to_string(h->exit_reason), h->boredom, h->fear);
    }

    //display shared CaseFile checklist
    printf("\nShared CaseFile checklist:\n");
    const enum EvidenceType* all_evidence;
    int evidence_count = get_all_evidence_types(&all_evidence);

    for (int i = 0; i < evidence_count; i++) {
        bool found = house.case_file.collected & all_evidence[i];
        printf(" - [%s] %s\n", found ? "✓" : " ", evidence_to_string(all_evidence[i]));
    }

    //Victory Result
    int exited_after_identifying = 0;
    for (int i = 0; i < hunter_count; i++) {
        struct Hunter* h = house.hunters[i];
        if (h->exited && house.case_file.solved) exited_after_identifying++;
    }

    printf("\nVictory Results:\n------------------------\n");
    printf("- Hunters exited after identifying the ghost: [%d/%d]\n", exited_after_identifying, hunter_count);
    printf("- Ghost guess: %s\n", house.case_file.solved ? ghost_to_string(ghost.type) : "N/A");
    printf("- Actual Ghost Type: %s\n\n", ghost_to_string(ghost.type));
    printf("Overall Result: %s\n", house.case_file.solved ? "Hunters Win!" : "Ghost Win!");

    //cleanup
    free(hunter_threads);
    huntercollection_cleanup(&house);

    for (int i = 0; i < house.room_count; i++) {
        sem_destroy(&house.rooms[i].mutex);
    }
    sem_destroy(&house.case_file.mutex);
    return 0;
}
