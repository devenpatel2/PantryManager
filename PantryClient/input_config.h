#ifndef INPUT_CONFIG_H
#define INPUT_CONFIG_H

// Selects the active scroll-input source. Define exactly one.
// To fall back to the original two pushbuttons, comment out INPUT_ENCODER
// and uncomment INPUT_BUTTONS, then recompile.
#define INPUT_ENCODER
// #define INPUT_BUTTONS

#if defined(INPUT_ENCODER) == defined(INPUT_BUTTONS)
#error "Exactly one of INPUT_ENCODER or INPUT_BUTTONS must be defined"
#endif

#endif
