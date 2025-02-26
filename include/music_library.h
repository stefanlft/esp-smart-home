#ifndef MUSIC_LIBRARY_H
#define MUSIC_LIBRARY_H

#include "piezo_wrapper.h"
#include "musical_notes.h"

struct piezo_note_t power_on_song[] = {
    {BREAK, 100},
    {FREQ_C3, 150},    // C4 for 200ms
    {FREQ_E3, 100},    // E4 for 100ms
    {FREQ_G3, 100},    // G4 for 200ms
    {FREQ_D3, 200},    // D4 for 400ms
    {FREQ_B5, 150},
};

struct piezo_note_t blink_song[] = {
    {BREAK, 100},
    {FREQ_B2, 200},
    {BREAK, 300},
    {FREQ_B7, 200},
};


#endif