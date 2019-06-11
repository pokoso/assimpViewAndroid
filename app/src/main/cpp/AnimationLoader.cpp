//
// Copied from https://gist.github.com/Bumrang/557358f710a0efa76f3a
//

#include "AnimationLoader.h"
#include "Utils.h"

bool AnimationLoader::loadAnimation(const aiScene *scene, Animator *m)
{
    Assimp::Importer importer; // used to import the model

    if (!scene)
    {
        MyLOGE("Error scene is null");
        return false;
    }

    // some debug stuff, delete later
    MyLOGI("Number of total meshes: %d", scene->mNumMeshes);
    MyLOGI("Animations: %d", scene->HasAnimations());

    if(!scene->HasAnimations())
        return true;

    // set 64 bones to identity, 64 is current limit, might increase it later
    processAnimations(scene, m);

    // start processing the model
    processNode(scene, scene->mRootNode, m);

    // must be called after processNode
    if (scene->HasAnimations())
        m->animations[m->currentAnim].buildBoneTree(scene, scene->mRootNode, &m->animations[m->currentAnim].root, m->boneID);

    m->modelTrans = glm::mat4(1.0f);
    m->modelLoaded = true;

    MyLOGI("loaded successfully.");
    return true;
};

void AnimationLoader::processNode(const aiScene* scene, aiNode* node, Animator* m)
{
    std::cout << "Processing a node: " << node->mName.C_Str() << std::endl; //debug
    // this is where the fun part starts.

    // cycle through each mesh within this node
    if (node->mNumMeshes > 0) {
        // cycle through each mesh
        for (unsigned int x = 0; x < node->mNumMeshes; x++) {
            processMesh(scene, node,
                        scene->mMeshes[node->mMeshes[x]], // scene contains all of the meshes, nodes simply have indices into the scene mesh array
                        m);
        }
    }
    if (m->boneID.find(node->mName.data) != m->boneID.end())
        std::cout << node->mName.data << " IS A BONE NODE!!!!";

    // then go through each child in the node and process them as well
    if (node->mNumChildren > 0)
    {
        for (unsigned int x = 0; x < node->mNumChildren; x++)
        {
            processNode(scene, node->mChildren[x], m);
        }
    }

};

// add some error handling (not all models have uvs, etc)
void AnimationLoader::processMesh(const aiScene* scene, aiNode* node, aiMesh* mesh, Animator* animator)
{
    std::cout << "Processing a mesh: " << mesh->mName.C_Str() << std::endl; //debug

    std::cout << "Has bones? " << mesh->mNumBones << std::endl;

    Animator::Mesh tempMesh;
    tempMesh.weights.resize(mesh->mNumVertices);
    std::fill(tempMesh.weights.begin(), tempMesh.weights.end(), glm::vec4(1.0f));
    tempMesh.boneID.resize(mesh->mNumVertices);
    std::fill(tempMesh.boneID.begin(), tempMesh.boneID.end(), glm::vec4(-123.0f));

    tempMesh.baseModelMatrix = Utils::toMat4(&node->mTransformation);
    if (node->mParent != NULL)
        tempMesh.baseModelMatrix = Utils::toMat4(&node->mParent->mTransformation) * Utils::toMat4(&node->mTransformation);

    if (mesh->HasBones())
    {
        for (int x = 0; x < mesh->mNumBones; x++)
        {
            // bone index, decides what bone we modify
            unsigned int index = 0;

            if (animator->boneID.find(mesh->mBones[x]->mName.data) == animator->boneID.end())
            { // create a new bone
                // current index is the new bone
                index = animator->boneID.size();
            }
            else
            {
                index = animator->boneID[mesh->mBones[x]->mName.data];
            }

            animator->boneID[mesh->mBones[x]->mName.data] = index;

            for (int y = 0; y < animator->animations[animator->currentAnim].channels.size(); y++)
            {
                if (animator->animations[animator->currentAnim].channels[y].name == mesh->mBones[x]->mName.data)
                    animator->animations[animator->currentAnim].boneOffset[mesh->mBones[x]->mName.data] = Utils::toMat4(&mesh->mBones[x]->mOffsetMatrix);
            }

            for (int y = 0; y < mesh->mBones[x]->mNumWeights; y++)
            {
                unsigned int vertexID = mesh->mBones[x]->mWeights[y].mVertexId;
                // first we check if the boneid vector has any filled in
                // if it does then we need to fill the weight vector with the same value
                if (tempMesh.boneID[vertexID].x == -123)
                {
                    tempMesh.boneID[vertexID].x = index;
                    tempMesh.weights[vertexID].x = mesh->mBones[x]->mWeights[y].mWeight;
                }
                else if (tempMesh.boneID[vertexID].y == -123)
                {
                    tempMesh.boneID[vertexID].y = index;
                    tempMesh.weights[vertexID].y = mesh->mBones[x]->mWeights[y].mWeight;
                }
                else if (tempMesh.boneID[vertexID].z == -123)
                {
                    tempMesh.boneID[vertexID].z = index;
                    tempMesh.weights[vertexID].z = mesh->mBones[x]->mWeights[y].mWeight;
                }
                else if (tempMesh.boneID[vertexID].w == -123)
                {
                    tempMesh.boneID[vertexID].w = index;
                    tempMesh.weights[vertexID].w = mesh->mBones[x]->mWeights[y].mWeight;
                }
            }

        }
    }
    animator->meshes.push_back(tempMesh);
};

void AnimationLoader::processAnimations(const aiScene* scene, Animator* model)
{
    if (scene->HasAnimations())
    {
        for (int i = 0; i < scene->mNumAnimations; i++)
        {
            aiAnimation* animation = scene->mAnimations[i];
            Animation tempAnim;
            tempAnim.name = animation->mName.data;
            tempAnim.duration = animation->mDuration;
            tempAnim.ticksPerSecond = animation->mTicksPerSecond;
            //tempAnim.data = scene->mAnimations[i];

            // load in required data for animation so that we don't have to save the entire scene
            for (int j = 0; j < animation->mNumChannels; j++)
            {
                aiNodeAnim* nodeAnim = animation->mChannels[j];
                Animation::Channel tempChan;
                tempChan.name = nodeAnim->mNodeName.data;

                for (int k = 0; k < nodeAnim->mNumPositionKeys; k++)
                    tempChan.mPositionKeys.push_back(nodeAnim->mPositionKeys[k]);

                for (int k = 0; k < nodeAnim->mNumRotationKeys; k++)
                    tempChan.mRotationKeys.push_back(nodeAnim->mRotationKeys[k]);

                for (int k = 0; k < nodeAnim->mNumScalingKeys; k++)
                    tempChan.mScalingKeys.push_back(nodeAnim->mScalingKeys[k]);

                tempAnim.channels.push_back(tempChan);
            }

            model->currentAnim = 0;

            for (int j = 0; j < MAX_BONES; j++)
            {
                tempAnim.boneTrans.push_back(glm::mat4(1.0f));
            }

            model->animations.push_back(tempAnim);
        }

        model->animations[model->currentAnim].root.name = "rootBoneTreeNode";
    }
};