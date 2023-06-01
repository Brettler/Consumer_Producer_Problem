# include "manager.h"

void* managerThread(void* arg) {
    Manager* manager = (Manager*)arg;
    char* message;
    do {
        message = removeBoundedBuffer(manager -> SharedBuffer);
        if (strcmp(message, "DONE") == 0) {
            manager -> doneCount += 1; // Decrement doneCount for each "DONE" message
            if (manager -> doneCount == 3) {
                // Exit the loop when all "DONE" messages have been received
                break;
            }
            continue;
        }
        printf("%s\n", message);
        // Free the message after processing (Free it only if != DONE. we send DONE as a literal string.
        free(message);
    } while(1);
    // We will add '/n' because in the moodle it allows.
    printf("DONE\n");
    return NULL;
}