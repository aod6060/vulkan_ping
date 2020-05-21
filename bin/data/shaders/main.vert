#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformVS
{
	mat4 proj;
	mat4 view;
	mat4 model;
} ubo;

layout(location = 0) in vec3 vertices;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(vertices, 1.0);
}