#pragma once
#ifndef _PRM_ALG_H_
#define _PRM_ALG_H_

#include "PRMNode.h"

extern std::vector<int> highWeightNodes;

extern bool genThirds, genNorms, genCombo, playPaths;

PRMNode *generateRootPRMNode(int numNodes);
void setRootPRMNode(PRMNode *rootNode);
PRMNode *generatePRMNode(int numNodes);

#endif
