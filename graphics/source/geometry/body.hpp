#ifndef IRGLAB_BODY_HPP
#define IRGLAB_BODY_HPP


#include "external/external.hpp"


#include "primitive/primitive.hpp"

#include "triangle.hpp"


namespace il
{
    // Type traits

    [[nodiscard, maybe_unused]] constexpr bool is_body_description_supported(
            small_natural_number dimension_count, [[maybe_unused]] vertex_access_type access_type)
    {
        return are_primitive_operations_supported(dimension_count);
    }


    // Declarations

    template<small_natural_number DimensionCount, vertex_access_type AccessType>
    class [[maybe_unused]] body;


    template<small_natural_number DimensionCount, vertex_access_type AccessType>
    class [[maybe_unused]] convex_body;


    // Aliases

    template<small_natural_number DimensionCount>
    using owning_body [[maybe_unused]] = body<DimensionCount, vertex_access_type::owned>;

    template<small_natural_number DimensionCount>
    using sharing_body [[maybe_unused]] = body<DimensionCount, vertex_access_type::shared>;

    template<small_natural_number DimensionCount>
    using tracking_body [[maybe_unused]] = body<DimensionCount, vertex_access_type::tracked>;


    template<small_natural_number DimensionCount>
    using owning_convex_body [[maybe_unused]] = convex_body<DimensionCount, vertex_access_type::owned>;

    template<small_natural_number DimensionCount>
    using sharing_convex_body [[maybe_unused]] = convex_body<DimensionCount, vertex_access_type::shared>;

    template<small_natural_number DimensionCount>
    using tracking_convex_body [[maybe_unused]] = convex_body<DimensionCount, vertex_access_type::tracked>;


    // Dimensional aliases

    namespace d2
    {
        template<vertex_access_type AccessType>
        using body [[maybe_unused]] = il::body<dimension_count, AccessType>;

        template<vertex_access_type AccessType>
        using convex_body [[maybe_unused]] = il::convex_body<dimension_count, AccessType>;

        using owning_body [[maybe_unused]] = body<vertex_access_type::owned>;
        using sharing_body [[maybe_unused]] = body<vertex_access_type::shared>;
        using tracking_body [[maybe_unused]] = body<vertex_access_type::tracked>;

        using owning_convex_body [[maybe_unused]] = convex_body<vertex_access_type::owned>;
        using sharing_convex_body [[maybe_unused]] = convex_body<vertex_access_type::shared>;
        using tracking_convex_body [[maybe_unused]] = convex_body<vertex_access_type::tracked>;
    }

    namespace d3
    {
        template<vertex_access_type AccessType>
        using body [[maybe_unused]] = il::body<dimension_count, AccessType>;

        template<vertex_access_type AccessType>
        using convex_body [[maybe_unused]] = il::convex_body<dimension_count, AccessType>;

        using owning_body [[maybe_unused]] = body<vertex_access_type::owned>;
        using sharing_body [[maybe_unused]] = body<vertex_access_type::shared>;
        using tracking_body [[maybe_unused]] = body<vertex_access_type::tracked>;

        using owning_convex_body [[maybe_unused]] = convex_body<vertex_access_type::owned>;
        using sharing_convex_body [[maybe_unused]] = convex_body<vertex_access_type::shared>;
        using tracking_convex_body [[maybe_unused]] = convex_body<vertex_access_type::tracked>;
    }


    // Owning Implementation

    template<small_natural_number DimensionCount>
    class [[maybe_unused]] body<DimensionCount, vertex_access_type::owned>
    {
        // Traits and types

    public:
        [[maybe_unused]] static constexpr small_natural_number dimension_count = DimensionCount;
        [[maybe_unused]] static constexpr vertex_access_type access_type = vertex_access_type::owned;

        [[maybe_unused]] static constexpr bool is_tracking = false;
        [[maybe_unused]] static constexpr bool is_owning = true;
        [[maybe_unused]] static constexpr bool is_virtual = !is_owning;


        using triangle [[maybe_unused]] = il::triangle<dimension_count, access_type>;

        using owning_triangle [[maybe_unused]] = il::owning_triangle<dimension_count>;
        using sharing_triangle [[maybe_unused]] = il::sharing_triangle<dimension_count>;
        using tracking_triangle [[maybe_unused]] = il::tracking_triangle<dimension_count>;

        using virtual_triangle [[maybe_unused]] = il


        using owned_vertex [[maybe_unused]] = il::owned_vertex<dimension_count>;
        using shared_vertex [[maybe_unused]] = il::shared_vertex<dimension_count>;
        using tracked_vertex [[maybe_unused]] = il::tracked_vertex<dimension_count, triangle>;

        using virtual_vertex [[maybe_unused]] = il::virtual_vertex<dimension_count, triangle>;

        using vertex [[maybe_unused]] = typename triangle::vertex;
    };


    // Tracking

    template<small_natural_number DimensionCount>
    class [[maybe_unused]] body<DimensionCount, true>
    {
        // Traits and types

    public:
        [[maybe_unused]] static constexpr small_natural_number dimension_count = DimensionCount;
        [[maybe_unused]] static constexpr vertex_access_type access_type = vertex_access_type::owned;

        [[maybe_unused]] static constexpr bool is_tracking = false;
        [[maybe_unused]] static constexpr bool is_owning = true;
        [[maybe_unused]] static constexpr bool is_virtual = !is_owning;

        using triangle [[maybe_unused]] = il::triangle<dimension_count, access_type>;
        using vertex [[maybe_unused]] = typename triangle::vertex;


        // Constructors and related methods

        template<
                typename Dummy = void, std::enable_if_t<
                        std::is_same_v<Dummy, void> &&
                        is_body_description_supported(dimension_count, is_tracking), int> = 0>
        [[nodiscard, maybe_unused]] explicit body(
                const std::initializer_list<tracked_vertex>& vertices,
                const std::initializer_list<shared_triangle>& triangles) noexcept:

                _vertices{vertices},
                _triangles{triangles}
        { }

        template<
                typename VertexBeginIterator, typename VertexEndIterator,
                typename TriangleBeginIterator, typename TriangleEndIterator, std::enable_if_t<
                        is_triangle_description_supported(dimension_count, is_tracking) &&
                        are_vertex_iterators<
                                dimension_count, is_tracking, VertexBeginIterator, VertexEndIterator> &&
                        are_triangle_iterators<
                                TriangleBeginIterator, TriangleEndIterator, dimension_count, is_tracking>,
                        int> = 0>
        [[nodiscard, maybe_unused]] explicit body(
                const VertexBeginIterator& vertices_begin, const VertexEndIterator& vertices_end,
                const TriangleBeginIterator& triangles_begin, const TriangleEndIterator& triangles_end) :

                _vertices{vertices_begin, vertices_end},
                _triangles{triangles_begin, triangles_end}
        { }

        template<
                typename VertexRange, typename TriangleRange, std::enable_if_t<
                        is_triangle_description_supported(dimension_count, is_tracking) &&
                        is_vertex_range<VertexRange, dimension_count, is_tracking> &&
                        is_triangle_range<TriangleRange, dimension_count, is_tracking>, int> = 0>
        [[nodiscard, maybe_unused]]  explicit body(
                const VertexRange& vertices, const TriangleRange& triangles) :

                _vertices{vertices.begin(), vertices.end()},
                _triangles{triangles.begin(), triangles.end()}
        { }


        template<
                typename Dummy = void, std::enable_if_t<
                        std::is_same_v<Dummy, void> && is_tracking,
                        int> = 0>
        [[maybe_unused]] void prune() const
        {
            for (const auto& vertex : _vertices) vertex.prune();
        }


        // Accessors

        [[nodiscard, maybe_unused]] const std::unordered_set<tracked_vertex>& vertices() const
        {
            return _vertices;
        }

        [[nodiscard, maybe_unused]] const std::unordered_set<shared_triangle>& triangles() const
        {
            return _triangles;
        }


        // Non-Modifiers

        [[maybe_unused]] friend constexpr void operator|=(
                bounds<dimension_count>& bounds, const body& body) noexcept
        {
            for (const auto& triangle : body._triangles) bounds |= *triangle;
        }

        [[nodiscard, maybe_unused]] friend constexpr bounds<dimension_count>& operator|(
                bounds <dimension_count>& old_bounds, const body& body) noexcept
        {
            bounds < dimension_count > new_bounds{old_bounds};
            new_bounds |= body;
            return new_bounds;
        }


        [[maybe_unused]] friend void operator+=(
                il::tracking_wireframe<dimension_count>& wireframe,
                const body& body)
        {
            for (const auto& triangle : body._triangles) wireframe += *triangle;
        }

        [[nodiscard, maybe_unused]] friend il::tracking_wireframe<dimension_count>& operator+(
                il::tracking_wireframe<dimension_count>& old_wireframe,
                const body& body)
        {
            il::tracking_wireframe<dimension_count> new_wireframe{old_wireframe};
            new_wireframe |= body;
            return new_wireframe;
        }


        [[nodiscard, maybe_unused]] friend std::ostream& operator<<(
                std::ostream& output_stream, const body& body)
        {
            output_stream << "Triangles:" << std::endl;

            for (const auto& triangle : body._triangles)
            {
                output_stream << *triangle;
            }

            return output_stream << std::endl;
        }

        [[maybe_unused]] constexpr void normalize() const
        {
            for (auto& vertex : _vertices) il::normalize<DimensionCount>(*vertex);
        }

        [[maybe_unused]] constexpr void operator*=(
                const transformation<dimension_count>& transformation) const noexcept
        {
            for (auto& vertex : _vertices) *vertex = *vertex * transformation;
        }


        // 3D

        template<
                typename Dummy = void, std::enable_if_t<
                        std::is_same_v<Dummy, void> && dimension_count == d3::dimension_count,
                        int> = 0>
        [[nodiscard, maybe_unused]] static body<3, true> parse(
                const std::vector<std::string>& lines)
        {
            std::vector<tracked_vertex> vertices;
            std::vector<shared_triangle> triangles;

            for (const auto& line : lines)
            {
                std::istringstream line_stream{line};

                char first;
                line_stream >> first;

                if (first == '#')
                {
                    continue;
                }

                if (first == 'v')
                {
                    float x, y, z;
                    line_stream >> x >> y >> z;

                    vertices.emplace_back(tracked_vertex{std::make_shared<owned_vertex>(x, y, z, 1.0f)});
                } else if (first == 'f')
                {
                    size_t first_index, second_index, third_index;
                    line_stream >> first_index >> second_index >> third_index;

                    // Numbering in .obj files starts with 1.

                    const auto& first_vertex = vertices[first_index - 1];
                    const auto& second_vertex = vertices[second_index - 1];
                    const auto& third_vertex = vertices[third_index - 1];

                    const auto shared_triangle = std::make_shared<triangle>(
                            first_vertex, second_vertex, third_vertex);

                    first_vertex += shared_triangle;
                    second_vertex += shared_triangle;
                    third_vertex += shared_triangle;

                    triangles.emplace_back(shared_triangle);
                }
            }

            return body
                    {
                            vertices.begin(), vertices.end(),
                            triangles.begin(), triangles.end()
                    };
        }


        template<
                typename Dummy = void, std::enable_if_t<
                        std::is_same_v<Dummy, void> && dimension_count == d3::dimension_count,
                        int> = 0>
        [[maybe_unused]] void operator&=(const d3::bounds& bounds) noexcept
        {
            static_assert(dimension_count == 3);

            d3::bounds current_bounds{ };
            current_bounds |= *this;

            const auto translation_difference =
                    bounds.get_center() - current_bounds.get_center();
            const auto&& translation =
                    d3::get_translation(
                            translation_difference.x,
                            translation_difference.y,
                            translation_difference.z);

            const auto scaling_factors =
                    bounds.get_difference() / current_bounds.get_difference();
            const auto min_scaling_factor =
                    scaling_factors.x < scaling_factors.y ?
                    (scaling_factors.x < scaling_factors.z ? scaling_factors.x : scaling_factors.y) :
                    scaling_factors.y < scaling_factors.z ? scaling_factors.y : scaling_factors.z;

            const auto&& scale_transformation =
                    d3::get_scale_transformation(min_scaling_factor);

            *this *= translation * scale_transformation;
        }

        template<
                typename Dummy = void, std::enable_if_t<
                        std::is_same_v<Dummy, void> && dimension_count == 3,
                        int> = 0>
        void operator&=(const float limit) noexcept
        {
            return *this &= _get_normalization_bounds(limit);
        }

    private:
        template<
                typename Dummy = void, std::enable_if_t<
                        std::is_same_v<Dummy, void> && dimension_count == 3,
                        int> = 0>
        [[nodiscard]] static d3::bounds _get_normalization_bounds(
                const rational_number limit) noexcept
        {
            return d3::bounds
                    {
                            -limit,
                            limit,

                            -limit,
                            limit,

                            -limit,
                            limit
                    };
        }


        // Data

    private:
        std::unordered_set<tracked_vertex> _vertices;
        std::unordered_set<shared_triangle> _triangles;
    };


    // Convex

    template<small_natural_number DimensionCount, vertex_access_type AccessType>
    class [[maybe_unused]] convex_body : public body<DimensionCount, AccessType>
    {
        [[nodiscard]] friend bool operator<(const d3::point& point, const convex_body& body)
        {
            return std::all_of(
                    body._triangles.begin(), body._triangles.end(),
                    [point](const auto& triangle)
                    { return point < *triangle; });
        }
    };


    // Parsing operator

    [[nodiscard, maybe_unused]] inline tracking_body<3> operator ""_tracking_body(
            const char* chars, natural_number)
    {
        std::istringstream char_stream{chars};

        std::vector<std::string> result;
        for (std::string line ; std::getline(char_stream, line) ;)
        {
            // Check if empty or contains only whitespace
            if (!line.empty() &&
                line.find_first_not_of(' ') != std::string::npos &&
                line.find_first_not_of('\t') != std::string::npos)
            {
                trim(line);
                result.emplace_back(line);
            }
        }

        return tracking_body<3>::parse(result);
    }

    // Dimensional alias

    namespace d3
    {
        [[nodiscard, maybe_unused]] inline tracking_body operator ""_body(
                const char* chars, natural_number passed)
        {
            return il::operator ""_tracking_body(chars, passed);
        }
    }
}

#endif
