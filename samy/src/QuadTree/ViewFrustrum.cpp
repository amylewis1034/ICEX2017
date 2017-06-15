#include "ViewFrustrum.hpp"
#include <glm/glm.hpp>

#include <iostream>
#include <glm/gtx/string_cast.hpp>

ViewFrustrum::ViewFrustrum() {}
ViewFrustrum::~ViewFrustrum() {}

void ViewFrustrum::extractPlanes(glm::mat4 P, glm::mat4 V) {
	glm::mat4 comp = P*V;
	glm::vec3 n;
	float l;

	Left.x = comp[0][3] + comp[0][0];
	Left.y = comp[1][3] + comp[1][0];
	Left.z = comp[2][3] + comp[2][0];
	Left.w = comp[3][3] + comp[3][0];
	n = glm::vec3(Left.x, Left.y, Left.z);
	l = glm::length(n);
	planes[0] = Left/l;

	Right.x = comp[0][3] - comp[0][0];
	Right.y = comp[1][3] - comp[1][0];
	Right.z = comp[2][3] - comp[2][0];
	Right.w = comp[3][3] - comp[3][0];

	n = glm::vec3(Right.x, Right.y, Right.z);
	l = glm::length(n);
	planes[1] = Right/l;

	Bottom.x = comp[0][3] + comp[0][1];
	Bottom.y = comp[1][3] + comp[1][1];
	Bottom.z = comp[2][3] + comp[2][1];
	Bottom.w = comp[3][3] + comp[3][1];

	n = glm::vec3(Bottom.x, Bottom.y, Bottom.z);
	l = glm::length(n);
	planes[2] = Bottom/l;

	Top.x = comp[0][3] - comp[0][1];
	Top.y = comp[1][3] - comp[1][1];
	Top.z = comp[2][3] - comp[2][1];
	Top.w = comp[3][3] - comp[3][1];

	n = glm::vec3(Top.x, Top.y, Top.z);
	l = glm::length(n);
	planes[3] = Top/l;

	Near.x = comp[0][3] + comp[0][2];
	Near.y = comp[1][3] + comp[1][2];
	Near.z = comp[2][3] + comp[2][2];
	Near.w = comp[3][3] + comp[3][2];

	n = glm::vec3(Near.x, Near.y, Near.z);
	l = glm::length(n);
	planes[4] = Near/l;

	Far.x = comp[0][3] - comp[0][2];
	Far.y = comp[1][3] - comp[1][2];
	Far.z = comp[2][3] - comp[2][2];
	Far.w = comp[3][3] - comp[3][2];

	n = glm::vec3(Far.x, Far.y, Far.z);
	l = glm::length(n);
	planes[5] = Far/l;
}

void ViewFrustrum::printPlanes() {
	for (int i = 0; i < 6; i++) {
		std::cout << glm::to_string(planes[i]) << std::endl;
	}
}