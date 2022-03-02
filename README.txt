/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
Project: cs500_arenas.f_ 2
Author: iñigo fernandez, arenas.f
Creation date: 10/02/2022
----------------------------------------------------------------------------------------------------------*/

* HOW TO USE THE PROGRAM?
	There are default values for the program, but for a custom scene, screen shot and dimensions it needs to follow the following 
	convention in the command line : "scene_name.txt" "screenshotname.png" "width" "height"
	obviusly without the "".
	The user only needs to run the program, the user can take an screen shot presing F1, and reload the image be pressing R
	He can change different values in the config.txt

* IMPORTANT PARTS OF CODE

	-RayTracer.cpp : the full implementation of the ray tracer
	-Model.cpp     : contain different shapes with the corresponding intersection with ray for the raytracer

* KNOWN ISSUES OR PROBLEMS: