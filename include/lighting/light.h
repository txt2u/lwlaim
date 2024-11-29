#include <pipeline/shader.h>
#include <cglm/cglm.h>
#include <stdbool.h>

typedef enum {
    LIGHT_TYPE_POINT = 0,
    LIGHT_TYPE_SPOT = 1,
    LIGHT_TYPE_DIRECTIONAL = 2
} LightType;

// Modify Light structure to optionally store camera direction for spotlights (camera direction will be used for light computations)
typedef struct {
    LightType type;
	
    vec3 position;   // Position of light
    vec3 color;      // Color of light
    float intensity; // Intensity of light

	int shader_program;
} Light;

void create_light(Light* light, int shader_program);
void create_point_light(Light* light, vec3 position, vec3 color, float intensity);
// void create_directional_light(Light* light, vec3 direction, vec3 color, float intensity);
// void create_spot_light(Light* light, vec3 position, vec3 direction, vec3 color, float intensity, float cutOff, float outerCutOff);