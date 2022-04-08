#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec2 TexCoords;
out vec3 TdirLdirection;
out vec3 TspotLposition;
out vec3 TspotLdirection;
out vec3 TpointLposition1;
out vec3 TpointLposition2;
out vec3 TViewPos;
out vec3 TFragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 viewPos;
uniform vec3 dirLdirection;
uniform vec3 spotLposition;
uniform vec3 spotLdirection;
uniform vec3 pointLposition1;
uniform vec3 pointLposition2;

void main()
{
    vec3 FragPos = vec3(model * vec4(aPos, 1.0));
    TexCoords = aTexCoords;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    mat3 TBN = transpose(mat3(T, B, N));
    TdirLdirection = TBN * dirLdirection;
    TspotLposition = TBN * spotLposition;
    TspotLdirection = TBN * spotLdirection;
    TpointLposition1 = TBN * pointLposition1;
    TpointLposition2 = TBN * pointLposition2;
    TViewPos = TBN * viewPos;
    TFragPos = TBN * FragPos;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}