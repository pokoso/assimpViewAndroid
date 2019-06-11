//
// Copied from https://gist.github.com/Bumrang/557358f710a0efa76f3a
//

#include "Animation.h"
#include "Utils.h"

void Animation::buildBoneTree(const aiScene* scene, aiNode* node, BoneNode* bNode, map<string, unsigned int> &boneID)
{
    if (scene->HasAnimations())
    {
        // found the node
        if (boneID.find(node->mName.data) != boneID.end())
        {
            BoneNode tempNode;
            tempNode.name = node->mName.data;
            tempNode.parent = bNode;
            tempNode.nodeTransform = Utils::toMat4(&node->mTransformation);

            // bones and their nodes always share the same name
            tempNode.boneTransform = boneOffset[tempNode.name];
            bNode->children.push_back(tempNode);
        }

        if (node->mNumChildren > 0)
        {
            for (unsigned int x = 0; x < node->mNumChildren; x++)
            {
                // if the node we just found was a bone node then pass it in (current bone node child vector size - 1)
                if (boneID.find(node->mName.data) != boneID.end())
                    buildBoneTree(scene, node->mChildren[x], &bNode->children[bNode->children.size() - 1], boneID);
                else
                    buildBoneTree(scene, node->mChildren[x], bNode, boneID);
            }
        }
    }
};