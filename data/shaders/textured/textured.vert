#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InPosL;			// Vertex in local coordinate system
layout (location = 1) in vec3 InColor;
layout (location = 2) in vec3 InNormalL;		// Normal in local coordinate system
layout (location = 3) in vec2 InTex;
layout (location = 4) in vec4 InTangent;

struct Instance
{
	mat4 world;
};

//! Corresponds to the C++ class Material. Stores the ambient, diffuse and specular colors for a material.
struct Material
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular; // w = SpecPower
};

struct Light
{
	// Color
	Material material;

	vec3 pos;
	float range;

	vec3 dir;
	float spot;

	vec3 att;
	float type;

	vec3 intensity;
	float id;
};

layout (std140, binding = 0) uniform UBO 
{
	// Camera 
	mat4 projection;
	mat4 view;
	

	vec4 lightDir;
	vec3 eyePos;

	float t;
	
	Light light[2];

	//vec4 test;
	//Light lights;//[10];	// Max 10 lights
} per_frame;

layout(push_constant) uniform PushConsts {
	 mat4 world;	// Model View Projection
	 vec3 color;	// Color
} pushConsts;

layout (location = 0) out vec3 OutNormalW;		// Normal in world coordinate system
layout (location = 1) out vec3 OutColor;
layout (location = 2) out vec2 OutTex;
layout (location = 3) out vec3 OutEyeDirW;		// Direction to the eye in world coordinate system
layout (location = 4) out vec3 OutLightDirW;

//
// w/ Instancing
//
void main() 
{
	OutColor = per_frame.light[1].att;//pushConsts.color; //InColor;
	OutTex = InTex;

	mat4 world = pushConsts.world;
	//mat4 world = ubo.instance[gl_InstanceIndex].world;
	gl_Position = per_frame.projection * per_frame.view * world * vec4(InPosL.xyz, 1.0);
	
    vec4 PosW = pushConsts.world  * vec4(InPosL, 1.0);
    OutNormalW = mat3(pushConsts.world ) * InNormalL;
  //  OutLightDirW = per_frame.lights.dir; //per_frame.lightDir.xyz;
    OutEyeDirW = per_frame.eyePos - PosW.xyz;		
}
//
// w/o Instancing
//
//void main() 
//{
//	OutColor = pushConsts.color; //InColor;
//	OutTex = InTex;
//	gl_Position = ubo.projection * ubo.view * pushConsts.world * vec4(InPosL.xyz, 1.0);
	
//    vec4 PosW = pushConsts.world  * vec4(InPosL, 1.0);
//    OutNormalW = mat3(pushConsts.world ) * InNormalL;
//    OutLightDirW = ubo.lightDir.xyz;
//    OutEyeDirW = ubo.eyePos - PosW.xyz;		
//}