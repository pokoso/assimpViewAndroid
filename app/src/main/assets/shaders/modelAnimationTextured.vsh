/*
 *    Copyright 2016 Anand Muralidhar
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

// shader associated with AssimpLoader

attribute   vec3 vertexPosition;
attribute   vec2 vertexUV;
attribute   vec4 weight;
attribute   vec4 boneID;

varying     vec2 textureCoords;

uniform     mat4 mvpMat;
uniform mat4 boneTransformation[255];

void main()
{
    int b1 = int(boneID.x);
    int b2 = int(boneID.y);
    int b3 = int(boneID.z);
    int b4 = int(boneID.w);

    mat4 bTrans = boneTransformation[b1] * weight.x;
    if (b2 != -1) {bTrans += boneTransformation[b2] * weight.y;}
    if (b3 != -1) {bTrans += boneTransformation[b3] * weight.z;}
    if (b4 != -1) {bTrans += boneTransformation[b4] * weight.w;}

    gl_Position     = mvpMat * bTrans * vec4(vertexPosition, 1.0);
    textureCoords   = vertexUV;
}
