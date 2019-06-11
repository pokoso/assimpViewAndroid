//
//
//

#ifndef ASSIMPDRAWER_UTILS_H
#define ASSIMPDRAWER_UTILS_H


#include <assimp/matrix4x4.h>
#include <glm/detail/type_mat.hpp>

class Utils {
public:
    static glm::mat4 toMat4(aiMatrix4x4* ai);
};


#endif //ASSIMPDRAWER_UTILS_H
