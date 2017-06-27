#include <vector>
#include <iostream>
#include <fstream>
#include <time.h>
#include "PRMAlg.h"
#include "Utilities.h"
#include <icex_common.hpp>
#include <glm/gtx/string_cast.hpp>

bool genThirds = false, genNorms = false, genCombo = false, playPaths = false;

std::vector<PRMNode *> roadMap;
std::vector<int> highWeightNodes;
int nodesVecNdx = 0;
int iteration = 0;
double thirdsThresh = 0.08;
double normThresh = 0.1;
double comboThresh = (1 - thirdsThresh) + normThresh;
double threshDelta = 0.01;
time_t t = clock();
bool debug = true;

PRMNode *generateRootPRMNode(int numNodes) {
	roadMap.clear();
	PRMNode *rootNode(new PRMNode());

	return rootNode;
}

void setRootPRMNode(PRMNode *rootNode) {
	roadMap.push_back(rootNode);
	rootNode->setNdx(roadMap.size() - 1);
}

PRMNode *generatePRMNode(int numNodes) {
	int nodeNdx = -1;

	int iterMod = iteration % 4;
	// Choose a node off the high weight list to expand off of
	if (highWeightNodes.size() > nodesVecNdx && (iterMod == 0 || iterMod == 1)) {
		// Iterate through vector instead of clearing it
		nodeNdx = highWeightNodes[nodesVecNdx++]; 
	}
	else { // Choose a random node to expand off of
		PRMNode *currentNode = NULL;

		// Thirds wants smallest weight node
		// Norms and combo want largest weight node
		double weightThresh;
		if (genThirds) {
			weightThresh = thirdsThresh - threshDelta;
		}
		else {
			weightThresh = genNorms ? normThresh : comboThresh + threshDelta;
		}

		do {
			nodeNdx = randRange(0, roadMap.size());
			currentNode = roadMap.at(nodeNdx);
			if (genThirds) {
				weightThresh += threshDelta;
			}
			else { // genNorms or genCombo
				weightThresh -= threshDelta;
			}
		} while (currentNode && 
			genThirds ? currentNode->getWeight() > weightThresh 
				: currentNode->getWeight() < weightThresh);
	}

	++iteration;
	PRMNode *newNode = new PRMNode(roadMap.at(nodeNdx));
	roadMap.push_back(newNode);
	newNode->setNdx(roadMap.size() - 1);

	// We have a complete path!
	std::cout << "length: " << newNode->getPathLength() << std::endl;
	if (newNode->getPathLength() == numNodes) {
		int roadMapSize = roadMap.size();

		if (debug) {
			double avgRoadmap = 0;
			for (PRMNode *mapNode : roadMap) {
				avgRoadmap += mapNode->getWeight();
			}
			std::cout << "roadMap size: " << roadMapSize << std::endl;
			std::cout << "avg roadmap node weight: " 
				<< avgRoadmap / roadMapSize << std::endl;
		}

		// Pull actual path off roadmap from current node up to root node
		std::vector<PRMNode *> path;
		double avgPathWeight = 0;
		while (newNode != NULL) {
			path.push_back(newNode);
			avgPathWeight += newNode->getWeight();
			newNode = newNode->getParent();
		}
		// We built path from the leaf up to root so reverse it
		std::reverse(path.begin(), path.end());

		avgPathWeight /= path.size();
		std::cout << avgPathWeight << std::endl;

		if (debug) {
			std::cout << "\npercent in high weight list: " 
				<< (double)highWeightNodes.size() / roadMapSize << std::endl;
		}

		roadMap.clear();

		// Write path to file
		std::ofstream outfile(RESOURCE_PATH "path_test.txt");
		if (outfile.is_open()) {
			outfile << path.size() << std::endl;
			for (int i = 0; i < path.size(); i++) {
				outfile << path[i]->getPosition()[0] << " " << path[i]->getPosition()[1] << " " << path[i]->getPosition()[2] << " " 
						<< path[i]->getDirection()[0] << " " << path[i]->getDirection()[1] << " " << path[i]->getDirection()[2] << std::endl;
			}
			// Write road map size, average node weight in path, 
			// and time to generate path to file
			outfile << roadMapSize << std::endl;
			outfile << avgPathWeight << std::endl;
			outfile << (((float)(clock() - t))/CLOCKS_PER_SEC) << std::endl;
		}
		else {
			std::cerr << "Unable to open file" << std::endl;
		}
		outfile.close();
		exit(0);
	}

	return newNode;
}
