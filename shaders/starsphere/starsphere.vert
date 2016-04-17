#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;

layout (std140, binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 view;
	vec4 lightDir;
	vec3 eyePos;
} ubo;

layout (std140, push_constant) uniform PushConsts 
{
	mat4 mvp;
} pushConsts;

layout (location = 0) out vec3 outUVW;

void main() 
{
	outUVW = inPos;

	// Remove the translation component
	mat4 fixedView = ubo.view;
	fixedView[3] = vec4(0, 0, 0, 1);

	gl_Position = ubo.projection * fixedView * pushConsts.mvp * vec4(inPos.xyz, 1.0);
}
