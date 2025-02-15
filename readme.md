<img align="center" padding="2" src="https://raw.githubusercontent.com/PanosK92/SpartanEngine/master/data/textures/banner.bmp"/>

<p>Spartan is a research-focused game engine designed for real-time solutions, providing a dynamic experience. Not intended for game development yet, it serves as a valuable study resource due to its clean, modern, and high-quality architecture.</p>

- <img align="left" width="32" src="https://clipart.info/images/ccovers/1534043159circle-twitter-logo-png.png"/>For occasional updates regarding the project's development, you can follow me on <a href="https://twitter.com/panoskarabelas1?ref_src=twsrc%5Etfw">twitter</a>.
  
- <img align="left" width="32" height="32" src="https://www.freeiconspng.com/thumbs/discord-icon/discord-icon-7.png">For questions, suggestions, help and any kind of general discussion join the [discord server](https://discord.gg/TG5r2BS).
  
- <img align="left" width="32" height="32" src="https://www.freeiconspng.com/uploads/git-github-hub-icon-25.png">For issues and anything directly related to the project, feel free to open an issue.
  
- <img align="left" width="32" height="32" src="https://opensource.org/sites/default/files/public/osi_keyhole_300X300_90ppi_0.png">Adhering to the <a href="https://en.wikipedia.org/wiki/MIT_License">MIT license</a> is appreciated. This means that you can copy all the code you want as long as you include a copy of the original license.

### Status
![build_status](https://github.com/PanosK92/SpartanEngine/actions/workflows/workflow.yml/badge.svg)
[![Discord](https://img.shields.io/discord/677302405263785986?label=Discord)](https://discord.gg/TG5r2BS)

# Livestreams

Occasional livestreams on Discord for interesting topics.

[![](https://raw.githubusercontent.com/PanosK92/SpartanEngine/master/assets/github/readme_1.4.jpg)](https://www.youtube.com/watch?v=QhyMal6RY7M)

## Media
[![](https://i.imgur.com/j6zIEI9.jpg)](https://www.youtube.com/watch?v=RIae1ma_DSo)
<img align="center" src="https://raw.githubusercontent.com/PanosK92/SpartanEngine/master/assets/github/readme_1.5.jpg"/>


<img align="center" src="https://raw.githubusercontent.com/PanosK92/SpartanEngine/master/assets/github//readme_1.1.jpg"/>|<img align="center" src="https://raw.githubusercontent.com/PanosK92/SpartanEngine/master/assets/github/readme_1.2.jpg"/>
:- | :-

## Features (v0.31)
- 10+ font file formats support (FreeType)
- 20+ audio file formats support (FMOD)
- 30+ image file formats support (FreeImage)
- 40+ model file formats support (Assimp)
- Vulkan and DirectX 11 backends (same HLSL shaders compile everywhere)
- Deferred rendering with transparency (under a single render path and using the same shaders)
- Principled BSDF supporting anisotropic, clearcoat and cloth materials (combined with things like normal mapping, parallax, masking, occlusion etc)
- Bloom (Based on a study of Resident Evil 2's RE Engine)
- Volumetric lighting
- Depth of Field
- Lights with physical units (lux for directional, candelas for point and spot lights)
- Shadows with penumbra and colored translucency (Cascaded and omnidirectional shadow mapping with Vogel filtering)
- SSAO (Screen space ambient occlusion). Can be extented to SSGI (Screen space global illumination)
- SSR (Screen space reflections)
- SSS (Screen space shadows)
- TAA (Temporal anti-aliasing)
- Physically based camera (Aperture, Shutter Speed, ISO)
- Depth of field (controlled by the aperture of the camera)
- Motion blur (controlled by the shutter speed of the camera)
- Real-time shader editor
- On the fly mip generation on the GPU, using a single dispatch.
- Font rendering
- Frustum culling
- Physics (Rigid bodies, Constraints, Colliders)
- Entity-component system
- Event system
- Mouse & keyboard input as well as controller support (tested with a PS5 controller)
- Debug rendering (Transform gizmo, scene grid, bounding boxes, colliders, raycasts, g-buffer visualization etc)
- Thread pool
- Engine rendered platform agnostic editor
- Profiling (CPU & GPU)
- Support for XML files
- Easy to build (Single click project generation which includes editor and runtime)
- AMD FidelityFX Contrast Adaptive Sharpening
- AMD FidelityFX Single Pass Downsampler
- AMD FidelityFX Super Resolution 2
- AMD Compressonator for texture compression

# Roadmap

## Currently

Feature            | Completion | Notes
:-                 | :-         | :-
Vulkan polishing       | 98%      | Outperform D3D11 in all cases and improve stability.
DirectX 12        | 10%   | The rendering API has matured thanks to Vulkan, finishing with DX12 should be easy.
Ray traced shadows      | -          | Low priority, first I get Vulkan to be as stable as possible.
Ray traced reflections     | -          | Low priority, first I get Vulkan to be as stable as possible.
Eye Adaptation        | -          | Low priority.
Subsurface Scattering      | -          | Low priority.
Linux support             | -          | Vulkan and SDL is there, working on a linux port is now possible.

## Future

- Skeletal Animation.
- Atmospheric Scattering.
- Dynamic resolution scaling.
- Export on Windows.
- UI components.
- Make editor more stylish.
- Scripting.
