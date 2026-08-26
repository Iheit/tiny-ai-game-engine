#include "lighting.hpp"
// Lighting is header-defined so the renderer can inline small shading operations.
// This translation unit keeps the module independently buildable and leaves room for
// future clustered/shadowed lighting without changing the public include path.
