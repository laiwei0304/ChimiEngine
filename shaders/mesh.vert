#version 450

layout(push_constant) uniform CameraData
{
    mat4 objectToClip;
} cameraData;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main()
{
    gl_Position = cameraData.objectToClip * vec4(inPosition, 1.0);
    fragColor = inColor;
}
