#ifndef GRAPHICS_APP_BASE_HPP
#define GRAPHICS_APP_BASE_HPP


#include "../external/pch.hpp"

#include "../env/Environment.hpp"
#include "../env/window.hpp"
#include "../renderer/renderer.hpp"


namespace il
{
    struct app_base
    {
        static constexpr std::string_view default_name = "Graphics App";
    	
        explicit app_base(const std::string_view name = default_name) :
            window_{ std::make_shared<window>(environment_, name) },
            artist_{ environment_, window_ } { }

        const std::string_view name;

    	
        virtual ~app_base() = default;
        app_base(app_base&) = delete;
        app_base(app_base&&) = delete;
        app_base& operator =(app_base&) = delete;
        app_base& operator =(app_base&&) = delete;

    	
        void run()&&
        {
            pre_run();

            window_->show();
        	
            while (!window_->should_close())
            {
                loop();
            }

            artist_.wait_idle();
        }

    	
    private:
        environment environment_{};

    	
    protected:
        virtual void pre_run() { }

        virtual void loop()
        {
            window_->process_events();

            artist_.draw_frame();
        }


        std::shared_ptr<window> window_;

        artist artist_;
    };
}

#endif
