#include <iostream>
#include <vector>
#include <cmath> 
#include <fstream>
#include <omp.h>
#include <limits> 
#include <fstream>
#include <cstdio>
#include <string>
#include "Vec3.hpp"
#include "Sphere.hpp"

int maxRayDepth = 5;

const float infinity = std::numeric_limits<float>::max(); 

#define STB_IMAGE_IMPLEMENTATION
#include "stb-master/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb-master/stb_image_write.h"

Vec3f ambientLight = 0.2;


const Vec3f backCol = Vec3f(0, 0, 0); 

int canvaSizeX = 640;
int canvaSizeY = 480;

void render(const std::vector<Sphere> spheres, int threads);

bool loadScene(std::vector<Sphere>& spheres)
{
	std::ifstream file;
	
	file.open ("scene.txt", std::ios::in);
	if(!file.is_open()) {
		return false;
	}
	std::string line;
	int lineNR = 0;
	Vec3<float> pos, color, emission;
	float radius, reflectivity;
	while(std::getline(file, line)) {	
		lineNR++;
		if(line.size() > 1 && line[0] == line[1] && line[1] == '/')
			continue;
		if(11 == sscanf(line.c_str(), "%f %f %f %f %f %f %f %f %f %f %f", 
			&pos.x, &pos.y, &pos.z, &radius, &color.x, &color.y, &color.z, &reflectivity,
			&emission.x, &emission.y, &emission.z)) {
				spheres.push_back(Sphere(pos, radius, color, reflectivity, emission));
			}
		else if(8 == sscanf(line.c_str(), "%f %f %f %f %f %f %f %f", 
			&pos.x, &pos.y, &pos.z, &radius, &color.x, &color.y, &color.z, &reflectivity)) {
				spheres.push_back(Sphere(pos, radius, color, reflectivity));
			}
		else
			std::cout << "scene, line " << lineNR << ": invalid data" << std::endl;
			
	}
	
	file.close();
	return true;
}

bool loadSettings() {
	std::ifstream file;
	
	file.open ("settings.txt", std::ios::in);
	if(!file.is_open()) {
		return false;
	}
	std::string line;
	int lineNR = 0;
	Vec3<float> pos, color, emission;
	char temp[50];
	float radius, reflectivity;
	if(std::getline(file, line)) {	
		lineNR++;
		if(4 != sscanf(line.c_str(), "%s %f %f %f", temp, &ambientLight.x, &ambientLight.y, &ambientLight.z))
			std::cout << "settings, line " << lineNR << ": invalid data" << std::endl;
	}
	
	if(std::getline(file, line)) {	
		lineNR++;
		if(3 != sscanf(line.c_str(), "%s %d %d", temp, &canvaSizeX, &canvaSizeY))
			std::cout << "settings, line " << lineNR << ": invalid data" << std::endl;
	}
	
	
	if(std::getline(file, line)) {	
		lineNR++;
		if(2 != sscanf(line.c_str(), "%s %d", temp, &maxRayDepth))
			std::cout << "settings, line " << lineNR << ": invalid data" << std::endl;
	}
	
	file.close();
	return true;
}

int main()
{
	int threads = omp_get_max_threads();
	
	std::vector<Sphere> spheres;
	if(!loadScene(spheres)) {
		std::cout << "failed to load scene" << std::endl;
		return 1;
	}
	
	if(!loadSettings()) {
		std::cout << "failed to load settings" << std::endl;
		return 1;
	}
		
	double start = omp_get_wtime();
	render(spheres, threads); 
	
	double time = omp_get_wtime() - start;
	printf("rendering time: %lf s\n", time);
		
	return 0;
}

Vec3f trace(const Vec3f &rayorig, const Vec3f &raydir, const std::vector<Sphere> &spheres, const int &depth);
 
void render(const std::vector<Sphere> spheres, int threads) 
{ 
    Vec3f* image = new Vec3f[canvaSizeX * canvaSizeY];
    float invWidth = 1 / float(canvaSizeX);
	float invHeight = 1 / float(canvaSizeY); 
    float fov = 30;
	float aspectratio = canvaSizeX / float(canvaSizeY); 
    float angle = tan(M_PI * 0.5 * fov / 180.); 
    
	#pragma omp parallel for schedule(dynamic) num_threads(threads)
	for (unsigned y = 0; y < canvaSizeY; ++y) {
		Vec3f* pixel = image;
		pixel += y*canvaSizeX;
		for (unsigned x = 0; x < canvaSizeX; ++x, ++pixel) { 
			float x1 = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio; 
			float y1 = (1 - 2 * ((y + 0.5) * invHeight)) * angle; 
			Vec3f raydir(x1, y1, 1.0f); 
			raydir.normalize();
			*pixel = trace(Vec3f(0), raydir, spheres, 0); 
		} 
	} 
	
	//prepare data for png file
    unsigned char* img2 = new unsigned char[canvaSizeX * canvaSizeY * 4]; 
	unsigned int j;
	float max = 0;
    for (unsigned int i = 0, j = 0; i < canvaSizeX * canvaSizeY; ++i, j+=4) { 
		max = 0;
		if(image[i].x > max)
			max = image[i].x;
		if(image[i].y > max)
			max = image[i].y;
		if(image[i].z > max)
			max = image[i].z;
		if(max > 1)
		{
			image[i].x /= max;
			image[i].y /= max;
			image[i].z /= max;
		}
        img2[j] = (unsigned char)(std::min(float(1), image[i].x) * 255);
        img2[j + 1] = (unsigned char)(std::min(float(1), image[i].y) * 255);
        img2[j + 2] = (unsigned char)(std::min(float(1), image[i].z) * 255); 
        img2[j + 3] = (unsigned char)(255); 
    } 
	//save to png file
	stbi_write_png("image.png", canvaSizeX, canvaSizeY, 4, img2, canvaSizeX * 4);
	
	//release memory
    delete[] image; 
	delete[] img2;
} 

Vec3f trace(const Vec3f &rayorig, const Vec3f &raydir, const std::vector<Sphere> &spheres, const int &depth)  {

	float tnear = infinity; 
	int index = -1;
    const Sphere* sphere = nullptr;  //closest sphere
	
    //finding closest sphere
    for (unsigned i = 0; i < spheres.size(); ++i) { 
        float t0 = infinity, t1 = infinity; 
        if (spheres[i].intersect(rayorig, raydir, t0, t1)) { 
            if (t0 < 0) t0 = t1; 
            if (t0 < tnear) { 
                tnear = t0; 
                sphere = &spheres[i]; 
				index = i;
            } 
        } 
    }
	
    // no sphere hit, return background color
    if (sphere == nullptr) 
		return backCol;
	
	Vec3f surfaceColor = 0;
    Vec3f hitPoint = rayorig + raydir * tnear; // intersection point
    Vec3f hitNormal = hitPoint - sphere->center;
    hitNormal.normalize();
    float bias = 0.001; // bias to counter self intersection
	
	Vec3f reflection = 0;
	float reflectionParam = 0;
	if(sphere->reflection > 0 && depth < maxRayDepth) { //calculate reflection vector
		reflectionParam = sphere->reflection; 
        Vec3f reflectionDir = raydir - hitNormal * 2 * raydir.dot(hitNormal); 
        reflectionDir.normalize(); 
        reflection = trace(hitPoint + hitNormal * bias, reflectionDir, spheres, depth + 1); 
	}
	else {
		for (unsigned i = 0; i < spheres.size(); ++i) { //calculate diffuse light
			if(index != i && spheres[i].emissionColor.length2() > 0) { 
                float lightPower = 1; 
				Vec3f lightDirection = spheres[i].center - hitPoint; 
				float distToLight = lightDirection.length2();
                lightDirection.normalize(); 
				
				float lt0, lt1;
				spheres[i].intersect(hitPoint + hitNormal * bias, lightDirection, lt0, lt1); //distance to light
                for (unsigned j = 0; j < spheres.size(); ++j) { //check if in shadow
                    if (i != j) { 
						float t0, t1; 
                        if (spheres[j].intersect(hitPoint + hitNormal * bias, lightDirection, t0, t1)) { 
							if(t0 < lt0) { //if there is intersection closer than light 
								lightPower = 0; 
								break; 
							}
                        } 
                    }
                }
				float lightAngle = std::max(float(0), lightDirection.dot(hitNormal));
				
				surfaceColor += sphere->surfaceColor * lightPower * lightAngle * spheres[i].emissionColor * (1.0f/distToLight); 
			}
		}
	}
	
	float k = 1.0f - reflectionParam;
	return ambientLight*sphere->surfaceColor + sphere->emissionColor + reflection * reflectionParam + k*surfaceColor;
}