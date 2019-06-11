//
// Copied from https://gist.github.com/Bumrang/557358f710a0efa76f3a
//

#include <sstream>
#include <glm/ext.hpp>
#include "Animator.h"
#include "myShader.h"


Animator::Animator(GLuint programId) {
    _programId = programId;
    modelLoaded = false;
};

// why even have this as an external function? just call it in loadModel
void Animator::init()
{
    if (!modelLoaded)
    {
//        log("Please load in a model before initializing buffers.", warning);
        assert(false);
        return;
    }

    // loop through each mesh and initialize them
    for (int x = 0; x < meshes.size(); x++) {
        glGenBuffers(1, &meshes[x].weightBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, meshes[x].weightBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * meshes[x].weights.size(), &meshes[x].weights[0], GL_STATIC_DRAW);

        glGenBuffers(1, &meshes[x].boneIdBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, meshes[x].boneIdBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * meshes[x].boneID.size(), &meshes[x].boneID[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, meshes[x].weightBuffer);
        meshes[x].weightAttribute = (GLuint)glGetAttribLocation(_programId, "weight");
        glEnableVertexAttribArray(meshes[x].weightAttribute);
        glVertexAttribPointer(meshes[x].weightAttribute, 4, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, meshes[x].boneIdBuffer);
        meshes[x].boneAttribute = (GLuint)glGetAttribLocation(_programId, "boneID");
        glEnableVertexAttribArray(meshes[x].boneAttribute);
        glVertexAttribPointer(meshes[x].boneAttribute, 4, GL_FLOAT, GL_FALSE, 0, 0);

        meshes[x].boneTransID = glGetUniformLocation(_programId, "boneTransformation");
    }
};


void Animator::tick(double time) {
    if(animations.size() > 0) {
        double timeInTicks = time * animations[currentAnim].ticksPerSecond;
        updateBoneTree(timeInTicks, &animations[currentAnim].root, glm::mat4(1.0f));
    }
};

void Animator::CalcInterpolatedPosition(int chanIndex, ai_real animTime, aiVector3D &aiTranslation) {
    ai_real factor = 0;
    if (animations[currentAnim].channels[chanIndex].mPositionKeys.size() > 1) {
        uint index = 0;
        uint nextIndex = 1;
        ai_real deltaTime = 0;
        for(uint i=0; i<animations[currentAnim].channels[chanIndex].mPositionKeys.size()-1; i++) {
            if(animTime < animations[currentAnim].channels[chanIndex].mPositionKeys[i+1].mTime) {
                index = i;
                nextIndex = i + 1;
                deltaTime = animations[currentAnim].channels[chanIndex].mPositionKeys[i+1].mTime - animations[currentAnim].channels[chanIndex].mPositionKeys[i].mTime;
                factor = (animTime - animations[currentAnim].channels[chanIndex].mPositionKeys[i].mTime) / deltaTime;
                break;
            }
        }
        lerp(aiTranslation,
             animations[currentAnim].channels[chanIndex].mPositionKeys[index].mValue,
             animations[currentAnim].channels[chanIndex].mPositionKeys[nextIndex].mValue,
             factor); // translation
    }
}

void Animator::CalcInterpolatedRotation(int chanIndex, ai_real animTime, aiQuaternion &aiRotation) {
    ai_real factor = 0;
    if (animations[currentAnim].channels[chanIndex].mRotationKeys.size() > 1) {
        uint index = 0;
        uint nextIndex = 1;
        ai_real deltaTime = 0;
        for(uint i=0; i<animations[currentAnim].channels[chanIndex].mRotationKeys.size()-1; i++) {
            if(animTime < animations[currentAnim].channels[chanIndex].mRotationKeys[i+1].mTime) {
                index = i;
                nextIndex = i + 1;
                deltaTime = animations[currentAnim].channels[chanIndex].mRotationKeys[i+1].mTime - animations[currentAnim].channels[chanIndex].mRotationKeys[i].mTime;
                factor = (animTime - animations[currentAnim].channels[chanIndex].mRotationKeys[i].mTime) / deltaTime;
                break;
            }
        }
        slerp(aiRotation,
              animations[currentAnim].channels[chanIndex].mRotationKeys[index].mValue,
              animations[currentAnim].channels[chanIndex].mRotationKeys[nextIndex].mValue,
              factor); // rotation
    }
}

void Animator::CalcInterpolatedScaling(int chanIndex, ai_real animTime, aiVector3D &aiScale) {
    ai_real factor = 0;
    if (animations[currentAnim].channels[chanIndex].mScalingKeys.size() > 1) {
        uint index = 0;
        uint nextIndex = 1;
        ai_real deltaTime = 0;
        for(uint i=0; i<animations[currentAnim].channels[chanIndex].mScalingKeys.size()-1; i++) {
            if(animTime < animations[currentAnim].channels[chanIndex].mScalingKeys[i+1].mTime) {
                index = i;
                nextIndex = i + 1;
                deltaTime = animations[currentAnim].channels[chanIndex].mScalingKeys[i+1].mTime - animations[currentAnim].channels[chanIndex].mScalingKeys[i].mTime;
                factor = (animTime - animations[currentAnim].channels[chanIndex].mScalingKeys[i].mTime) / deltaTime;
                break;
            }
        }
        lerp(aiScale,
             animations[currentAnim].channels[chanIndex].mScalingKeys[index].mValue,
             animations[currentAnim].channels[chanIndex].mScalingKeys[nextIndex].mValue,
             factor); // scale
    }
}

void Animator::updateBoneTree(double timeInTicks, Animation::BoneNode* node, glm::mat4 parentTransform) {
//    MyLOGI("updateBoneTree timeInTicks:%f", timeInTicks);

    int chanIndex = 0;
    for (int x = 0; x < animations[currentAnim].channels.size(); x++) {
        if (node->name == animations[currentAnim].channels[x].name) {
            chanIndex = x;
        }
    }

    ai_real animTime = std::fmod(timeInTicks, animations[currentAnim].duration);

    aiVector3D aiTranslation(animations[currentAnim].channels[chanIndex].mPositionKeys[0].mValue);
    CalcInterpolatedPosition(chanIndex, animTime, aiTranslation);

    aiVector3D aiScale(animations[currentAnim].channels[chanIndex].mScalingKeys[0].mValue);
    CalcInterpolatedScaling(chanIndex, animTime, aiScale);

    aiQuaternion aiRotation(animations[currentAnim].channels[chanIndex].mRotationKeys[0].mValue);
    CalcInterpolatedRotation(chanIndex, animTime, aiRotation);

    glm::vec3 translation((GLfloat)aiTranslation.x, (GLfloat)aiTranslation.y, (GLfloat)aiTranslation.z);
    glm::vec3 scaling((GLfloat)aiScale.x, (GLfloat)aiScale.y, (GLfloat)aiScale.z);
    glm::quat rotation((GLfloat)aiRotation.w, (GLfloat)aiRotation.x, (GLfloat)aiRotation.y, (GLfloat)aiRotation.z);

    glm::mat4 finalModel =
            parentTransform
            * glm::translate(glm::mat4(1.0f), translation)
            * glm::mat4_cast(rotation)
            * glm::scale(glm::mat4(1.0f), scaling);

    animations[currentAnim].boneTrans[boneID[node->name]] = finalModel * animations[currentAnim].boneOffset[node->name];

    // loop through every child and use this bone's transformations as the parent transform
    for (int x = 0; x < node->children.size(); x++) {
        updateBoneTree(timeInTicks, &node->children[x], finalModel);
    }
};

// this is just a generic render function for quick and easy rendering
void Animator::render() {
    if (!modelLoaded) {
        assert(false);
        return;
    }

    if(animations.size() == 0)
        return;

    glUniformMatrix4fv(meshes[0].boneTransID, animations[currentAnim].boneTrans.size(), GL_FALSE, (GLfloat*)&animations[currentAnim].boneTrans[0][0]);
};

void Animator::setModelTrans (glm::mat4 in)
{
    modelTrans = in;
};

