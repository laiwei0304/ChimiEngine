#version 450

layout(push_constant) uniform CameraData
{
    mat4 objectToClip;
    vec4 lightDirectionAndIntensity;
    vec4 lightColorAndAmbient;
} cameraData;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragObjectNormal;
layout(location = 0) out vec4 outColor;

void main()
{
    vec3 lightDirection = normalize(-cameraData.lightDirectionAndIntensity.xyz);
    vec3 normal = normalize(fragObjectNormal);
    float lightIntensity = cameraData.lightDirectionAndIntensity.w;
    float ambient = cameraData.lightColorAndAmbient.w;
    float lambert = max(dot(normal, lightDirection), 0.0);

    vec3 litColor =
        fragColor *
        cameraData.lightColorAndAmbient.rgb *
        (ambient + lambert * lightIntensity);
    outColor = vec4(litColor, 1.0);
}
