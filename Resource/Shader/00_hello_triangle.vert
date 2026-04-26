#version 450

out gl_PerVertex{
    vec4 gl_Position;
};

vec3 vertices[] = {
    vec3(0.0, 0.0, 0.0),
    vec3(0.0, 0.5, 0.0),
    vec3(-0.5, 0.0, 0.0)
};

vec4 colors[] = {
    vec4(1.0, 0.0, 0.0, 1.0),
    vec4(0.0, 1.0, 0.0, 1.0),
    vec4(0.0, 0.0, 1.0, 1.0)
};

layout(location = 0) out vec4 vertexColor;

void main(){
    vec3 inPos = vertices[gl_VertexIndex];
    gl_Position = vec4(inPos.x, -inPos.y, inPos.z, 1.0);
    vertexColor = colors[gl_VertexIndex];
}
