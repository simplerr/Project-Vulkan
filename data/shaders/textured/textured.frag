#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 1) uniform sampler2D samplerColorMap;

layout (location = 0) in vec3 InNormalW;
layout (location = 1) in vec3 InColor;
layout (location = 2) in vec2 InTex;
layout (location = 3) in vec3 InEyeDirW;
layout (location = 4) in vec3 InLightDirW;

layout (location = 0) out vec4 OutFragColor;

void main() 
{
	// Ambient factor
	vec3 color = vec3(0.2f);
	vec3 Color = vec3(1, 1, 1);	

	vec3 normal = normalize(InNormalW);
	vec3 lightDir = normalize(InLightDirW);
	vec3 V = normalize(InEyeDirW);
	vec3 R = reflect(-lightDir, normal);

	// Diffuse
	float shade = clamp(dot(normal, lightDir), 0.0f, 1.0f);
	vec3 diffuse = shade * Color;
	color += diffuse;

	// Specular
	shade = pow(max(dot(R, V), 0.0), 512.0);
	vec3 specular = shade * Color;
	color += specular;	

	OutFragColor = texture(samplerColorMap, InTex) * vec4(color, 1.0f);
}