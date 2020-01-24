#version 330 core
out vec4 FragColor;
in vec3 pos;

float near = 0.1; 
float far  = 300.0; 
  
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{
    float ratio = 2.0 * pos.z / 100.0;
    float b = float(max(0.0, (1.0 - ratio)));
    float r = float(max(0.0, (ratio - 1.0)));
    float g = 1.0 - b - r;

    float w = pos.z/100.0;
    float depth = LinearizeDepth(gl_FragCoord.z) / far;
    FragColor = vec4(vec3(w), 1.0);
} 