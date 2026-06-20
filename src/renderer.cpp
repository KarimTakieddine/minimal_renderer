#include <span>

#include <memory_cursor.hpp>
#include <memory_view.hpp>

#include "locations_descriptor.h"
#include "renderer.h"

namespace
{
    constexpr size_t ALLOCATOR_SIZE = 1 << 12;
}

namespace renderer
{
    void allocate(Allocator* allocator)
    {
        allocator->allocate(ALLOCATOR_SIZE);

        auto* locationsDescriptor = allocator->requestMemory<LocationsDescriptor>();

        locationsDescriptor->materialColorLocation = 1;
        locationsDescriptor->colorLocation = 2;
        locationsDescriptor->transformLocation = 3;
        locationsDescriptor->uvLocation = 4;
        locationsDescriptor->positionLocation = 9;
    }

    void render(const Allocator* allocator)
    {
        const auto* allocatorStart = allocator->peek();
        std::span<const std::byte, ALLOCATOR_SIZE> allocatorSpan(static_cast<const std::byte*>(allocatorStart), ALLOCATOR_SIZE);

        auto memoryCursor = MemoryCursor<16>();

        ConstMemoryView allocatorMemoryView(allocatorSpan);
        
        const auto locationsDescriptor =
            allocatorMemoryView.read_object<LocationsDescriptor>(memoryCursor.getOffset());
        memoryCursor.step<LocationsDescriptor>();
    }
}