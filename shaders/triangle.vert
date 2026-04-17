#version 450

layout(push_constant) uniform CameraData
{
    mat4 viewProjection;
} cameraData;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main()
{
    gl_Position = cameraData.viewProjection * vec4(inPosition, 1.0);
    fragColor = inColor;
}
