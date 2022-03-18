#version 430

#include "../fragments/fs_common_inputs.glsl"

// We output a single color to the color buffer
layout(location = 0) out vec4 frag_color;

////////////////////////////////////////////////////////////////
/////////////// Instance Level Uniforms ////////////////////////
////////////////////////////////////////////////////////////////

// Represents a collection of attributes that would define a material
// For instance, you can think of this like material settings in 
// Unity
struct Material {
	sampler2D Diffuse;
	float     Shininess;
	int		  Toggle;
	bool	  ToggleOn;
};
// Create a uniform for the material
uniform Material u_Material;

// Create a uniform for 1D LUT for ramp
uniform sampler1D s_ToonTerm;

////////////////////////////////////////////////////////////////
///////////// Application Level Uniforms ///////////////////////
////////////////////////////////////////////////////////////////

#include "../fragments/multiple_point_lights.glsl"

////////////////////////////////////////////////////////////////
/////////////// Frame Level Uniforms ///////////////////////////
////////////////////////////////////////////////////////////////

#include "../fragments/frame_uniforms.glsl"
#include "../fragments/color_correction.glsl"

// https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
void main() {
	// Normalize our input normal
	vec3 normal = normalize(inNormal);
	vec3 lightAccumulation;
	vec3 result;
	vec3 V = normalize(u_CamPos.xyz - inWorldPos);
	float red = texture(s_ToonTerm, result.r).r;

	lightAccumulation = CalcAllLightContribution(inWorldPos, normal, u_CamPos.xyz, u_Material.Shininess);


	// Get the albedo from the diffuse / albedo map
	vec4 textureColor = texture(u_Material.Diffuse, inUV);

	if (u_Material.Toggle == 1) { //ambient only
		result = inColor * textureColor.rgb;
	}

	if (u_Material.Toggle == 2) { //specular only
		lightAccumulation = CalcSpecOnly(inWorldPos, normal, u_CamPos.xyz, u_Material.Shininess);
		result = lightAccumulation;
	}

	if (u_Material.Toggle == 3) { //ambient + specular
		// combine for the final result
		result = lightAccumulation  * inColor * textureColor.rgb;
	}

	if (u_Material.Toggle == 4) { //toon shading 
		lightAccumulation = CalcToonShadingOnly(inWorldPos, normal, u_CamPos.xyz, u_Material.Shininess);
		result = lightAccumulation * inColor * textureColor.rgb;
	}

	if (u_Material.Toggle == 5) { //Diffuse warp/ramp
		//lightAccumulation = CalcRampDiffuseOnly(inWorldPos, normal, u_CamPos.xyz, u_Material.Shininess, red);
		//result = lightAccumulation * inColor * textureColor.rgb;
		lightAccumulation = CalcAllLightContribution(inWorldPos, normal, u_CamPos.xyz, u_Material.Shininess);
		result = lightAccumulation * inColor * textureColor.rgb;
		if (u_Material.ToggleOn) {
			result.r = texture(s_ToonTerm, result.r).r;
			result.g = texture(s_ToonTerm, result.g).g;
			result.b = texture(s_ToonTerm, result.b).b;
		}
	}

	if (u_Material.Toggle == 6) { //Specular warp/ramp
		if (u_Material.ToggleOn)
			lightAccumulation = CalcAllLightContribution(inWorldPos, normal, u_CamPos.xyz, texture(s_ToonTerm, inColor.r).r);
		else 
			lightAccumulation = CalcAllLightContribution(inWorldPos, normal, u_CamPos.xyz, u_Material.Shininess);
		result = lightAccumulation * inColor * textureColor.rgb;	
	}

	frag_color = vec4(ColorCorrect(result), textureColor.a);
}

