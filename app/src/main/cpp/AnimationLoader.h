//
// Copied from https://gist.github.com/Bumrang/557358f710a0efa76f3a
//

#ifndef ASSIMPDRAWER_MODELLOADER_H
#define ASSIMPDRAWER_MODELLOADER_H


#include <assimp/scene.h>
#include "Animator.h"

class AnimationLoader
{
private:
    void processNode(const aiScene* scene, aiNode* node, Animator* m);
    void processMesh(const aiScene* scene, aiNode* node, aiMesh* mesh, Animator* animator);
    void processAnimations(const aiScene* scene, Animator* m);


public:
    // this will load all of the required data and dump it into the model struct
    bool loadAnimation(const aiScene *scene, Animator *m);
};


#endif //ASSIMPDRAWER_MODELLOADER_H
