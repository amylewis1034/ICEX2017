#include <vector>
#include <iostream>
#include <fstream>
#include <time.h>
#include "Utilities.h"
#include "PRMAlg.h"

bool genThirds = false, genNorms = false, genCombo = false, playPaths = false;

std::vector<Node *> roadMap;
std::vector<int> highWeightNodes;
int nodesVecNdx = 0;
int iteration = 0;
double thirdsThresh = 0.08;
double normThresh = 0.1;
double comboThresh = (1 - thirdsThresh) + normThresh;
double threshDelta = 0.01;
time_t t = clock();
bool debug = true;

Node *generateRootPRMNode() {
	roadMap.clear();
	Node *rootNode(new Node());

	return rootNode;
}

void setRootPRMNode(Node *rootNode) {
	roadMap.push_back(rootNode);
	rootNode->setNdx(roadMap.size() - 1);
}

Node *generatePRMNode() {
	int nodeNdx = -1;

	int iterMod = iteration % 4;
	// Choose a node off the high weight list to expand off of
	if (highWeightNodes.size() > nodesVecNdx && (iterMod == 0 || iterMod == 1)) {
		// Iterate through vector instead of clearing it
		nodeNdx = highWeightNodes[nodesVecNdx++]; 
	}
	else { // Choose a random node to expand off of
		Node *currentNode = NULL;

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
	Node *newNode = new Node(roadMap.at(nodeNdx));
	roadMap.push_back(newNode);
	newNode->setNdx(roadMap.size() - 1);

	// We have a complete path!
	if (newNode->getPathLength() == totalPathLen) {
		int roadMapSize = roadMap.size();

		if (debug) {
			double avgRoadmap = 0;
			for (Node *mapNode : roadMap) {
				avgRoadmap += mapNode->getWeight();
			}
			std::cout << "roadMap size: " << roadMapSize << std::endl;
			std::cout << "avg roadmap node weight: " 
				<< avgRoadmap / roadMapSize << std::endl;
		}

		// Pull actual path off roadmap from current node up to root node
		std::vector<Node *> path;
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
		std::ofstream outfile("resources/path.txt");
		if (outfile.is_open()) {
			outfile << path.size() << std::endl;
			for (int i = 0; i < path.size(); i++) {
				outfile << path.at(i)->getPosition().transpose() << " " 
					<< path.at(i)->getDirection().transpose() << std::endl;
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