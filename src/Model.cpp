#include "Model.h"
#include <vector>

//-------------------------------------------------------------------------------------
//---------------------------------------- MODELS -------------------------------------
//-------------------------------------------------------------------------------------

//intersection test of ray vs plane
float plane::CheckRay(ray mRay, intersection_stats& stats)
{
	glm::vec3 PC = mRay.origin - position;
	float denominator = glm::dot(mRay.dir, normal);

	//parallel to the plane
	if (denominator == 0) return -1.0f;

	//return the intersection
	return -glm::dot(PC, normal) / denominator;
}

//check half space test of ray vs a plane
bool plane::CheckHalfSpaceRay(ray mRay, glm::vec2& interval)
{
	//initialize interval and varibales to use in the algorithm
	glm::vec2 m_interval{ 0.0f, FLT_MAX };
	glm::vec3 PC = mRay.origin - position;
	float denominator = glm::dot(mRay.dir, normal);
	float numerator = glm::dot(PC, normal);

	//get intersection time
	intersection_stats stats;
	float t = CheckRay(mRay, stats);

	//ray towards the back plane
	if (denominator < 0.0f)
	{
		//if ray starts in front
		if (numerator > 0.0f)
			m_interval.x = t;
	}
		
	//if ray towards the front plane
	else if (denominator > 0.0f)
	{
		//if ray starts behind
		if (numerator < 0.0f)
			m_interval.y = t;
		//ray stars infront, no interval
		else
			return false;
	}
	
	//return a succesfull test
	interval = m_interval;
	return true;
}

//intersection test of ray versus sphere
float sphere::CheckRay(ray mRay, intersection_stats& stats)
{
    //compute the constants of the quadratic equation
    float a = glm::length2(mRay.dir);
    float b = 2 * (glm::dot(mRay.dir, mRay.origin - position));
    float c = glm::length2(mRay.origin - position) - (radius * radius);

    //precompute some variables of the quadratic equation
    float inside_sqrt = b * b - 4 * a * c;
    float denominator = (2 * a);

    //check if the intersection havent got solution
    if (inside_sqrt <= 0 || denominator == 0.0f) return -1.0f;

    //compute the 2 solutions to the intersection
    float sqrt = glm::sqrt(inside_sqrt);
    float t1 = (-b - sqrt) / (2 * a);
    float t2 = (-b + sqrt) / (2 * a);
    
    //no intersection
    if (t1 < 0 && t2 < 0) return -1.0f;

    //there is intersection
	float t = 0.0f;
	if (t1 >= 0 && t2 >= 0)
	{
		t = t1;
		//store the intersection point
		stats.intersP = mRay.at(t);
		stats.normal = glm::normalize(stats.intersP - position);
	}
	if (t1 < 0)
	{
		t = t2;
		//store the intersection point
		stats.intersP = mRay.at(t);
		stats.normal = glm::normalize(position - stats.intersP);
	}
	//return the intersection time
	return t;
}

//intersection test of ray vs box
float box::CheckRay(ray mRay, intersection_stats& stats)
{
	//create the 6 planes of a box
	std::vector<plane> planes;
	planes.push_back({ position, glm::normalize(glm::cross(length, height)) });				//front
	planes.push_back({ position + width, glm::normalize(glm::cross(height, length)) });		//back
	planes.push_back({ position, glm::normalize(glm::cross(height, width)) });				//left
	planes.push_back({ position + length, glm::normalize(glm::cross(width, height)) });		//right
	planes.push_back({ position, glm::normalize(glm::cross(width, length)) });				//bot
	planes.push_back({ position + height, glm::normalize(glm::cross(length, width)) });		//top

	//initialize interval
	glm::vec2 interval = { 0.0f, FLT_MAX };
	glm::vec3 min_normal = {};
	glm::vec3 max_normal = {};

	//iterate through the planes to check if the ray lies inside all of them
	for (plane p : planes)
	{
		//compute the current interval for the current plane
		glm::vec2 current_interval;
		if (!p.CheckHalfSpaceRay(mRay, current_interval))
			return -1.0f;

		//store temporal interval to track normal
		glm::vec2 temp = interval;

		//intersect intervals and check if everything went fine
		interval = { glm::max(current_interval.x, interval.x), glm::min(current_interval.y, interval.y) };
		if (interval.y < interval.x) return -1.0f;

		//check if any normal needs to be updated
		if (temp.x != interval.x) min_normal = p.normal;
		if (temp.y != interval.y) max_normal = -p.normal;
	}

	//if ray starts inside the box
	if (interval.x == 0.0f)
	{
		stats.normal = max_normal;
		stats.intersP = mRay.at(interval.y);
		return interval.y;
	}
		
	//else ray is outside the box
	stats.normal = min_normal;
	stats.intersP = mRay.at(interval.x);
	return interval.x;
}

float triangle::CheckRay(ray mRay, intersection_stats& stats)
{
	//check if ray crosses plane
	plane pl{ V1, glm::cross(a,b) };
	float t = pl.CheckRay(mRay, stats);
	if(t < 0.0f) return -1.0f;
	
	//initialize values
	float bb = glm::dot(b, b);
	float aa = glm::dot(a, a);
	float ab = glm::dot(a, b);

	//set intersection stats
	stats.intersP = mRay.at(t);
	stats.normal = pl.normal;

	//compute affine coordinates
	glm::mat2x2 mat = (1.0f / (aa * bb - ab * ab)) * glm::mat2x2(bb, -ab, -ab, aa);
	glm::vec2 coord = mat * glm::vec2(glm::dot(stats.intersP - V1, a), glm::dot(stats.intersP - V1, b));

	//check if intersection with triangle
	if (coord.x >= 0.0f && coord.y >= 0.0f && (coord.x + coord.y) <= 1.0f)
		return t;
	else
		return -1.0f;
}

float polygon::CheckRay(ray mRay, intersection_stats& stats)
{
	//init variables
	intersection_stats curr_stats;
	float t = FLT_MAX;
	bool intersection_valid = false;

	//iterate through the triangles 
	for (triangle& tri : triangles)
	{
		float temp_t = tri.CheckRay(mRay, curr_stats);

		//get min t
		if (temp_t < t && temp_t >= 0.0f)
		{
			t = temp_t;
			stats = curr_stats;
			intersection_valid = true;
		}
	}

	//return if there was a solution
	if (intersection_valid)
		return t;
	else
		return -1.0f;
}

//-------------------------------------------------------------------------------------
//---------------------------------------- MATERIALS ----------------------------------
//-------------------------------------------------------------------------------------


bool diffuse::GetBounceRay(ray mRay, ray& bounce_ray, intersection_stats int_stats)
{
	//get a random vector bounced
	float theta = glm::linearRand(0.0f, 1.0f) * 2.0f * PI;
	float phi = glm::acos(glm::linearRand(-1.0f, 1.0f));

	float x = sin(phi) * cos(theta);
	float y = sin(phi) * sin(theta);
	float z = cos(phi);

	//create the new bounced vector
	glm::vec3 new_dir = { x,y,z };
	bounce_ray = { int_stats.intersP + int_stats.normal * EPSILON, glm::normalize(new_dir + int_stats.normal) };

	return true;
}

bool metallic::GetBounceRay(ray mRay, ray& bounce_ray, intersection_stats int_stats)
{
	//get reflected vector
	glm::vec3 refl = mRay.dir - 2.0f * glm::dot(mRay.dir, int_stats.normal) * int_stats.normal ;

	//get random vector to offset the reflected vector
	float x = glm::linearRand(-1.0f, 1.0f);
	float y = glm::linearRand(-1.0f, 1.0f);
	float z = glm::linearRand(-1.0f, 1.0f);

	glm::vec3 v = glm::vec3{ x,y,z };
	if(v != glm::vec3(0.0f))
		v = roughness * glm::normalize(v);

	//check if the reflected vector is valid
	refl += v;
	float sign = glm::dot(int_stats.normal, refl);
	if (sign < 0.0f) return false;

	//create the new bounced ray
	bounce_ray = { int_stats.intersP + int_stats.normal * EPSILON, refl };
	return true;
}