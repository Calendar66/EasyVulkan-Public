/**
 * @file DataStructures.hpp
 * @brief Common data structures used throughout the EasyVulkan framework
 * @details This file contains reusable data structures for graphics programming,
 *          including vectors, matrices, and other geometric primitives.
 */

#pragma once

#include "Common.hpp"
#include <cmath>
#include <array>

namespace ev {

/* -------------------------------------------------------------------------- */
/*                               Basic data type                              */
/* -------------------------------------------------------------------------- */
/**
 * @struct Vec2
 * @brief 2D vector structure for positions, texcoords, etc.
 */
template<typename T>
struct Vec2 {
    T x, y;

    Vec2() : x(0), y(0) {}
    Vec2(T x, T y) : x(x), y(y) {}

    // Basic vector operations
    Vec2<T> operator+(const Vec2<T>& other) const { return Vec2<T>(x + other.x, y + other.y); }
    Vec2<T> operator-(const Vec2<T>& other) const { return Vec2<T>(x - other.x, y - other.y); }
    Vec2<T> operator*(T scalar) const { return Vec2<T>(x * scalar, y * scalar); }
    
    // Dot product
    T dot(const Vec2<T>& other) const { return x * other.x + y * other.y; }
    
    // Length calculations
    T lengthSquared() const { return x * x + y * y; }
    T length() const { return std::sqrt(lengthSquared()); }
    
    // Normalization
    Vec2<T> normalized() const {
        T len = length();
        return len > 0 ? Vec2<T>(x / len, y / len) : Vec2<T>();
    }
};

/**
 * @struct Vec3
 * @brief 3D vector structure for positions, normals, colors, etc.
 */
template<typename T>
struct Vec3 {
    T x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(T x, T y, T z) : x(x), y(y), z(z) {}

    // Basic vector operations
    Vec3<T> operator+(const Vec3<T>& other) const { return Vec3<T>(x + other.x, y + other.y, z + other.z); }
    Vec3<T> operator-(const Vec3<T>& other) const { return Vec3<T>(x - other.x, y - other.y, z - other.z); }
    Vec3<T> operator*(T scalar) const { return Vec3<T>(x * scalar, y * scalar, z * scalar); }
    
    // Dot product
    T dot(const Vec3<T>& other) const { return x * other.x + y * other.y + z * other.z; }
    
    // Cross product
    Vec3<T> cross(const Vec3<T>& other) const {
        return Vec3<T>(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }
    
    // Length calculations
    T lengthSquared() const { return x * x + y * y + z * z; }
    T length() const { return std::sqrt(lengthSquared()); }
    
    // Normalization
    Vec3<T> normalized() const {
        T len = length();
        return len > 0 ? Vec3<T>(x / len, y / len, z / len) : Vec3<T>();
    }
};

/**
 * @struct Vec4
 * @brief 4D vector structure for homogeneous coordinates, colors with alpha, etc.
 */
template<typename T>
struct Vec4 {
    T x, y, z, w;

    Vec4() : x(0), y(0), z(0), w(0) {}
    Vec4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
    Vec4(const Vec3<T>& v, T w = 1) : x(v.x), y(v.y), z(v.z), w(w) {}

    // Basic vector operations
    Vec4<T> operator+(const Vec4<T>& other) const { return Vec4<T>(x + other.x, y + other.y, z + other.z, w + other.w); }
    Vec4<T> operator-(const Vec4<T>& other) const { return Vec4<T>(x - other.x, y - other.y, z - other.z, w - other.w); }
    Vec4<T> operator*(T scalar) const { return Vec4<T>(x * scalar, y * scalar, z * scalar, w * scalar); }
    
    // Dot product
    T dot(const Vec4<T>& other) const { return x * other.x + y * other.y + z * other.z + w * other.w; }
};

/* -------------------------------------------------------------------------- */
/*                               Vertex struct                                */
/* -------------------------------------------------------------------------- */
/**
 * @struct Vertex
 * @brief Basic vertex structure for 3D rendering
 */
struct Vertex {
    Vec3<float> position;  ///< Vertex position in 3D space
    Vec3<float> normal;    ///< Vertex normal for lighting calculations
    Vec2<float> texCoord;  ///< Texture coordinates
    Vec4<float> color;     ///< Vertex color (RGBA)

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};
        
        // Position
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, position);
        
        // Normal
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, normal);
        
        // TexCoord
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);
        
        // Color
        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, color);
        
        return attributeDescriptions;
    }
};

/**
 * @struct SimpleVertex
 * @brief Basic vertex structure for 2D rendering
 */
struct SimpleVertex {
    Vec2<float> position;  ///< Vertex position in
    Vec3<float> color;  ///< Vertex color
    Vec2<float> texCoord;  ///< Texture coordinates

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(SimpleVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }
    
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);
        
        // Position
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(SimpleVertex, position);
        
        // Color
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(SimpleVertex, color);

        // TexCoord
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(SimpleVertex, texCoord);
        
        return attributeDescriptions;
    }
};


/* -------------------------------------------------------------------------- */
/*                              Other common data                             */
/* -------------------------------------------------------------------------- */
/**
 * @brief Structure to track command buffer and its pool
 */
struct CommandBufferInfo {
    VkCommandBuffer commandBuffer; ///< Command buffer handle
    VkCommandPool commandPool;     ///< Associated command pool
};

/**
 * @brief Structure to track pipeline and its layout
 */

struct PipelineInfo {
    VkPipeline pipeline; ///< Pipeline handle
    VkPipelineLayout pipelineLayout; ///< Pipeline layout handle
};

/**
 * @brief Structure to image and its view
 */

struct ImageInfo {
    VkImage image; ///< Image handle
    VkImageView imageView; ///< Image view handle
    VmaAllocation allocation; ///< Allocation handle
    uint32_t width; ///< Width of the image
    uint32_t height; ///< Height of the image
    VkImageLayout layout; ///< Layout of the image
};



// Common type aliases for convenience
using Vec2f = Vec2<float>;
using Vec2d = Vec2<double>;
using Vec2i = Vec2<int>;
using Vec2u = Vec2<unsigned int>;

using Vec3f = Vec3<float>;
using Vec3d = Vec3<double>;
using Vec3i = Vec3<int>;
using Vec3u = Vec3<unsigned int>;

using Vec4f = Vec4<float>;
using Vec4d = Vec4<double>;
using Vec4i = Vec4<int>;
using Vec4u = Vec4<unsigned int>;

} // namespace ev 