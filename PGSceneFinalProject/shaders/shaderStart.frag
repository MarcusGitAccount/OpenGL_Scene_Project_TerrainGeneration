#version 410 core

in vec3 normal;
in vec4 fragPosEye;
in vec2 texCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;

// texturing and shadow computation
uniform sampler2D diffuseTexture; uniform sampler2D shadowMap;
uniform	mat3 normalMatrix;

// lighting
uniform mat3 lightDirMatrix;
uniform	vec3 lightDir;
uniform	vec3 lightColor;

uniform float source1Weight;
uniform float source2Weight;

uniform vec3 cameraPosLight;
uniform vec3 cameraDirLight;

float ambientStrength = 0.2f;
float specularStrength = 0.5f;
float shininess = 32.0f;

float computeShadow()
{	
	// perform perspective divide
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    if(normalizedCoords.z > 1.0f) {
        return 0.0f;
	}

    // Transform to [0,1] range
    normalizedCoords = normalizedCoords * 0.5f + 0.5f;
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;    
    // Get depth of current fragment from light's perspective
    float currentDepth = normalizedCoords.z;
    // Check whether current frag pos is in shadow
    float bias = 0.005f;
    float shadow = currentDepth - bias> closestDepth  ? 1.0f : 0.0f;

    return shadow;	
}

vec3 computeLightComponents(vec3 cameraPosEye, vec3 lightDirection, vec3 lightCol)
{		
	//vec3 cameraPosEye = vec3(0.f, 4.f, 0.f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(normalMatrix * normal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDirection);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fragPosEye.xyz);
		
	//compute ambient light
	vec3 ambient = ambientStrength * lightCol;
	
	//compute diffuse light
	vec3 diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightCol;
	
	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	vec3 specular = specularStrength * specCoeff * lightCol;

	//vec3 component = (ambient + diffuse + specular);

	float shadow = computeShadow();
	vec3 component = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);

	return component;
}

vec3 pointingLight(vec3 lightPos, vec3 cameraPosEye, vec3 lightCol)
{		
	//vec3 cameraPosEye = vec3(0.f, 4.f, 0.f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(normalMatrix * normal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightPos - fragPosEye.xyz);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fragPosEye.xyz);
		
	//compute ambient light
	vec3 ambient = ambientStrength * lightCol;
	
	//compute diffuse light
	vec3 diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightCol;
	
	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	vec3 specular = specularStrength * specCoeff * lightCol;

	float distance = length(lightPos - fragPosEye.xyz);
	float attenuation = 1.f / (distance  * distance * distance);

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

void main() 
{
	//computeLightComponents();
	
	//vec3 baseColor = vec3(1.0f, 0.55f, 0.0f);//orange
	
	//ambient *= baseColor;
	//diffuse *= baseColor;
	//specular *= baseColor;

	vec4 baseColor = texture(diffuseTexture, texCoords);
	//vec3 color = min((ambient + diffuse) + specular, 1.0f);

	// base light
	vec3 result = source1Weight * computeLightComponents(vec3(0.f), lightDir, lightColor) * baseColor.xyz;

	// camera light
	result = result + source2Weight * computeLightComponents(cameraPosLight, cameraDirLight, vec3(.65f)) * baseColor.xyz;
	//result = result + source2Weight * computeLightComponents(vec3(2.649141f, 3.221051f, 6.835550f), vec3(0.f), vec3(.5f)) * baseColor.xyz;
    //fColor = texture(diffuseTexture, texCoords);
    fColor = vec4(result, 1.0f);
    //fColor = vec4(vec3(1.f, 0.54f, 0.f), 1.0f);
}
