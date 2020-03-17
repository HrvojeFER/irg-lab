#ifndef GRAPHICS_WINDOW_HPP
#define GRAPHICS_WINDOW_HPP


#include "pch.hpp"

#include "app.hpp"


namespace irglab
{
	class window
	{
	public:
		struct size
		{
			const short int width;
			const short int height;

			size(const short int& width, const short int& height) : width(width), height(height) {}
		};
		
		static inline const size default_initial_size = { 800, 600 };

		
		const std::string title;
		const size current_size;

		
		explicit window(std::string title, const size& initial_size = default_initial_size) :
			title(std::move(title)),
			current_size(initial_size),
			inner_(create_inner(title, initial_size), destroy_inner) {}


		void loop() const
		{
			while(!should_close())
			{
				glfwPollEvents();
			}
		}
		
		[[nodiscard]] bool should_close() const { return glfwWindowShouldClose(inner_.get()); }

		void close() const
		{
			glfwWindowShouldClose(inner_.get());
		}


	private:
		// Inner window
		std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow*)>> inner_;

		[[nodiscard]] static GLFWwindow* create_inner(
			const std::string& title,
			const size& initial_size = default_initial_size)
		{
#if !defined(NDEBUG)
			glfwSetErrorCallback(error_callback);
#endif

			glfwInit();

			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

			const auto window = glfwCreateWindow(
				initial_size.width,
				initial_size.height,
				title.c_str(),
				nullptr,
				nullptr);

			if (window == nullptr)
			{
				throw std::runtime_error("Failed to create window.");
			}

#if !defined(NDEBUG)
			std::cout << "Window created." << std::endl;
#endif

			return window;
		}

		void static destroy_inner(GLFWwindow* inner)
		{
			glfwDestroyWindow(inner);

			if (inner != nullptr)
			{
				throw std::runtime_error("Failed to destroy window.");
			}

#if !defined(NDEBUG)
			std::cout << "Window destroyed." << std::endl;
#endif

			glfwTerminate();
		}


#if !defined(NDEBUG)
		// ReSharper disable once CppParameterMayBeConst
		void static error_callback(int code, const char* description)
		{
			std::cerr << "GLFW error:" << std::endl;

			std::cerr << "\t" << "Code: " << code << std::endl;

			std::cerr << "\t" << "Description" << description << std::endl;
		}
#endif

		
		// Drawing surface
#if defined(GLFW_INCLUDE_VULKAN)
#if defined(GRAPHICS_APP_HPP)
		friend class app;
#endif

		std::optional<vk::UniqueSurfaceKHR> drawing_surface_;

		[[nodiscard]] static std::vector<std::string> get_required_extension_names()
		{
			unsigned int count;
			const auto name_char_array = glfwGetRequiredInstanceExtensions(&count);

			std::vector<std::string> result;
			result.resize(count);
			for (unsigned int i = 0 ; i < count ; ++i)
			{
				result[i] = *(name_char_array + i);
			}

			return result;
		}

		void create_drawing_surface(const vk::Instance& vulkan_instance)
		{
			VkSurfaceKHR temp = nullptr;
			glfwCreateWindowSurface(vulkan_instance, inner_.get(), nullptr, &temp);

			if (temp == nullptr)
			{
				throw std::runtime_error("Failed to create surface.");
			}

#if !defined(NDEBUG)
			std::cout << "Drawing surface created." << std::endl;
#endif

			drawing_surface_ = vk::UniqueSurfaceKHR(temp);
		}


		[[nodiscard]] constexpr bool has_drawing_surface() const noexcept
		{
			return drawing_surface_.has_value();
		}

		[[nodiscard]] vk::SurfaceKHR get_drawing_surface() const
		{
			return drawing_surface_.value().get();
		}
#endif
	};
}

#endif
