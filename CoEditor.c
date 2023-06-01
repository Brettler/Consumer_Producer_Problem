# include "CoEditor.h"

void* coEditorThread(void* arg) {
    CoEditor* args = (CoEditor*)arg;
    UnboundedBuffer* dispatcherBuffer = args -> dispatcherBuffer;
    BoundedBuffer* SharedBuffer = args -> SharedBuffer;

    char* message;
    do {
        message = removeUnboundedBuffer(dispatcherBuffer);
        if (strcmp(message, "DONE") == 0) {
            insertBoundedBuffer(SharedBuffer, "DONE"); // Forward the "DONE" message
        } else {
            // Simulate editing by waiting for 0.1 seconds
            usleep(100000);
            insertBoundedBuffer(SharedBuffer, message); // Forward the message to the manager
        }
    } while(strcmp(message, "DONE") != 0);

    return NULL;
}
