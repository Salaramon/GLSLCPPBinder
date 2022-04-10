#version 450

// multiple render targets, gbuffer
layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gColor;

in VS_OUT {
    vec3 fragPosition;
    vec3 fragNormal;
    vec2 TexCoords;

    mat3 tbnMat;
} fs_in;

struct ModelUnitData {
    int xDelta; 
    int yDelta;
    int wDelta;
    int hDelta;
    int layerDelta;
};


layout(shared, binding = 1) uniform ModelUnitTables {
    uniform ModelUnitData unitTable[6];
};


uniform sampler2DArray textureLib;
uniform int modelNumber;

const int diffuse_unit_index = 0;
const int specular_unit_index = 1;
const int normals_unit_index = 2;

vec3 getTexUnitCoords(vec3 texCoords, int modelNumber, int unitIndex) {
    return vec3(texCoords.x + float(unitTable[modelNumber * 3 + unitIndex].xDelta) / textureSize(textureLib,0).x, 
                texCoords.y + float(unitTable[modelNumber * 3 + unitIndex].yDelta) / textureSize(textureLib,0).y,
                texCoords.z + unitTable[modelNumber * 3 + unitIndex].layerDelta); 
}

void main() {
    vec3 diffuseCoords = vec3(fs_in.TexCoords, 0);
    vec3 specularCoords = getTexUnitCoords(vec3(fs_in.TexCoords, 0), modelNumber, specular_unit_index);
    vec3 normalCoords = getTexUnitCoords(vec3(fs_in.TexCoords, 0), modelNumber, normals_unit_index);
    
    gPosition = fs_in.fragPosition;

    vec2 normalsDelta = vec2(unitTable[modelNumber * 3 + normals_unit_index].xDelta, unitTable[modelNumber * 3 + normals_unit_index].yDelta);
    if (normalsDelta != vec2(0.0)) // normal map exists ?
        gNormal = normalize(fs_in.tbnMat * vec3(texture(textureLib, normalCoords).rgb * 2.0 - 1.0));
    else
        gNormal = fs_in.tbnMat[2];

    gColor = vec4(texture(textureLib, diffuseCoords).rgb, texture(textureLib, specularCoords).r);
    //gColor = vec4(gNormal, texture(textureLib, specularCoords).r);

    /*
    x y w h l
    0 0 h h 0 h 0 0
    0 0 0 h 0 0 0 h
    h l l 0 0 0 0 0
    1 0 0 0 0 1 

    x y w h l
    0 0 h h 0
    h 0 0 0 0
    0 h 0 0 0
    h h l l 0
    0 0 0 0 1
    0 0 0 0 1
    */



    //gColor = vec4(texture(textureLib, normalCoords).rgb, float(unitTable[0].xDelta) / (8192.0));
}
