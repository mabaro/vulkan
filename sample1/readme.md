# Vulkan projects devlog

<a id="markdown-header" name="header" depthFrom:1 depthTo:3 updateOnSave:true orderedList:true></a>
<!-- TOC -->

- [Vulkan projects devlog](#vulkan-projects-devlog)
    - [Plan and current state](#plan-and-current-state)
        - [Abstraction](#abstraction)
            - [GpuDevice](#gpudevice)
            - [Buffer](#buffer)
            - [Formats](#formats)
                - [Mesh](#mesh)
    - [Libraries and tools used](#libraries-and-tools-used)
        - [Math stuff](#math-stuff)
        - [SDL](#sdl)
        - [Vulkan](#vulkan)
        - [VMA vulkan memory allocator](#vma-vulkan-memory-allocator)
        - [Visual Studio Code](#visual-studio-code)
            - [Key accelerators](#key-accelerators)
                - [Plugins](#plugins)
        - [Markdown](#markdown)
            - [VSCode plugins](#vscode-plugins)

<!-- /TOC -->
## Plan and current state

Implement trivial vec/matrix stuff
#define assertuintcompare( a, comp, b )				core_assert_macro( AssertType::Assert, (a)comp( b ), "%s " #comp " %s\n\t%u, %u", #a, #b, static_cast<uint>( a ), static_cast<uint>( b ) )

1. Implement first test version from Vulkan tutorial
    1. Here -> <https://vulkan-tutorial.com/Vertex_buffers/Vertex_input_description>
1. Integrate IMGUI: <https://vkguide.dev/docs/extra-chapter/implementing_imgui/> DONE!
1. Integrate ABC library (profiling, timers, lent_ptr...)
1. Integrate [libshaderc](https://github.com/google/shaderc/blob/main/libshaderc/README.md) in order to be able to compile the shaders from the application itself
1. Integrate [DXC](https://hub.docker.com/r/gwihlidal/dxc/)
1. [Abstraction](#abstraction)
  1. GpuDevice
  2. [Buffer](#buffer) - data (heterogeneous or homogeneous)
  3. Texture - images to read/write from
  4. Samplers - define how the shader retrieves data from textures/buffers
  5. Shaders - define how data is processed
  6. Pipeline - defines the configuration of the pipeline (i.e., shader, textures/buffers,...)
1. Memory allocators

### Abstraction

#### GpuDevice

```
struct GpuDevice {
    BufferHandle CreateBuffer(const BufferCreation& info);
    ...
    void DestroyBuffer(BufferHandle&& buffer);
};
```

#### Buffer
```
struct Buffer {
  BufferHandle handle;
  u32 size;
  ...
};
```

#### Formats
##### Mesh
- Mode1
    - 8 byte position stream (i.e., world matrix + normalized(local_vec) * a + local_vec)
        - RGBA16 (snorm [-1..+1] model_matrix = mesh_scale_matrix * model_matrix
    - 12 byte properties stream
        - RGB10A2 -> normal (+bitangent sign)
            - or XY normal | Z material_id | A bitangent sign
        - RGB10A2 tangent(+2bit material id) -> 4 material per object
            - or RG tangent | BA material_id
        - [optional] RG16 UVs 16bit UNORM UVs [0..1] -> using per-model scale factor [-4..+4] 8k textures
- Mode2
    - 16 byte position stream
        - RGBA16 pos
        - RGBA8 bone index  (4 bones per vert)
        - RGBA8 bone weight (4 bones per vert)
    - 12 byte properties stream
        - RGB10A2 normal (+bitangent sign)
        - RGB10A2 tangent (+2 bit material_id)
        - RG16 UV
## Libraries and tools used

### Math stuff
https://github.com/BlackMATov/vmath.hpp/blob/main/headers/vmath.hpp/vmath_vec_fun.hpp

### SDL
- [SDL2](https://gist.github.com/YukiSnowy/dc31f47448ac61dd6aedee18b5d53858) is gonna be used for cross-platform:
  - input
  - surface creation

### Vulkan
- Vulkan tutorials
    - [Vulkan tutorial](https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/Swap_chain)
  - [Vulkan guide](https://vkguide.dev)

### VMA (vulkan memory allocator)
https://gpuopen.com/vulkan-memory-allocator

### Visual Studio Code

#### Key accelerators
control+shift+O -> search symbol in current file
control+T -> search symbol in current workspace
control+P -> # workspace @ file

##### Plugins

### Markdown
#### VSCode plugins
- [Markdown TOC generator](https://marketplace.visualstudio.com/items?itemName=huntertran.auto-markdown-toc)
