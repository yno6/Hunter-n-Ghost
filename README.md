# Yashvi Nayak 
        Final Project _ COMP 2401 A

# Description: 
        This project is a multi-threaded ghost-hunting simulation where hunters and ghosts move between rooms inside a house, interact indirectly through shared evidence, and hunters try to identify the ghost. The house is made up of many connected rooms (13 rooms for this simulation). Each ghost has a specific ghost type and three particular (unique combination of) evidences that point to this unique ghost type. Each hunter carries one device (at once) and tries to collect matching evidence to identify the ghost.

# The simulation ends when: 
        - Hunters gather enough evidence to identify the ghost, or
        - all hunters get afraid or bored and exit, or
        - The ghost gets bored and exits

# Purpose of all files: 
        main.c   ->   Coordinates the overall program flow by initializing the house, creating all hunter and ghost threads, 
                      starting the simulation, waiting for all threads to finish, and display the results.

        room.c   ->   Implements all room-related functionality, including initialization, connections, evidence tracking, and 
                      maintaining which hunters or ghosts are inside each room.
        room.h   ->   declares/defines the functions for room operations (implemented in room.c), and so other modules (hunters,   
                      ghosts, main) can safely access room-related functions.

        hunter.c ->   Implements hunter behavior, including movement, evidence collection, device swapping, exiting, and the  
                      full hunter thread loop.
        hunter.h ->   Declares/defines functions for creating, managing, and running hunters, showing only what other modules need 
                      to interact with hunters.

        ghost.c  ->   Contains the logic for ghost behavior, including movement, leaving evidence, boredom tracking, and the 
                      full ghost thread loop.
        ghost.h  ->   Declares/defines the ghost interface functions so the ghost.c, main program and other modules can initialize 
                      and run the ghost thread.

        helpers.c ->  Provides helper functions for converting enums to strings, generating random values, validating evidence 
                      masks, logging actions, and populating the house layout.
        helpers.h ->  Defines the helper utility functions, implemented in helpers.c, and required by ghosts, hunters, & rooms for 
                      further work.

## Compiling & Running: To compile and run this program via the command line, 
                        
                        1. Ensure all source files (hunter.c, ghost.c, room.c, main.c, & defs.h) and other supporting files are in the same                             directory (as well as the makefile). 
                        
                        2. To compile using makefile, just type the following: 
                                make   (this will compile only the modified .c files and produce the executable ghost_sim)
                                OR
                                make tsan (to compile with ThreadSanitizer to check race conditions)
                        
                        3. Run the program using --> ./name_of_the_executable_file 
                            e.g.    ./ghost_sim  (in this case)
                            e.g.    valgrind --leak-check=full ./ghost_sim  (to use with valgrind for memory leak check)
                        
                        4. Now you should be able to see the printed info, and user input prompt to further run the program  
                           after giving apropriate user inputs. 

## Sources: 
- COMP 2401 Intro to Systems Programming Course @ Carelton University
