#version 330 core
out vec4 FragColor;
in vec3 pos;

void main()
{
    float ratio = 2.0 * pos.z / 100.0;
    float b = float(max(0.0, (1.0 - ratio)));
    float r = float(max(0.0, (ratio - 1.0)));
    float g = 1.0 - b - r;

    float w = pos.z/100.0;
    if(gl_FrontFacing)
        FragColor = vec4(r,g,b, 1.0f);
    else
        FragColor = vec4(r,g,b, 0.0f);
} 