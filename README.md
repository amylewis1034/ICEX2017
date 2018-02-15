# ICEX 2017 Underwater Visualization

## Code Overview
The project uses an entity component style engine. Each GameObject is essentially a collection of Components which define the behavior of the GameObject. For example, the GameObject that represents a controllable character might contain a Transform component to store orientation and position, a Camera component that handles providing relevant view transformations during rendering, and a PlayerInput component so that the player can be controlled via mouse and keyboard.

The World class holds all of the GameObjects and handles initializing and updating all GameObjects. It also contains the render loop, which gets relevant camera information and then hands off rendering the scene to a Renderer.

To make it easier to create the GameObjects and add them to the World, json configuration files can be parsed and loaded. The syntax is simple and involves defining all the desired GameObjects along with their Components. Check out world.json for a basic example. By default, the executable will load world.json. If command line arguments are passed they will all be interpreted as configuration files to load. The paths passed are expected to be relevant to the resources directory.

To work with PRM paths, one can attach both a PlayerInput and a PRMInput to the player GameObject and then use the 'p' key to toggle between the two. To generate the path, use the renderthirds.json file (Make sure the scene is the same. Also maybe disable postprocessing step?).

### References
* [Learn OpenGL](https://learnopengl.com/)
* [Unity Engine](https://unity3d.com/)
* Kyle Piddington's [MoonEngine](https://github.com/kyle-piddington/MoonEngine)
* [Game Programming Patterns](http://gameprogrammingpatterns.com/contents.html)
* [Evolve Your Hierarchy from Cowboy Programming](http://cowboyprogramming.com/2007/01/05/evolve-your-heirachy/)
* [The Book of Shaders](https://thebookofshaders.com/)
* [Component Based Engine Design from Randy Gaul](http://www.randygaul.net/2013/05/20/component-based-engine-design/)
* [Fix Your Timestep!](http://gafferongames.com/game-physics/fix-your-timestep/)
* [OGLdev Modern OpenGL Tutorials](http://ogldev.atspace.co.uk)
* [John Chapman's Motion Blur Tutorial](http://john-chapman-graphics.blogspot.com/2013/01/what-is-motion-blur-motion-pictures-are.html)
* [Quick GUI for MBED projects using Dear ImGui and Serial](http://justanotherelectronicsblog.com/?p=179)
* [Terrain Generation with a Heightmap](http://www.chadvernon.com/blog/resources/directx9/terrain-generation-with-a-heightmap/)
* [Real-Time Rendering](http://www.realtimerendering.com/)
* Fog and Atmospheric Scattering [1](http://in2gpu.com/2014/07/22/create-fog-shader/)


### Third Party Libraries
* `tiny_obj_loader.h` from [syoyo](https://github.com/syoyo/tinyobjloader)
* `stb_image.h` from [nothings](https://github.com/nothings/stb)
* [OpenGL Mathematics Library (GLM)](http://glm.g-truc.net/0.9.8/index.html)
* [RapidJSON](https://github.com/miloyip/rapidjson) from [miloyip](https://github.com/miloyip)
* [GLFW](http://www.glfw.org/)
* [GLEW](http://glew.sourceforge.net/)
* [ImGui](https://github.com/ocornut/imgui)
