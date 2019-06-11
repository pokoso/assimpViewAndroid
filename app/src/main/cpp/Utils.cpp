//
//
//

#include <glm/detail/type_mat4x4.hpp>
#include "Utils.h"


glm::mat4 Utils::toMat4(aiMatrix4x4* ai)
{
    glm::mat4 mat;

    mat[0][0] = ai->a1; mat[1][0] = ai->a2; mat[2][0] = ai->a3; mat[3][0] = ai->a4;
    mat[0][1] = ai->b1; mat[1][1] = ai->b2; mat[2][1] = ai->b3; mat[3][1] = ai->b4;
    mat[0][2] = ai->c1; mat[1][2] = ai->c2; mat[2][2] = ai->c3; mat[3][2] = ai->c4;
    mat[0][3] = ai->d1; mat[1][3] = ai->d2; mat[2][3] = ai->d3; mat[3][3] = ai->d4;

    return mat;
}