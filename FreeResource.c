#include "FreeResource.h"
void FreeResources(Config *config, CoEditor **ArrayCoEditors, pthread_t *coEditorThreads, Manager *manager,
                   BoundedBuffer *SharedBuffer, UnboundedBuffer **DispatcherBuffersArray, Dispatcher *dispatcher) {
    // Free ArrayCoEditors
    for (int i = 0; i < NUM_ARTICLES_TYPES; ++i) {
        free(ArrayCoEditors[i]);
    }
    free(ArrayCoEditors);

    // Free coEditorThreads, manager, SharedBuffer
    free(coEditorThreads);
    free(manager);
    destructorBoundedBuffer(SharedBuffer);

    // Free DispatcherBuffersArray
    for (int i = 0; i < NUM_ARTICLES_TYPES; ++i) {
        destructorUnboundedBuffer(DispatcherBuffersArray[i]);
    }
    free(DispatcherBuffersArray);

    // Free dispatcher
    free(dispatcher);

    // Free the config
    cleanConfig(config);
}