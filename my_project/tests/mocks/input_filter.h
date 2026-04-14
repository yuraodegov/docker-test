#pragma once
#include <stdint.h>
typedef struct { uint8_t value; uint8_t count; } InputFilter;
static inline void initializeInputFilter(InputFilter* f, uint8_t v) { f->value=1; f->count=0; }
static inline void updateInputFilter(InputFilter* f, uint8_t v)     { f->value=v; }
static inline uint8_t getInputFilter(InputFilter* f)                { return f->value; }
