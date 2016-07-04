# Project-Vulkan

### Todo
- Enable textures
- Make it run on Linux
- More information in the window header 
- Textured/Colored in benchmark.txt
- Fix camera rotation in OpenGL
- Fix bug where more than 64*4*4*2 objects can't be rendered
- Test changing textures efficiency
- Add a GPU time counter
- Fix the shaders
- Allocating multiple descriptor sets with different textures
- Create bounding box function for models
- Render text
- Optimize Object::GetWorldMatrix()
- Optimize vkDeviceWaitIdle in VulkanApp::Render()

SubmitPostPresentMemoryBarrier causes device lost when more than 2k objects

****Make a test case that changes the Pipeline state for every object??
- Vulkans overhead for changing pipelines change to be very constant, no matter what states you change (shader, culling etc...)
- OpenGL on the otherhand is a lot faster when just changing simple things (culling, winding order etc.) but lacks a lot when changing shaders or fill -> wireframe
- However, even if Vulkan is faster you still want to do some kind of application optimization like grouping all objects with the same pipeline togheter to reduce pipeline swapping (which isn't the topic of this paper)
