#pragma once

#include <SFML/Graphics.hpp>

#include "Model.h"
#include <vector>
#include <string>



struct Camera
{
	glm::vec3 pos;
	glm::vec3 target;
	float focal;

	glm::vec3 view;
	glm::vec3 up;
	glm::vec3 right;
};


class RayTracerClass
{
public:
	static RayTracerClass& Instance()
	{
		static RayTracerClass instance;
		return instance;
	}
	
	void Initialize(unsigned Width, unsigned Height);

	void RenderScene();
	void RenderThread();
	void RenderNoThread();

	void DeleteScene();
	void Restart();
	bool LoadScene(std::string file_name);
	void TakeScreenShot(std::string screen_shoot_name) { image.saveToFile(screen_shoot_name); }

	model* CheckRay(ray mRay, intersection_stats& stats);
	glm::vec3	TraceRayRecursive(ray mRay, unsigned current_bounces);

	ray	GetRayToPixel(glm::uvec2 pixel);

	//GETTORS
	sf::Image&			get_image()   { return image; };
	sf::Texture&		get_texture() { return texture; };
	sf::Sprite&			get_sprite()  { return sprite; };
	sf::RenderWindow&	get_window()  { return window; };

public:
	std::vector<model*> objects;
	Camera				mCamera;
	std::string		    scene_name = "Scene.txt";
	std::string         screen_shoot_name = "screenshot.png";

	//raytracer variables
	bool				mbfinished = false;
	unsigned			cluster_size = 1000;
	unsigned			pixels_processed = 0;
	unsigned			bouncess = 20;
	unsigned			total_samples = 100;
	unsigned			current_samples = 0;

	//thread variables
	unsigned			thread_count = 0;
	unsigned			thread_pixels = 0;
	unsigned			last_pixels = 0;
	bool				mbUseThreads = true;
private:
	RayTracerClass() = default;
	RayTracerClass(const RayTracerClass&) = default;

	sf::Image   image;
	sf::Texture texture;
	sf::Sprite  sprite;
	sf::RenderWindow window;

	glm::uvec2 window_size = {};
	glm::vec3 ambient = { 1.0f,1.0f,1.0f };
	std::vector<glm::uvec2> pixels;
	glm::vec3* FrameBuffer = nullptr;
};

#define RayTracer (RayTracerClass::Instance())