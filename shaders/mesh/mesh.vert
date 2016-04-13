#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec2 inUV;
layout (location = 4) in vec4 inTangent;

// After adding the push constans this uniform buffer don't see to work
layout (binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 model;
	vec4 lightPos;
} ubo;

layout(std140, push_constant) uniform pushBlock
{
    mat4 MVP;	// Model View Projection
} pushConstantsBlock;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec2 outUV;
layout (location = 3) out vec3 outViewVec;
layout (location = 4) out vec3 outLightVec;

void main() 
{
	outNormal = inNormal;
	outColor = inColor;
	outUV = inUV;
	//gl_Position = ubo.projection * ubo.model * vec4(inPos.xyz, 1.0);
	gl_Position = pushConstantsBlock.MVP * vec4(inPos.xyz, 1.0);
	
    vec4 pos = pushConstantsBlock.MVP * vec4(inPos, 1.0);
    outNormal = mat3(pushConstantsBlock.MVP) * inNormal;
	//vec3 lPos = mat3(ubo.model) * ubo.lightPos.xyz;
	vec3 lPos = vec3(0.0);
    outLightVec = lPos - pos.xyz;
    outViewVec = -pos.xyz;		
}