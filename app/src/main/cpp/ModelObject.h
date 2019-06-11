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

#ifndef ASSIMPLOADER_H
#define ASSIMPLOADER_H

#include <map>
#include <vector>
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "myGLM.h"
#include "myGLCamera.h"
#include "myGLFunctions.h"
#include "Animator.h"
#include "AnimationLoader.h"

using namespace std;

// info used to render a mesh
struct MeshInfo {
    string nodeName;
    GLuint  textureIndex;
    int     numberOfFaces;
    GLuint  faceBuffer;
    GLuint  vertexBuffer;
    GLuint  textureCoordBuffer;
    glm::mat4 baseModelMatrix;
};

class ModelObject {

public:
    ModelObject();
    ~ModelObject();

    void Render3DModel(MyGLCamera * cam);
    bool Load3DModel(std::string modelFilename);
    void Delete3DModel();

private:
    void ProcessNode(aiNode *node);
    void ProcessMesh(aiNode* node, aiMesh *mesh);
    bool LoadTexturesToGL(std::string modelFilename);

private:
    std::vector<struct MeshInfo> modelMeshes;       // contains one struct for every mesh in model
    Assimp::Importer* _importerPtr;
    const aiScene* _aiScene;                           // assimp's output data structure
    bool isObjectLoaded;

    std::map<std::string, GLuint> textureNameMap;   // (texture filename, texture name in GL)

    GLuint  vertexAttribute, vertexUVAttribute;     // attributes for shader variables
    GLuint  shaderProgramID;
    GLint   mvpLocation, textureSamplerLocation;    // location of MVP in the shader

    AnimationLoader* _animationLoader;
    Animator* _animator;
};

#endif //ASSIMPLOADER_H
