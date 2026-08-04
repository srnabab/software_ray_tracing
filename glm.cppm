module;
#include <glm/glm.hpp>
#include <glm/ext.hpp>

export module GLM;

export using::glm::vec4;
export using::glm::vec3;
export using::glm::vec2;
export using::glm::ivec2;
export using::glm::ivec3;

export using::glm::operator-;
export using::glm::operator+;
export using::glm::operator*;
export using::glm::operator/;

export namespace GLM {
	 using::glm::lookAt;
	 using::glm::normalize;
	 using::glm::cross;
	 using::glm::clamp;
	 using::glm::tan;
	 using::glm::dot;
	 using::glm::max;
	 using::glm::min;
	 using::glm::pow;
	 using::glm::radians;
	 using::glm::distance;
}