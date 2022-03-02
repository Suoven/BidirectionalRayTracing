#include "Raytracer.h"
#include "ThreadPool.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>
#include <thread>

//initialize the raytracer
void RayTracerClass::Initialize(unsigned Width, unsigned Height)
{
	window_size = glm::uvec2(Width, Height);
	//create window, texture and image
	window.create(sf::VideoMode(Width, Height), "SFML works!");
	texture.create(Width, Height);
	image.create(Width, Height, sf::Color::Black);
	
	//initialize vector of indices to suffle the pixels process
	for (unsigned y = 0; y < Height; y++)
		for (unsigned x = 0; x < Width; x++)
			pixels.push_back(glm::uvec2{ x,y });

	//allocate memory for the frame buffer
	FrameBuffer = new glm::vec3[Width * Height]();

	//shuffle the pixels
	std::random_device rd;
	std::mt19937 g(rd());
	//std::shuffle(pixels.begin(), pixels.end(), g);

	//get number of threads
	thread_count = std::thread::hardware_concurrency() - 1;
	thread_pixels = (Width * Height) / thread_count;
	last_pixels = (Width * Height) - thread_pixels;
	ThreadPool.Instance();
}

//load the scene from a file
bool RayTracerClass::LoadScene(std::string file_name)
{
	//load config file
	std::ifstream config_file("config.txt");
	if (config_file.is_open() && config_file.good())
	{
		std::string line;
		// iterate through the file
		while(std::getline(config_file, line))
		{
			std::string bounces_str = "Bounces ";
			std::string samples_str = "Samples ";
			std::string clustersize_str = "ClusterSize ";
			std::string use_thread_str = "UseThreads ";
			if ((line.compare(0, bounces_str.size(), bounces_str) == 0))
			{
				//get the important info from the line
				std::istringstream info(line.substr(bounces_str.size()).c_str());
				info >> bouncess;
			}
			else if((line.compare(0, samples_str.size(), samples_str) == 0))
			{
				//get the important info from the line
				std::istringstream info(line.substr(samples_str.size()).c_str());
				info >> total_samples;
			}
			else if ((line.compare(0, clustersize_str.size(), clustersize_str) == 0))
			{
				//get the important info from the line
				std::istringstream info(line.substr(clustersize_str.size()).c_str());
				info >> cluster_size;
			}
			else if ((line.compare(0, use_thread_str.size(), use_thread_str) == 0))
			{
				//get the important info from the line
				std::istringstream info(line.substr(use_thread_str.size()).c_str());
				info >> mbUseThreads;
			}
		}
	}

	//open the file
	std::ifstream scene_file(file_name.c_str());
	if (!scene_file.is_open() || !scene_file.good())
	{
		std::cout << "Filed to open " << file_name << std::endl;
		return false;
	}

	scene_name = file_name;
	//store current line
	std::string line;

	//different types of strings
	std::string sphere_str	 = "SPHERE ";
	std::string box_str		 = "BOX ";
	std::string diffuse_str  = "DIFFUSE ";
	std::string metallic_str = "METAL ";
	std::string ambient_str  = "AMBIENT ";
	std::string camera_str	 = "CAMERA ";
	std::string light_str    = "LIGHT ";
	std::string polygon_str  = "POLYGON ";
		
	//iterate through the file
	while (std::getline(scene_file, line))
	{
		//check if we need to skip the line
		if (line.empty() || line.front() == '#') continue;
		
		//char that will used to remove non usefull data like "(" or "," 
		char remover;

		//check for sphere
		if (line.compare(0, sphere_str.size(), sphere_str) == 0)
		{
			//get the important info from the line
			std::istringstream info(line.substr(sphere_str.size()).c_str());

			//create a new sphere and copy the contents and push it in the list of objects
			sphere* new_sphere = new sphere();
			info >> remover >> new_sphere->position.x >> remover >> new_sphere->position.y >> remover >> new_sphere->position.z
				>> remover >> new_sphere->radius;
			objects.push_back(new_sphere);
		}

		//check for box
		else if (line.compare(0, box_str.size(), box_str) == 0)
		{
			//get the important info from the line
			std::istringstream info(line.substr(box_str.size()));

			//create a new box and copy the contents and push it in the list of objects
			box* new_box = new box();
			info >> remover >> new_box->position.x >> remover >> new_box->position.y >> remover >> new_box->position.z >> remover
				 >> remover >> new_box->length.x >> remover >> new_box->length.y >> remover >> new_box->length.z >> remover
				 >> remover >> new_box->width.x >> remover >> new_box->width.y >> remover >> new_box->width.z >> remover
				 >> remover >> new_box->height.x >> remover >> new_box->height.y >> remover >> new_box->height.z;
			objects.push_back(new_box);
		}

		//check for light
		else if (line.compare(0, light_str.size(), light_str) == 0)
		{
			//get the important info from the line
			std::istringstream info(line.substr(light_str.size()));

			//create a new box and copy the contents and push it in the list of objects
			light* new_light = new light();
			info >> remover >> new_light->position.x >> remover >> new_light->position.y >> remover >> new_light->position.z
				>> remover >> new_light->radius
				>> remover >> new_light->color.x >> remover >> new_light->color.y >> remover >> new_light->color.z;
			objects.push_back(new_light);
		}

		//check for light
		else if (line.compare(0, polygon_str.size(), polygon_str) == 0)
		{
			//get the important info from the line
			std::istringstream info(line.substr(polygon_str.size()));

			//create a new polygon and copy the contents and push it in the list of objects
			polygon* new_pol = new polygon();
			int vertex_count = 0;
			glm::vec3 fan_center, v2, v3;

			//read vertex count fan center and the first vertex of the first triangle
			info >> vertex_count >> remover >> fan_center.x >> remover >> fan_center.y >> remover >> fan_center.z >> remover
				>> remover >> v3.x >> remover >> v3.y >> remover >> v3.z >> remover;	

			for (int i = 0; i < vertex_count - 2; i++)
			{
				//get v2 from previuous and read v3
				v2 = v3;
				info >> remover >> v3.x >> remover >> v3.y >> remover >> v3.z >> remover;

				//push new triangle to the polygon
				new_pol->triangles.push_back({ fan_center, v2, v3 });
			}
			objects.push_back(new_pol);
		}

		//check for diffuse
		else if (line.compare(0, diffuse_str.size(), diffuse_str) == 0)
		{
			//get the important info from the line
			std::istringstream info(line.substr(diffuse_str.size()));

			//get the last object and change its color
			model* object = objects.back();
			info >> remover >> object->color.x >> remover >> object->color.y >> remover >> object->color.z;
			object->mMaterial = new diffuse();
		}

		//check for metallic
		else if (line.compare(0, metallic_str.size(), metallic_str) == 0)
		{
			//get the important info from the line
			std::istringstream info(line.substr(metallic_str.size()));

			//get the last object and change its color
			model* object = objects.back();
			metallic* mMaterial = new metallic();
			info >> remover >> object->color.x >> remover >> object->color.y >> remover >> object->color.z >> remover 
				 >> mMaterial->roughness;
			object->mMaterial = mMaterial;
		}

		//check for ambient
		else if (line.compare(0, ambient_str.size(), ambient_str) == 0)
		{
			//get the important info from the line
			std::istringstream info(line.substr(ambient_str.size()));

			//update the ambient color intensity
			info >> remover >> ambient.x >> remover >> ambient.y >> remover >> ambient.z;
		}

		//check for camera
		else if (line.compare(0, camera_str.size(), camera_str) == 0)
		{
			//get the important info from the line
			std::istringstream info(line.substr(camera_str.size()));

			//update the ambient color intensity
			info >> remover >> mCamera.pos.x >> remover >> mCamera.pos.y >> remover >> mCamera.pos.z >> remover >>
				remover >> mCamera.target.x >> remover >> mCamera.target.y >> remover >> mCamera.target.z >> remover >>
				remover >> mCamera.up.x >> remover >> mCamera.up.y >> remover >> mCamera.up.z >> remover >> mCamera.focal;

			//force up vector to be perpendicular to view
			mCamera.up = glm::normalize(mCamera.up);
			mCamera.view = glm::normalize(mCamera.target - mCamera.pos);
			mCamera.right = glm::normalize(glm::cross(mCamera.view, mCamera.up));
			mCamera.up = glm::cross(mCamera.right, mCamera.view);
		}
	}
	
	scene_file.close();
	return true;
}

//render the scene tracing rays
void RayTracerClass::RenderScene()
{
	//check if we finished the algorithm
	if (current_samples == total_samples)
	{
		mbfinished = true;
		TakeScreenShot(screen_shoot_name);
	}

	//check if the raytracing finished
	if (!mbfinished)
	{
		//render depending on the mode
		if (mbUseThreads)
			RenderThread();
		else
			RenderNoThread();
		
		//check if all the pixels were processed then screen shot the final image and stop tracing rays
		if (pixels_processed >= pixels.size())
		{
			//wait for threads to end
			while (!ThreadPool.tasks_finished()) {
				//render current frame
				texture.update(image);
				sprite.setTexture(texture);
				window.draw(sprite);
				window.display();
			}
			current_samples++;
			pixels_processed = 0;
		}
	}

	//set title
	std::string title = "Current Samples ";
	title += std::to_string(current_samples + 1);
	window.setTitle(sf::String(title.c_str()));

	//render current frame
	texture.update(image);
	sprite.setTexture(texture);
	window.draw(sprite);
	window.display();
}

void RayTracerClass::RenderThread()
{
	//function that will the threads use
	auto f = [&](unsigned idx)
	{
		for (unsigned i = idx; i < cluster_size + idx; i++)
		{
			if (i >= pixels.size())
				break;
			glm::uvec2 pixel = pixels[i];
			//get random pixel and the ray to it
			ray mRay = GetRayToPixel(pixel);

			//compute the color of that pixel and conver it to sf::Color
			FrameBuffer[pixel.x + pixel.y * window_size.x] += TraceRayRecursive(mRay, bouncess);
			glm::vec3 color = FrameBuffer[pixel.x + pixel.y * window_size.x];
			color /= current_samples + 1;
			//color = glm::pow(color, glm::vec3(1.0f / 2.2f));
			color = glm::clamp(color, 0.0f, 1.0f);
			sf::Color pixel_color((unsigned)(color.x * 255.0f), (unsigned)(color.y * 255.0f), (unsigned)(color.z * 255.0f));

			//set the pixel color in the image
			image.setPixel(pixel.x, pixel.y, pixel_color);
		}
	};

	//iterate pushing work for the threads
	unsigned idx = 0;
	while (pixels_processed < pixels.size())
	{
		glm::uvec2 pixel = pixels[pixels_processed];
		ThreadPool.push(f, idx);
		idx += cluster_size;
		pixels_processed += cluster_size;
	}
}

void RayTracerClass::RenderNoThread()
{
	//iterate through a number of random pixels and process them
	for (unsigned i = 0; i < cluster_size; i++, pixels_processed++)
	{
		glm::uvec2 pixel = pixels[pixels_processed];
		ray mRay = GetRayToPixel(pixel);

		//compute the color of that pixel and conver it to sf::Color
		FrameBuffer[pixel.x + pixel.y * window_size.x] += TraceRayRecursive(mRay, bouncess);
		glm::vec3 color = FrameBuffer[pixel.x + pixel.y * window_size.x];
		color /= current_samples + 1;
		//color = glm::pow(color, glm::vec3(1.0f / 2.2f));
		color = glm::clamp(color, 0.0f, 1.0f);
		sf::Color pixel_color((unsigned)(color.x * 255.0f), (unsigned)(color.y * 255.0f), (unsigned)(color.z * 255.0f));

		//set the pixel color in the image
		image.setPixel(pixel.x, pixel.y, pixel_color);
	}
}

void RayTracerClass::DeleteScene()
{
	//wait for threads to end
	while (!ThreadPool.tasks_finished()) {}

	//delete threadpool
	ThreadPool.ShutDown();

	//delete the objects
	while (!objects.empty())
	{
		model* temp = objects.back();
		objects.pop_back();
		delete temp->mMaterial;
		delete temp;
	}

	delete[] FrameBuffer;
	//close the window
	window.close();
}

model* RayTracerClass::CheckRay(ray mRay, intersection_stats& stats)
{
	//initialize variables
	model* intersected_obj = nullptr;
	intersection_stats curr_stats;
	float t = FLT_MAX;

	//iterate through the objects of the screen
	for (auto* obj : objects)
	{
		//intersect ray with object
		float temp_t = obj->CheckRay(mRay, curr_stats);

		//check if not intersection
		if (temp_t < 0.0f) continue;

		//check if object is closer than previous one
		if (temp_t <= t)
		{
			stats = curr_stats;
			t = temp_t;
			intersected_obj = obj;
		}
	}
	//return the pointer to the closest object intersected
	return intersected_obj;
}

glm::vec3 RayTracerClass::TraceRayRecursive(ray mRay, unsigned current_bounces)
{
	//check if we end the number of bounces
	if (current_bounces <= 0) return ambient;

	//check what object the ray intersects
	intersection_stats stats;
	model* obj = CheckRay(mRay, stats);
	if (!obj) return ambient;

	//check if a light was reached
	if (obj->mbLight)
		return obj->color;
	else
	{
		//get bounced vector from the surface
		ray bounced_ray;
		if (!obj->mMaterial->GetBounceRay(mRay, bounced_ray, stats))
			return ambient;

		//keep the recursive until we run out of bounces or reach a light
		return obj->color * TraceRayRecursive(bounced_ray, --current_bounces);
	}
}

ray RayTracerClass::GetRayToPixel(glm::uvec2 pixel)
{
	//compute aspect ratio and half of the window
	float aspect_ratio = (float)window_size.x / (float)window_size.y;
	glm::vec2 half_window_size = glm::vec2((float)window_size.x / 2.0f, (float)window_size.y / 2.0f);

	//randomize pixel for antialiasing
	glm::vec2 pixel_random_pos = { (float)pixel.x + glm::linearRand(0.0f, 1.0f),
								   (float)pixel.y + glm::linearRand(0.0f, 1.0f) };

	//compute the ndc coordinate of the pixel
	glm::vec2 ndc_coord = glm::vec2((pixel_random_pos.x - half_window_size.x) / half_window_size.x,
									(-(pixel_random_pos.y - half_window_size.y)) / half_window_size.y);

	//compute the world pos and return the ray corresponding to that world position
	glm::vec3 world_pos = mCamera.pos + mCamera.focal * mCamera.view + (ndc_coord.x / 2.0f) * mCamera.right + (ndc_coord.y / (2.0f * aspect_ratio)) * mCamera.up;
	return ray{ mCamera.pos, glm::normalize(world_pos - mCamera.pos) };
}

void RayTracerClass::Restart()
{
	//wait for threads to end
	while (!ThreadPool.tasks_finished()) {}

	//reset all values
	mbfinished = false;
	pixels_processed = 0;
	for (unsigned y = 0; y < window_size.y; y++)
		for (unsigned x = 0; x < window_size.x; x++)
		{
			FrameBuffer[x + y * window_size.x] = glm::vec3(0.0f);
			image.setPixel(x, y, sf::Color::Black);
		}

	//render current frame so we have a black screen for a second to give feedback that we restarted
	texture.update(image);
	sprite.setTexture(texture);
	window.draw(sprite);
	window.display();

	//destroy all objects
	while (!objects.empty())
	{
		model* obj = objects.back();
		delete obj;
		objects.pop_back();
	}

	//load the scene again
	LoadScene(scene_name);
	current_samples = 0;
}