#include <SFML/Graphics.hpp>
#include "Raytracer.h"

int main(int argc, char ** argv)
{
    //initialize the ray tracer
    int WIDTH  = 500;
    int HEIGHT = 500;
    
    std::string inputFile = "A3_Box.txt";
    std::string screenshotName = "screenshot.png";
    bool        takeScreenshot = false;

    if (argc > 1)
        inputFile = argv[1];
    if (argc > 2)
    {
        screenshotName = argv[2];
        takeScreenshot = true;
        RayTracer.screen_shoot_name = screenshotName;
    }
    if (argc > 4)
    {
        WIDTH = std::atoi(argv[3]);
        HEIGHT = std::atoi(argv[4]);
    }

    //initialize raytracer
    RayTracer.Initialize(WIDTH, HEIGHT);

    //load the scene
    RayTracer.LoadScene(inputFile);

    //get the window
    sf::RenderWindow& window = RayTracer.get_window();

    // gameloop
    while (window.isOpen())
    {
        RayTracer.RenderScene();

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::F1))
            takeScreenshot = true;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
            RayTracer.Restart();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::T))
            RayTracer.mbUseThreads = !RayTracer.mbUseThreads;

        if (takeScreenshot)
        {
            RayTracer.TakeScreenShot(screenshotName);
            takeScreenshot = false;
        }

        // Handle input
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                RayTracer.DeleteScene();
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
            RayTracer.DeleteScene();
    }
    return 0;
}