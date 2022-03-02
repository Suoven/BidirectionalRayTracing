#pragma once
#include "math.h"
#include <vector>

struct ray
{
	glm::vec3 origin;
	glm::vec3 dir;
	glm::vec3 at(float t) { return origin + dir * t; }
};

struct intersection_stats
{
	glm::vec3 intersP;
	glm::vec3 normal;
};

//-------------------------------------------------------------------------------------
//---------------------------------------- MODELS -------------------------------------
//-------------------------------------------------------------------------------------
struct material;

struct model
{
	glm::vec3 color;
	glm::vec3 position;

	material* mMaterial = nullptr;
	bool mbLight = false;

	virtual float CheckRay(ray mRay, intersection_stats& stats) = 0;
};

struct plane : public model
{
	plane(glm::vec3 pos, glm::vec3 n) :  normal{ n } { position = pos; }

	glm::vec3 normal; 
	float CheckRay(ray mRay, intersection_stats& stats);
	bool CheckHalfSpaceRay(ray mRay, glm::vec2& interval);
};

struct sphere : public model
{
	float radius;
	float CheckRay(ray mRay, intersection_stats& stats);
};

struct box : public model
{
	glm::vec3 width;
	glm::vec3 height;
	glm::vec3 length;

	float CheckRay(ray mRay, intersection_stats& stats);
};

struct triangle : public model
{
	triangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) : V1{v1}, a{v2 - v1}, b{v3 - v1} {}
	glm::vec3 V1, a, b;

	float CheckRay(ray mRay, intersection_stats& stats);
};

struct polygon : public model
{
	std::vector<triangle> triangles;

	float CheckRay(ray mRay, intersection_stats& stats);
};

struct light : public sphere
{
	light() { mbLight = true; }
};

//-------------------------------------------------------------------------------------
//---------------------------------------- MATERIALS ----------------------------------
//-------------------------------------------------------------------------------------

struct material
{
	virtual bool GetBounceRay(ray mRay, ray& bounce_ray, intersection_stats int_stats) = 0;
};

struct diffuse : material
{
	bool GetBounceRay(ray mRay, ray& bounce_ray, intersection_stats int_stats);
};

struct metallic : material
{
	float roughness = 0.2f;
	bool GetBounceRay(ray mRay, ray& bounce_ray, intersection_stats int_stats);
};