#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InPosL;			// Vertex in local coordinate system
layout (location = 1) in vec3 InColor;
layout (location = 2) in vec3 InNormalL;		// Normal in local coordinate system
layout (location = 3) in vec2 InTex;

layout (location = 4) in mat4 InstanceMatrix;

uniform mat4 gWorld;
uniform mat4 gWorldInvTranspose;
uniform mat4 gView;
uniform mat4 gProjection;
uniform vec3 gEyePos;
uniform vec3 gLightDir;
uniform bool gUseInstancing;

layout (location = 0) out vec3 OutNormalW;		// Normal in world coordinate system
layout (location = 1) out vec3 OutColor;
layout (location = 2) out vec2 OutTex;
layout (location = 3) out vec3 OutEyeDirW;		// Direction to the eye in world coordinate system
layout (location = 4) out vec3 OutLightDirW;

// Push constants
void main() 
{
	OutColor = vec3(1, 0, 0);
	OutTex = InTex;

	mat4 world;

	if(gUseInstancing)
		world = InstanceMatrix;
	else if(!gUseInstancing)
		world = gWorld;

	gl_Position = gProjection * gView * world * vec4(InPosL.xyz, 1.0);
	
    vec4 PosW = world * vec4(InPosL, 1.0);
    OutNormalW = mat3(world) * InNormalL;
    OutLightDirW = gLightDir.xyz;
    OutEyeDirW = gEyePos - PosW.xyz;
}