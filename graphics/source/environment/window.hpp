#ifndef IRGLAB_WINDOW_HPP
#define IRGLAB_WINDOW_HPP


#include "external/external.hpp"

#include "environment.hpp"


namespace il
{
    class window
    {
    public:
        using resize_callback [[maybe_unused]] = std::function<void(vk::Extent2D)>;
        using key_callback [[maybe_unused]] = std::function<void()>;
        struct [[maybe_unused]] cursor_position
        {
            double x;
            double y;
        };
        using mouse_button_callback [[maybe_unused]] = std::function<void(cursor_position)>;


        static constexpr std::string_view default_title{"Irglab"};
        static constexpr vk::Extent2D default_initial_size{600, 600};

        [[nodiscard, maybe_unused]] explicit window(
                const environment &environment,
                const std::string_view title = default_title,
                const vk::Extent2D initial_extent = default_initial_size) :
                title{title},
                _inner{_create_inner(title, initial_extent), _destroy_inner},
                _drawing_surface{_create_drawing_surface(environment)}
        {
            glfwSetWindowUserPointer(_inner.get(), reinterpret_cast<void *>(this));
            glfwSetWindowSizeCallback(_inner.get(), _glfw_resize_callback);
            glfwSetKeyCallback(_inner.get(), _glfw_key_callback);
            glfwSetMouseButtonCallback(_inner.get(), _glfw_mouse_button_callback);

#if !defined(NDEBUG)
            std::cout << std::endl << "-- window done --" << std::endl << std::endl;
#endif
        }

        const std::string title;

        [[nodiscard, maybe_unused]] const vk::SurfaceKHR &drawing_surface() const noexcept
        {
            return *_drawing_surface;
        }

        [[nodiscard, maybe_unused]] vk::Extent2D query_extent() const
        {
            int width, height;
            glfwGetFramebufferSize(_inner.get(), &width, &height);

            return
                    {
                            static_cast<unsigned int>(width),
                            static_cast<unsigned int>(height)
                    };
        }


        [[maybe_unused]] void show() const
        {
            glfwShowWindow(_inner.get());
        }

        [[nodiscard, maybe_unused]] bool should_close() const
        { return glfwWindowShouldClose(_inner.get()); }

        [[maybe_unused]] static void process_events()
        {
            glfwPollEvents();
        }


        [[maybe_unused]] static void wait_events()
        {
            glfwWaitEvents();
        }

        [[maybe_unused]] void on_resize(const resize_callback &callback)
        {
            _resize_callbacks.emplace_back(callback);
        }

        [[maybe_unused]] void on_key(const int key, const int action, const key_callback &callback)
        {
            _key_callbacks[key][action].emplace_back(callback);
        }

        [[maybe_unused]] void on_key_oneshot(const int key, const int action, const key_callback &callback)
        {
            _oneshot_key_callbacks[key][action].emplace_back(callback);
        }

        [[maybe_unused]] void on_mouse_button(const int button, const int action, const mouse_button_callback &callback)
        {
            _mouse_button_callbacks[button][action].emplace_back(callback);
        }

        [[maybe_unused]] void close() const
        {
            glfwSetWindowShouldClose(_inner.get(), GLFW_TRUE);
        }


    private:
        static void _glfw_resize_callback(GLFWwindow *window, int width, int height)
        {
            const auto self = reinterpret_cast<il::window *>(
                    glfwGetWindowUserPointer(window));

            for (const auto &resize_callback : self->_resize_callbacks)
            {
                resize_callback(
                        {
                                static_cast<unsigned int>(width),
                                static_cast<unsigned int>(height)
                        });
            }
        }

        static void _glfw_key_callback(
                GLFWwindow *window,
                int key,
                [[maybe_unused]] int scan_code,
                int action,
                [[maybe_unused]] int mods)
        {
            auto self = reinterpret_cast<il::window *>(
                    glfwGetWindowUserPointer(window));

            for (const auto &key_callback : self->_key_callbacks[key][action])
            {
                key_callback();
            }

            for (const auto &oneshot_key_callback :
                    self->_oneshot_key_callbacks[key][action])
            {
                oneshot_key_callback();
            }

            self->_oneshot_key_callbacks.clear();
        }

        static void _glfw_mouse_button_callback(GLFWwindow *window, int button, int action, [[maybe_unused]] int mods)
        {
            cursor_position cursor_position{ };
            glfwGetCursorPos(window, &cursor_position.x, &cursor_position.y);

            {
                auto self = reinterpret_cast<il::window *>(glfwGetWindowUserPointer(window));
                for (const auto &mouse_button_callback : self->_mouse_button_callbacks[button][action])
                {
                    mouse_button_callback(cursor_position);
                }
            }
        }

        const std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow *)>> _inner;
        const vk::UniqueSurfaceKHR _drawing_surface;

        std::vector<resize_callback> _resize_callbacks{ };

        template<typename CallbackType>
        using user_input_callback_map =
        std::unordered_map<int, std::unordered_map<int, std::vector<CallbackType>>>;
        user_input_callback_map<key_callback> _key_callbacks{ };
        user_input_callback_map<key_callback> _oneshot_key_callbacks{ };
        user_input_callback_map<mouse_button_callback> _mouse_button_callbacks{ };


        [[nodiscard]] static GLFWwindow *_create_inner(
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
            std::cout << "window handle created" << std::endl;
#endif

            return window;
        }

        void static _destroy_inner(GLFWwindow *inner)
        {
            glfwDestroyWindow(inner);
        }

        [[nodiscard]] vk::UniqueSurfaceKHR _create_drawing_surface(const environment &environment) const
        {
            VkSurfaceKHR temp = nullptr;
            const auto glfw_result =
                    glfwCreateWindowSurface(
                            environment.vulkan_instance(),
                            _inner.get(),
                            nullptr,
                            &temp);

            if (glfw_result != VK_SUCCESS || temp == nullptr)
            {
                throw std::runtime_error("Failed to create drawing_surface.");
            }
            auto result = vk::UniqueSurfaceKHR{temp, environment.vulkan_instance()};

#if !defined(NDEBUG)
            std::cout << "Drawing surface created" << std::endl;
#endif
            return result;
        }
    };
}

#endif
