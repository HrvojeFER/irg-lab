#ifndef GRAPHICS_APP_HPP
#define GRAPHICS_APP_HPP


#include "pch.hpp"

#include "environment.hpp"
#include "window.hpp"
#include "artist.hpp"


namespace irglab
{
	class app
	{
	public:
        static inline const std::string_view default_name = "Graphics App";
        const std::string_view name;

        explicit app(const std::string_view name = default_name) :
			window_{ environment_ },
            artist_{ environment_, window_ }
        {
            window_.show();
        }

        void run() &&
		{
            while(!window_.should_close())
            {
                window_.process_events();

                artist_.draw_frame();
            }

            artist_.wait_idle();
        }
		
	private:
        environment environment_;
		
        window window_;

        artist artist_;

	};
}

#endif
