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

#include "ModelObject.h"
#include "myShader.h"
#include "misc.h"
#include <opencv2/opencv.hpp>
#include "Utils.h"

using namespace std::chrono;

static double GetCurrentTime() {
    return duration_cast< milliseconds >(system_clock::now().time_since_epoch()).count() * 0.001;
}

double startTime = 0;
bool gUseAnimation = true;

/**
 * Class constructor, loads shaders & gets locations of variables in them
 */
ModelObject::ModelObject() {
    _importerPtr = new Assimp::Importer;
    _aiScene = NULL;
    isObjectLoaded = false;

    // shader related setup -- loading, attribute and uniform locations

    string vertexShader;
    if(gUseAnimation)
        vertexShader = "shaders/modelAnimationTextured.vsh";
    else
        vertexShader = "shaders/modelTextured.vsh";

    std::string fragmentShader  = "shaders/modelTextured.fsh";
    shaderProgramID         = LoadShaders(vertexShader, fragmentShader);
    vertexAttribute         = GetAttributeLocation(shaderProgramID, "vertexPosition");
    vertexUVAttribute       = GetAttributeLocation(shaderProgramID, "vertexUV");
    mvpLocation             = GetUniformLocation(shaderProgramID, "mvpMat");
    textureSamplerLocation  = GetUniformLocation(shaderProgramID, "textureSampler");

    CheckGLError("AssimpLoader::AssimpLoader");

    if(gUseAnimation) {
        _animationLoader = new AnimationLoader();
        _animator = new Animator (shaderProgramID);
    }

    startTime = GetCurrentTime();
}

/**
 * Class destructor, deletes Assimp importer pointer and removes 3D model from GL
 */
ModelObject::~ModelObject() {
    Delete3DModel();
    if(_importerPtr) {
        delete _importerPtr;
        _importerPtr = NULL;
    }
    _aiScene = NULL; // gets deleted along with importerPtr
}

void ModelObject::ProcessMesh(aiNode* node, aiMesh *mesh) {
    struct MeshInfo newMeshInfo; // this struct is updated for each mesh in the model
    GLuint buffer;

    // create array with faces
    // convert from Assimp's format to array for GLES
    unsigned int *faceArray = new unsigned int[mesh->mNumFaces * 3];
    unsigned int faceIndex = 0;
    for (unsigned int t = 0; t < mesh->mNumFaces; ++t) {

        // read a face from assimp's mesh and copy it into faceArray
        const aiFace *face = &mesh->mFaces[t];
        memcpy(&faceArray[faceIndex], face->mIndices, 3 * sizeof(unsigned int));
        faceIndex += 3;

    }

    newMeshInfo.numberOfFaces = mesh->mNumFaces;
    newMeshInfo.baseModelMatrix = Utils::toMat4( &node->mTransformation );
    newMeshInfo.nodeName = node->mName.C_Str();

    MyLOGI("Node:%s, Number of total faces: %d", node->mName.C_Str(), newMeshInfo.numberOfFaces);

    // buffer for faces
    if (newMeshInfo.numberOfFaces) {

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(unsigned int) * mesh->mNumFaces * 3, faceArray,
                     GL_STATIC_DRAW);
        newMeshInfo.faceBuffer = buffer;

    }
    delete[] faceArray;

    // buffer for vertex positions
    if (mesh->HasPositions()) {

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(float) * 3 * mesh->mNumVertices, mesh->mVertices,
                     GL_STATIC_DRAW);
        newMeshInfo.vertexBuffer = buffer;

        MyLOGI("Number of total mNumVertices: %d", mesh->mNumVertices);

    }

    // buffer for vertex texture coordinates
    // ***ASSUMPTION*** -- handle only one texture for each mesh
    if (mesh->HasTextureCoords(0) && textureNameMap.size() > 0) {

        float * textureCoords = new float[2 * mesh->mNumVertices];
        for (unsigned int k = 0; k < mesh->mNumVertices; ++k) {
            textureCoords[k * 2] = mesh->mTextureCoords[0][k].x;
            textureCoords[k * 2 + 1] = mesh->mTextureCoords[0][k].y;
        }
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(float) * 2 * mesh->mNumVertices, textureCoords,
                     GL_STATIC_DRAW);
        newMeshInfo.textureCoordBuffer = buffer;
        delete[] textureCoords;

    } else {
        MyLOGI("No Texture");
    }

    // unbind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // copy texture index (= texture name in GL) for the mesh from textureNameMap
    aiMaterial *mtl = _aiScene->mMaterials[mesh->mMaterialIndex];
    aiString texturePath;	//contains filename of texture
    if (AI_SUCCESS == mtl->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath)) {
        unsigned int textureId = textureNameMap[texturePath.data];
        newMeshInfo.textureIndex = textureId;
    } else {
        newMeshInfo.textureIndex = 0;
        MyLOGI("No Texture2");
    }

    modelMeshes.push_back(newMeshInfo);
}

void ModelObject::ProcessNode(aiNode *node) {

    if (node->mNumMeshes > 0)
    {
        for (unsigned int x = 0; x < node->mNumMeshes; x++)
        {
            ProcessMesh(node, _aiScene->mMeshes[node->mMeshes[x]]);
        }
    }

    MyLOGI("Number of total meshes: %d", _aiScene->mNumMeshes);

    if (node->mNumChildren > 0)
    {
        for (unsigned int x = 0; x < node->mNumChildren; x++)
        {
            ProcessNode(node->mChildren[x]);
        }
    }
}

/**
 * Read textures associated with all materials and load images to GL
 */
bool ModelObject::LoadTexturesToGL(std::string modelFilename) {

    // read names of textures associated with all materials
    textureNameMap.clear();

    for (unsigned int m = 0; m < _aiScene->mNumMaterials; ++m) {

        int textureIndex = 0;
        aiString textureFilename;
        aiReturn isTexturePresent = _aiScene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE,
                                                                        textureIndex,
                                                                        &textureFilename);
        while (isTexturePresent == AI_SUCCESS) {
            //fill map with textures, OpenGL image ids set to 0
            textureNameMap.insert(std::pair<std::string, GLuint>(textureFilename.data, 0));

            // more textures? more than one texture could be associated with a material
            textureIndex++;
            isTexturePresent = _aiScene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE,
                                                                   textureIndex, &textureFilename);
        }
    }

    int numTextures = (int) textureNameMap.size();
    MyLOGI("Total number of textures is %d ", numTextures);

    if(numTextures == 0)
        return true;

    // create and fill array with texture names in GL
    GLuint * textureGLNames = new GLuint[numTextures];
    glGenTextures(numTextures, textureGLNames);

    // Extract the directory part from the file name
    // will be used to read the texture
    std::string modelDirectoryName = GetDirectoryName(modelFilename);

    // iterate over the textures, read them using OpenCV, load into GL
    std::map<std::string, GLuint>::iterator textureIterator = textureNameMap.begin();
    int i = 0;
    for (; textureIterator != textureNameMap.end(); ++i, ++textureIterator) {

        std::string textureFilename = (*textureIterator).first;  // get filename
        std::string textureFullPath = modelDirectoryName + "/" + textureFilename;
        (*textureIterator).second = textureGLNames[i];	  // save texture id for filename in map

        // load the texture using OpenCV
        MyLOGI("Loading texture %s", textureFullPath.c_str());
        cv::Mat textureImage = cv::imread(textureFullPath);
        if (!textureImage.empty()) {

            // opencv reads textures in BGR format, change to RGB for GL
            cv::cvtColor(textureImage, textureImage, CV_BGR2RGB);
            // opencv reads image from top-left, while GL expects it from bottom-left
            // vertically flip the image
            cv::flip(textureImage, textureImage, 0);

            // bind the texture
            glBindTexture(GL_TEXTURE_2D, textureGLNames[i]);
            // specify linear filtering
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            // load the OpenCV Mat into GLES
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureImage.cols,
                         textureImage.rows, 0, GL_RGB, GL_UNSIGNED_BYTE,
                         textureImage.data);
            CheckGLError("AssimpLoader::loadGLTexGen");

        } else {

            MyLOGE("Couldn't load texture %s", textureFilename.c_str());

            //Cleanup and return
            delete[] textureGLNames;
            return false;

        }
    }

    //Cleanup and return
    delete[] textureGLNames;
    return true;
}

/**
 * Loads a general OBJ with many meshes -- assumes texture is associated with each mesh
 * does not handle material properties (like diffuse, specular, etc.)
 */
bool ModelObject::Load3DModel(std::string modelFilename) {

    MyLOGI("Scene will be imported now");
    _aiScene = _importerPtr->ReadFile(modelFilename, aiProcessPreset_TargetRealtime_Quality);

    // Check if import failed
    if (!_aiScene) {
        std::string errorString = _importerPtr->GetErrorString();
        MyLOGE("Scene import failed: %s", errorString.c_str());
        return false;
    }
    MyLOGI("Imported %s successfully.", modelFilename.c_str());

    if(gUseAnimation) {
        _animationLoader->loadAnimation(_aiScene, _animator);
        _animator->init();
    }

    if(!LoadTexturesToGL(modelFilename)) {
        MyLOGE("Unable to load textures");
        return false;
    }
    MyLOGI("Loaded textures successfully");

    ProcessNode(_aiScene->mRootNode);
    MyLOGI("Loaded vertices and texture coords successfully");

    isObjectLoaded = true;
    return true;
}

/**
 * Clears memory associated with the 3D model
 */
void ModelObject::Delete3DModel() {
    if (isObjectLoaded) {
        //TODO clear modelMeshes stuff
//        for (unsigned int i = 0; i < modelMeshes.size(); ++i) {
//            glDeleteTextures(1, &(modelMeshes[i].textureIndex));
//        }
        modelMeshes.clear();

        MyLOGI("Deleted Assimp object");
        isObjectLoaded = false;
    }
}


/**
 * Renders the 3D model by rendering every mesh in the object
 */
void ModelObject::Render3DModel(MyGLCamera * cam) {

    if (!isObjectLoaded) {
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgramID);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(textureSamplerLocation, 0);

    auto numberOfLoadedMeshes = modelMeshes.size();

//    MyLOGI("Render3DModel elpasedTime: %f, DeltaTime: %f", elpasedTime, DeltaTime);

    if(gUseAnimation) {
        _animator->tick(GetCurrentTime() - startTime);
        _animator->render();
    }

    // render all meshes
    for (unsigned int i = 0; i < numberOfLoadedMeshes; ++i) {
        glm::mat4 mvpMat = cam->GetMVP(modelMeshes[i].baseModelMatrix);
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, (const GLfloat *) &mvpMat);

        // Texture
        if (modelMeshes[i].textureIndex) {
            glBindTexture( GL_TEXTURE_2D, modelMeshes[i].textureIndex);

            // Texture coords
            glBindBuffer(GL_ARRAY_BUFFER, modelMeshes[i].textureCoordBuffer);
            glEnableVertexAttribArray(vertexUVAttribute);
            glVertexAttribPointer(vertexUVAttribute, 2, GL_FLOAT, 0, 0, 0);
        }

        // Faces
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelMeshes[i].faceBuffer);

        // Vertices
        glBindBuffer(GL_ARRAY_BUFFER, modelMeshes[i].vertexBuffer);
        glVertexAttribPointer(vertexAttribute, 3, GL_FLOAT, 0, 0, 0);
        glEnableVertexAttribArray(vertexAttribute);

        glDrawElements(GL_TRIANGLES, modelMeshes[i].numberOfFaces * 3, GL_UNSIGNED_INT, 0);

        CheckGLError("AssimpLoader::renderObject() ");
    }

    // unbind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

