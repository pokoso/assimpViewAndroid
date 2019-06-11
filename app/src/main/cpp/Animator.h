//
// Copied from https://gist.github.com/Bumrang/557358f710a0efa76f3a
//

#ifndef ASSIMPDRAWER_MODEL_H
#define ASSIMPDRAWER_MODEL_H


#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include "misc.h"
#include "myGLM.h"
#include "myGLFunctions.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "Animation.h"

#define MAX_BONES 255

class Animator
{
public:
    struct Mesh {
        std::vector <glm::vec4> weights;
        std::vector <glm::vec4> boneID;

        glm::mat4 baseModelMatrix;

        GLuint weightBuffer;
        GLuint boneIdBuffer;
        GLuint weightAttribute, boneAttribute;

        GLint boneTransID;
    };

    Animator(GLuint programId);

    void init();
    void tick(double time);
    void render();

    std::vector<Animation> animations;
    unsigned int currentAnim;
    std::map <std::string, unsigned int> boneID;
    glm::mat4 modelTrans;
    bool modelLoaded;
    std::string rootPath;
    std::vector <Mesh> meshes;

private:
    void setAniamtion(std::string name);
    std::vector <std::string> animNames;

    void updateBoneTree(double time, Animation::BoneNode* node, glm::mat4 transform);
    void CalcInterpolatedPosition(int chanIndex, ai_real animTime, aiVector3D &aiTranslation);
    void CalcInterpolatedRotation(int chanIndex, ai_real animTime, aiQuaternion &aiTranslation);
    void CalcInterpolatedScaling(int chanIndex, ai_real animTime, aiVector3D &aiScale);

    void setModelTrans(glm::mat4);

    Assimp::Interpolator<aiQuaternion> slerp;
    Assimp::Interpolator<aiVector3D> lerp;

    GLuint _programId;
};


#endif //ASSIMPDRAWER_MODEL_H
