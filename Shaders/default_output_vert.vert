#version 450
#pragma shader_stage(vertex)

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 txcoord;
layout (location = 0) out vec2 fragCoord;

layout (set = 1, binding = 0) uniform Window {
	vec2 screenSize;
} windowSize;

void main() {
	mat4 ortho = mat4(
		2.0/windowSize.screenSize.x, 0.0, 0.0, 0.0,
		0.0,-2.0/windowSize.screenSize.y, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		-1.0, 1.0, 0.0, 1.0
	);
	
	gl_Position = ortho * vec4(position, 1.0);
    fragCoord = txcoord;
}