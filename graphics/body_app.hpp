#ifndef GRAPHICS_BODY_APP
#define GRAPHICS_BODY_APP


#include "pch.hpp"

#include "app_base.hpp"
#include "assets.hpp"

#include "wireframe.hpp"
#include "convex_body.hpp"
#include "scene.hpp"


namespace irglab
{
	struct body_app final : app_base
	{
		explicit body_app(const std::string& path_to_object_file = "./objects/tetrahedron.obj") :
			app_base("Body")
		{
			const auto body_parsing_report =
				three_dimensional::convex_body::parse(read_object_file(path_to_object_file));
			std::cout << "Body parsing report:" << std::endl <<
				body_parsing_report;

			auto body = body_parsing_report.body;

			const auto bound_fit_report = body &= vulkan_friendly_limit;
			std::cout << "Body fit report: " << std::endl <<
				bound_fit_report;

			wireframe_ += body;
			wireframe_.remove_duplicate_wires();
			wireframe_.normalize();
			std::cout << "Wireframe:" << std::endl << 
				wireframe_ << std::endl;
		}

	private:
		static inline const auto vulkan_friendly_limit = 0.5f;

		three_dimensional::scene scene_{};
		three_dimensional::wireframe wireframe_{};

		void pre_run() override
		{
			set_body_for_drawing();
			window_.show();
		}
		
		void set_body_for_drawing() const
		{
			const auto points = wireframe_.get_points();
			const auto view_transformation = scene_.get_view_transformation();

			std::vector<vertex> vertices{ points.size() };
			std::transform(points.begin(), points.end(), vertices.begin(),
				[&] (const three_dimensional::point& point) -> vertex
				{
					const auto transformation_result = 
						scene_.get_perspective_projection(point) * (view_transformation * point);

					return
					{
						{
							transformation_result.x / transformation_result.w,
							transformation_result.y / transformation_result.w
						},
						{1.0f, 0.6f, 0.0f}
					};
				});

			artist_.set_vertices_to_draw(vertices);
		}
	};
}

#endif
