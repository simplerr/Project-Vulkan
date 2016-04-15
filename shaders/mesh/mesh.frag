#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 1) uniform sampler2D samplerColorMap;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inViewVec;
layout (location = 4) in vec3 inLightVec;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	// Ambient factor
	vec3 color = vec3(0.2f);	

	vec3 normal = normalize(inNormal);
	vec3 lightDir = normalize(inLightVec);
	vec3 V = normalize(inViewVec);
	vec3 R = reflect(-lightDir, normal);

	// Diffuse
	float shade = clamp(dot(normal, lightDir), 0.0f, 1.0f);
	vec3 diffuse = shade * inColor;
	color += diffuse;

	// Specular
	shade = pow(max(dot(R, V), 0.0), 16.0);
	vec3 specular = shade * inColor;
	color += specular;	

	outFragColor = texture(samplerColorMap, inUV) * vec4(color, 1.0f);
}