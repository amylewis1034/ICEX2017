#pragma once
#ifndef _PRM_ALG_H_
#define _PRM_ALG_H_

#include "PRMNode.h"

extern std::vector<int> highWeightNodes;

extern bool genThirds, genNorms, genCombo, playPaths;

PRMNode *generateRootPRMNode();
void setRootPRMNode(PRMNode *rootNode);
PRMNode *generatePRMNode();

#endif
