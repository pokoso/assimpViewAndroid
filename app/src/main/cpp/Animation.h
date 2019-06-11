//
// Copied from https://gist.github.com/Bumrang/557358f710a0efa76f3a
//

#ifndef ASSIMPDRAWER_ANIMATION_H
#define ASSIMPDRAWER_ANIMATION_H

#include <map>
#include <vector>

#include <assimp/scene.h>
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_mat4x4.hpp>

using namespace std;

class Animation
{
public:
    std::string name;
    double duration;
    double ticksPerSecond;
    // all of the bone transformations, this is modified every frame
    // assimp calls it a channel, its anims for a node aka bone
    std::vector <glm::mat4> boneTrans;
    std::map <std::string, glm::mat4> boneOffset;

    struct Channel
    {
        std::string name;
        glm::mat4 offset;
        std::vector <aiVectorKey> mPositionKeys;
        std::vector <aiQuatKey> mRotationKeys;
        std::vector <aiVectorKey> mScalingKeys;
    };
    std::vector <Channel> channels;

    struct BoneNode
    {
        std::string name;
        BoneNode* parent;
        std::vector <BoneNode> children;
        glm::mat4 nodeTransform;
        glm::mat4 boneTransform;
    };
    BoneNode root;

    void buildBoneTree(const aiScene* scene, aiNode* node, Animation::BoneNode* bNode, map<string, unsigned int> &boneID);
};


#endif //ASSIMPDRAWER_ANIMATION_H
