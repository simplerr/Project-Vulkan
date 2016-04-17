#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InPosL;			// Vertex in local coordinate system
layout (location = 1) in vec3 InColor;
layout (location = 2) in vec3 InNormalL;		// Normal in local coordinate system
layout (location = 3) in vec2 InTex;
layout (location = 4) in vec4 InTangent;

layout (std140, binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 view;
	vec4 lightDir;
	vec3 eyePos;
} ubo;

layout(push_constant) uniform PushConsts {
	 mat4 world;	// Model View Projection
	 vec3 color;	// Color
} pushConsts;

layout (location = 0) out vec3 OutNormalW;		// Normal in world coordinate system
layout (location = 1) out vec3 OutColor;
layout (location = 2) out vec2 OutTex;
layout (location = 3) out vec3 OutEyeDirW;		// Direction to the eye in world coordinate system
layout (location = 4) out vec3 OutLightDirW;

// Push constants
void main() 
{
	OutColor = pushConsts.color; //InColor;
	OutTex = InTex;
	gl_Position = ubo.projection * ubo.view * pushConsts.world * vec4(InPosL.xyz, 1.0);
	
    vec4 PosW = pushConsts.world  * vec4(InPosL, 1.0);
    OutNormalW = mat3(pushConsts.world ) * InNormalL;
    OutLightDirW = ubo.lightDir.xyz;
    OutEyeDirW = ubo.eyePos - PosW.xyz;		
}

// Default = uniform buffers
// With push constants this dont work
//void main() 
//{
//	outNormal = inNormal;
//	outColor = inColor;
//	outUV = inUV;
//	gl_Position = ubo.projection * ubo.model * vec4(inPos.xyz, 1.0);
//	//gl_Position = pushConstantsBlock.MVP * vec4(inPos.xyz, 1.0);
	
//    vec4 pos = ubo.projection * ubo.model  * vec4(inPos, 1.0);
//    outNormal = mat3(ubo.projection * ubo.model ) * inNormal;
//	//vec3 lPos = mat3(ubo.model) * ubo.lightPos.xyz;
//	vec3 lPos = vec3(0.0);
//    outLightVec = lPos - pos.xyz;
//    outViewVec = -pos.xyz;		
//}