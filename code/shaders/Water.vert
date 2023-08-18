#version 330

layout(location = 0) in vec3 Position;
//layout(location = 1) in vec2 TexCoord;

//out vec2 TexCoord0;
out vec4 ClipSpace;

uniform mat4 MatProj;  // матрица проекции
uniform mat4 MatView;  // матрица вида
uniform mat4 MatModel; // матрица модели

void main(void)
{
    //ClipSpace = MatProj * MatView * MatModel * vec4(Position, 1.0);
    ClipSpace = MatProj * MatView * MatModel * vec4(Position.x, Position.y, 0.0, 1.0);
    gl_Position = ClipSpace;
    //TexCoord0 = vec2(Position.x / 2.0 + 0.5, Position.y / 2.0 + 0.5);
}