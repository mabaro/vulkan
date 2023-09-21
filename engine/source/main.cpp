#include <algorithm>
#include <iostream>
#include <vector>

// kk dummy implementations
struct VkBuffer { };
struct VkDeviceMemory { };
struct VkDeviceSize { };
struct VkBufferUsageFlags { };

namespace gfx {
////////////////////////////////////////////////////////////////////////////////

struct Handle {
    uint32_t handle;
    bool     operator==(const Handle& other) const
    {
        return other.handle == handle;
    }
};

struct BufferHandle {
    Handle handle;

    bool operator==(const BufferHandle& other) const
    {
        return other.handle == handle;
    }
};

namespace vulkan {

struct Buffer {
    VkBuffer           vkBuffer;
    VkDeviceMemory     vkDeviceMemory;
    VkDeviceSize       vkDeviceSize;
    VkBufferUsageFlags vkUsageFlags;
    uint32_t           size;
    uint32_t           globalOffset;
    BufferHandle       handle;
    BufferHandle       parentHandle;
    const char*        name = nullptr;
};

}   // namespace vulkan{

enum class BufferUsageFlags { NONE = 0, READ, WRITE, COPY };

struct BufferCreation {
    uint32_t         size       = 0;
    BufferUsageFlags usageFlags = BufferUsageFlags::NONE;
    const char*      name       = nullptr;

    BufferCreation& setSize(uint32_t size)
    {
        size = size;
        return *this;
    }
    BufferCreation& setName(const char* name)
    {
        name = name;
        return *this;
    }
    BufferCreation& setUsageFlags(BufferUsageFlags usageFlags)
    {
        usageFlags = usageFlags;
        return *this;
    }
};

class GfxDevice {
    struct BufferData {
        BufferHandle   handle;
        vulkan::Buffer buffer;
    };

public:
    BufferHandle CreateBuffer(const BufferCreation& /* info */)
    {
        const uint32_t handleIndex = _bufferHandles.size();
        _bufferHandles.push_back(
            BufferData {.handle = BufferHandle {.handle = handleIndex},
                .buffer         = vulkan::Buffer {}});
        return _bufferHandles.back().handle;
    }

    void DestroyBuffer(BufferHandle&& buffer)
    {
        std::find_if(_bufferHandles.begin(), _bufferHandles.end(),
            [&buffer](const BufferData& bufferIt) {
                return bufferIt.handle.handle == buffer.handle;
            });
    }

private:
    std::vector<BufferData> _bufferHandles;
};

void
TestAbstraction()
{
    BufferCreation bufferInfo {.size = 1024,
        .usageFlags = BufferUsageFlags::NONE,
        .name = "Test buffer"};
    bufferInfo.setName("Test buffer").setSize(1024);
}

////////////////////////////////////////////////////////////////////////////////
}   // namespace gfx {

int
main(int /* argc */, char** /* argv */)
{

    return 0;
}
