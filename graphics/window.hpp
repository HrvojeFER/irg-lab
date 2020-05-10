#ifndef GRAPHICS_WINDOW_HPP
#define GRAPHICS_WINDOW_HPP


#include "pch.hpp"

#include "environment.hpp"


namespace irglab
{
	struct window
	{
		static inline const std::string_view default_title{ "Window" };
		static inline const vk::Extent2D default_initial_size{ 600, 600 };

		const std::string_view title;

		using resize_callback = std::function<void(vk::Extent2D)>;
		using key_callback = std::function<void()>;
		struct cursor_position
		{
			double x;
			double y;
		};
		using mouse_button_callback = std::function<void(cursor_position)>;
		
		explicit window(
			const environment& environment,
			const std::string_view title = default_title,
			const vk::Extent2D initial_extent = default_initial_size) : 

			title(title),
			inner_(create_inner(title, initial_extent), destroy_inner),
			drawing_surface_(create_drawing_surface(environment))
		{
			glfwSetWindowUserPointer(inner_.get(), reinterpret_cast<void*>(this));
			glfwSetWindowSizeCallback(inner_.get(), glfw_resize_callback);
			glfwSetKeyCallback(inner_.get(), glfw_key_callback);
			glfwSetMouseButtonCallback(inner_.get(), glfw_mouse_button_callback);

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

		void on_resize(const resize_callback& callback)
		{
			resize_callbacks_.emplace_back(callback);
		}

		void on_key(const int key, const int action, const key_callback& callback)
		{
			key_callbacks_[key][action].emplace_back(callback);
		}

		void on_key_oneshot(const int key, const int action, const key_callback& callback)
		{
			oneshot_key_callbacks_[key][action].emplace_back(callback);
		}
		
		void on_mouse_button(const int button, const int action, const mouse_button_callback& callback)
		{
			mouse_button_callbacks_[button][action].emplace_back(callback);
		}

		void close() const
		{
			glfwSetWindowShouldClose(inner_.get(), GLFW_TRUE);
		}

	private:
		const std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow*)>> inner_;
		const vk::UniqueSurfaceKHR drawing_surface_;

		std::vector<resize_callback> resize_callbacks_{};

		template<typename CallbackType>
		using user_input_callback_map =
			std::unordered_map<int, std::unordered_map<int, std::vector<CallbackType>>>;
		user_input_callback_map<key_callback> key_callbacks_{};
		user_input_callback_map<key_callback> oneshot_key_callbacks_{};
		user_input_callback_map<mouse_button_callback> mouse_button_callbacks_{};


		[[nodiscard]] static GLFWwindow* create_inner(
			const std::string_view title,
			const vk::Extent2D initial_size = default_initial_size)
		{
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
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

		// ReSharper disable CppParameterMayBeConst
		// ReSharper disable CppParameterNeverUsed
		static void glfw_resize_callback(GLFWwindow* window, int width, int height)
		{
			const auto this_ = reinterpret_cast<irglab::window*>(
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

		static void glfw_key_callback(GLFWwindow* window, int key, int scan_code, int action, int mods)
		{
			auto this_ = reinterpret_cast<irglab::window*>(
				glfwGetWindowUserPointer(window));

			for (const auto& key_callback : this_->key_callbacks_[key][action])
			{
				key_callback();
			}

			for (const auto& oneshot_key_callback : 
				this_->oneshot_key_callbacks_[key][action])
			{
				oneshot_key_callback();
			}

			this_->oneshot_key_callbacks_.clear();
		}

		static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
		{
			auto this_ = reinterpret_cast<irglab::window*>(
				glfwGetWindowUserPointer(window));

			cursor_position cursor_position{};
			glfwGetCursorPos(window, &cursor_position.x, &cursor_position.y);

			const auto cursor_position_const{ cursor_position };

			for (const auto& mouse_button_callback : 
				this_->mouse_button_callbacks_[button][action])
			{
				mouse_button_callback(cursor_position_const);
			}
		}
	};
}

#endif
