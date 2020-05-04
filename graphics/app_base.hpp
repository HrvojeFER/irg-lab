#ifndef GRAPHICS_APP_BASE_HPP
#define GRAPHICS_APP_BASE_HPP


#include "pch.hpp"

#include "environment.hpp"
#include "window.hpp"
#include "artist.hpp"


namespace irglab
{
    struct app_base
    {
        static inline const std::string_view default_name = "Graphics App";
        const std::string_view name;

        explicit app_base(const std::string_view name = default_name) :
            window_{ environment_, name },
            artist_{ environment_, window_ } { }

        virtual ~app_base() = default;
        app_base(app_base&) = delete;
        app_base(app_base&&) = delete;
        app_base& operator =(app_base&) = delete;
        app_base& operator =(app_base&&) = delete;

        void run()&&
        {
            pre_run();

            window_.show();
        	
            while (!window_.should_close())
            {
                loop();
            }

            artist_.wait_idle();
        }

    private:
        environment environment_;
    	
    protected:
        virtual void pre_run() { }

    	virtual void loop()
        {
            window_.process_events();

            artist_.draw_frame();
        }

        window window_;

        artist artist_;
    };
}

#endif
