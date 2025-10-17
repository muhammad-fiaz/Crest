/**
 * @file mutex_wrapper.cpp
 * @brief C++ wrapper for mutex operations
 */

#include <mutex>

extern "C" {

void* crest_mutex_create() {
    return new std::mutex();
}

void crest_mutex_destroy(void* mutex) {
    if (mutex) {
        delete static_cast<std::mutex*>(mutex);
    }
}

}
