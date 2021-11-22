#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 fragColor;

layout(location = 0) out vec4 outColor;


struct complex
{
    float real;
    float imaginary;
};


float u_min = -2.5;
float u_max = 1;
float v_min = -1.5;
float v_max = 1.5;

// Uvijek isto za Vulkan.
float x_min = -1.0;
float x_max = 1.0;
float y_min = -1.0;
float y_max = 1.0;

complex get_complex_pos(vec2 position)
{
    // Dodao sam [(max + min) / 2] radi centriranja.
    return complex
    (
        (u_max + u_min) / 2 + position.x * (u_max - u_min) * (0.5), // 1 / (x_max - x_min)
        
        // Obrnuto jer Y ide prema dolje po Vulkan-u.
        (v_max + v_min) / 2 + position.y * (v_max - v_min) * (-0.5) // 1 / (y_min - y_max)  
    );
}


complex add(complex z1, complex z2)
{
    return complex
    (
        z1.real + z2.real,
        z1.imaginary + z2.imaginary
    );
}

complex multiply(complex z1, complex z2)
{
    return complex
    (
        z1.real * z2.real - z1.imaginary * z2.imaginary,
        z1.real * z2.imaginary + z1.imaginary * z2.real
    );
}

float size(complex z)
{
    return sqrt(z.real * z.real + z.imaginary * z.imaginary);
}


int iteration_limit = 100;
float epsilon = 10000;

bool is_converging(complex z)
{
    return size(z) < epsilon;
}

complex z_start = complex(0, 0);

int iterate (complex z, complex c)
{
    int k = -1;
    
    do
    {
        ++k;
        z = add(multiply(z, z), c);
    } 
    while(k < iteration_limit && is_converging(z));
    
    return k;
}


vec4 iteration_limit_color = vec4(0.0, 0.0, 0.0, 1.0);
vec4 start_color = vec4(1.0, 0.6, 0.0, 1.0);
vec4 mid_color = vec4(0.5, 0.0, 0.0, 1.0);
vec4 end_color = vec4(0.0, 0.0, 1.0, 1.0);
float shade_fall_gradient = 0.5;
float shade_rise_gradient = 1.5;


vec4 get_color_for_divergent_iteration_level(int k)
{
    if (k == iteration_limit)
    {
        return vec4(0.0, 0.0, 0.0, 1.0);
    }

    float shade = k / float(iteration_limit);
    float shade_fall = pow(shade, shade_fall_gradient);
    float shade_mid = sin(3.14 * shade);
    float shade_rise = pow(shade, shade_rise_gradient);

    return vec4
    (
        start_color.r * shade_rise + mid_color.r * shade_mid + end_color.r * shade_fall, 
        start_color.g * shade_rise + mid_color.g * shade_mid + end_color.g * shade_fall,         
        start_color.b * shade_rise + mid_color.b * shade_mid + end_color.b * shade_fall,
        1.0
    );
}


void main() 
{
    complex pos = get_complex_pos(inPosition);
    
    int divergent_iteration_level = iterate(z_start, pos);
    outColor = get_color_for_divergent_iteration_level(divergent_iteration_level);
}

