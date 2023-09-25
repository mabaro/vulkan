# Vulkan projects devlog

<a id="markdown-header" name="header" depthFrom:1 depthTo:3 updateOnSave:true orderedList:true></a>
<!-- TOC -->

- [Vulkan projects devlog](#vulkan-projects-devlog)
    - [Plan and current state](#plan-and-current-state)
        - [Abstraction](#abstraction)
            - [GpuDevice](#gpudevice)
            - [Buffer](#buffer)
    - [Libraries and tools used](#libraries-and-tools-used)
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

1. Implement first test version from Vulkan tutorial
    1. Here -> <https://vulkan-tutorial.com/en/Drawing_a_triangle/Graphics_pipeline_basics/Fixed_functions>
1. Integrate IMGUI: <https://vkguide.dev/docs/extra-chapter/implementing_imgui/>
1. Integrate [libshaderc](https://github.com/google/shaderc/blob/main/libshaderc/README.md) in order to be able to compile the shaders from the application itself
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

## Libraries and tools used

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
