#pragma once
#ifndef _PRM_ALG_H_
#define _PRM_ALG_H_

#include "Node.h"

extern std::vector<int> highWeightNodes;

extern bool genThirds, genNorms, genCombo, playPaths;

Node *generateRootPRMNode();
void setRootPRMNode(Node *rootNode);
Node *generatePRMNode();

#endif