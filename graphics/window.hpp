#ifndef GRAPHICS_WINDOW_HPP
#define GRAPHICS_WINDOW_HPP


#include "pch.hpp"

#include "environment.hpp"


namespace irglab
{
	struct window
	{
		static inline const std::string_view default_title{ "Window" };
		static inline const vk::Extent2D default_initial_size{ 800, 600 };

		const std::string_view title;
		
		explicit window(
			const environment& environment,
			const std::string_view title = default_title,
			const vk::Extent2D initial_extent = default_initial_size) : 

			title(title),
			inner_(create_inner(title, initial_extent), destroy_inner),
			drawing_surface_(create_drawing_surface(environment))
		{
			glfwSetWindowUserPointer(inner_.get(), reinterpret_cast<void*>(this));

#if !defined(NDEBUG)
			std::cout << std::endl << "-- Window done --" << std::endl << std::endl;
#endif
		}


		[[nodiscard]] const vk::SurfaceKHR& drawing_surface() const noexcept
		{
			return *drawing_surface_;
		}

		[[nodiscard]] vk::Extent2D query_extent() const
		{
			int width, height;
			glfwGetFramebufferSize(inner_.get(), &width, &height);

			return
			{
				static_cast<unsigned int>(width),
				static_cast<unsigned int>(height)
			};
		}
		

		void show() const
		{
			glfwShowWindow(inner_.get());
		}

		[[nodiscard]] bool should_close() const { return glfwWindowShouldClose(inner_.get()); }

		// ReSharper disable once CppMemberFunctionMayBeStatic
		void process_events() const
		{
			glfwPollEvents();
		}

		// ReSharper disable once CppMemberFunctionMayBeStatic
		void wait_events() const
		{
			glfwWaitEvents();
		}

		void on_resize(const std::function<void(vk::Extent2D)>& callback)
		{
			resize_callbacks_.push_back(callback);
		}
		
		void close() const
		{
			glfwSetWindowShouldClose(inner_.get(), GLFW_TRUE);
		}


	private:
		std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow*)>> inner_;
		const vk::UniqueSurfaceKHR drawing_surface_;

		std::vector<std::function<void(vk::Extent2D)>> resize_callbacks_{};


		[[nodiscard]] static GLFWwindow* create_inner(
			const std::string_view title,
			const vk::Extent2D initial_size = default_initial_size)
		{
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
			glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

			const auto window = glfwCreateWindow(
				initial_size.width,
				initial_size.height,
				title.data(),
				nullptr,
				nullptr);

			if (window == nullptr)
			{
				throw std::runtime_error("Failed to create window.");
			}

#if !defined(NDEBUG)
			std::cout << "Window handle created" << std::endl;
#endif

			return window;
		}

		void static destroy_inner(GLFWwindow* inner)
		{
			glfwDestroyWindow(inner);
		}

		[[nodiscard]] vk::UniqueSurfaceKHR create_drawing_surface(
			const environment& environment) const
		{
			VkSurfaceKHR temp = nullptr;
			const auto glfw_result =
				glfwCreateWindowSurface(
					environment.vulkan_instance(), 
					inner_.get(),
					nullptr,
					&temp);

			if (glfw_result != VK_SUCCESS || temp == nullptr)
			{
				throw std::runtime_error("Failed to create drawing_surface.");
			}
			auto result = vk::UniqueSurfaceKHR{ temp, environment.vulkan_instance() };

#if !defined(NDEBUG)
			std::cout << "Drawing surface created" << std::endl;
#endif
			return result;
		}

		// ReSharper disable twice CppParameterMayBeConst
		static void resize_callback(GLFWwindow* window, int width, int height)
		{
			const irglab::window* this_ = reinterpret_cast<irglab::window*>(
				glfwGetWindowUserPointer(window));

			for (const auto& resize_callback : this_->resize_callbacks_)
			{
				resize_callback(
					{
						static_cast<unsigned int>(width),
						static_cast<unsigned int>(height)
					});
			}
		}
	};
}

#endif
