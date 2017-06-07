#pragma once
#ifndef _SHAPE_H_
#define _SHAPE_H_

#include <string>
#include <vector>
#include <memory>

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>

class Program;

class Shape
{
public:
	Shape();
	virtual ~Shape();
	void loadMesh(const std::string &meshName);
	void loadMesh(const std::string &meshName, bool texOn);
	void computeNormals();
	void init();
	void resize();
	void moveToGround();
	void print();
	Eigen::Vector3f getCenter();
	void draw(const std::shared_ptr<Program> prog) const;
	
// private:
	std::vector<unsigned int> eleBuf;
	std::vector<float> posBuf;
	std::vector<float> norBuf;
	std::vector<float> texBuf;
	unsigned eleBufID;
	unsigned posBufID;
	unsigned norBufID;
	unsigned texBufID;
   unsigned vaoID;
};

#endif
