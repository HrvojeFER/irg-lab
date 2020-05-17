#ifndef CONVEX_POLYGON_APP_HPP
#define CONVEX_POLYGON_APP_HPP


#include "pch.hpp"


#include "app_base.hpp"


#include "direction.hpp"

#include "wireframe.hpp"

#include "convex_polygon.hpp"


namespace irglab
{
	struct convex_polygon_app final : app_base
	{
		explicit convex_polygon_app() : app_base("Drawing window") { }

	private:
        void pre_run() override
        {
            std::cout << "Welcome to the Convex Polygon Artisan Program (TM) " <<
                "(abbreviated C-PAP from now on and pronounced /'si:paep/, " <<
                "not to be confused with Continuous positive airway pressure), " <<
                "where I take your points and fill the joints! " <<
                "(To form a convex polygon, of course.)" << std::endl;
            std::cout << "If you don't know what a convex polygon is, " <<
                "you can read the wikipedia page about it at the following link: " << std::endl;
            std::cout << "https://en.wikipedia.org/wiki/Convex_polygon" << std::endl << std::endl;
            std::cout << "You don't HAVE to read it. Its up to you to decide on that." << std::endl <<
                std::endl;

            std::cout << "Press any key to continue onto the controls..." << std::endl << std::endl;
            std::cin.get();

            std::cout << "On to the controls, then!" << std::endl;
            std::cout << "\tTo set the points, " <<
                "click on the drawing window with your left mouse button." << std::endl;
            std::cout << "\tTo reset the points, hit space." << std::endl;
            std::cout << "WARNING: Do not LITERALLY HIT space! It is just a common expression." <<
                std::endl <<
                "Press it gently. C-PAP (TM) is not responsible for " <<
                "any damage to your hardware caused by being a barbarian." << std::endl;
            std::cout << "\tTo draw a convex polygon with the set points press enter." << std::endl;
            std::cout << "I just now realized that I could have said 'press space' " <<
                "instead of 'hit space', haha. I'm sorry for the confusing instructions. " <<
                std::endl <<
                "I hope your keyboard is fine, because if it isn't, you can't run " <<
                "C-PAP (TM) properly and I wouldn't want that to happen. :(" << std::endl;
            std::cout << "I will try to make a convex polygon " <<
                "with the highest number of the points you set as I possibly can." << std::endl;
            std::cout << "I will be at my best if you set the points in a circular motion." <<
                std::endl;
            std::cout << "\tTo check if a point is inside the drawn convex polygon, " <<
                "click on the drawing screen with the right mouse button." << std::endl;
            std::cout << "\tTo reset the convex polygon, press delete." << std::endl << std::endl;

            std::cout << "Press any key to indicate that you are sill alive and " <<
                "that you understood all of that in spite of " <<
                "the obnoxious tries at making you laugh..." <<
                std::endl << std::endl;
            std::cin.get();

            std::cout << "That's it! You are now a fully capable C-PAP(TM) user! " <<
                "I should tell the staff to make some kind of certificate for this." << std::endl;
            std::cout << "Have fun! :)" << std::endl << std::endl;

        	
            window_->on_mouse_button(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS,
                [&](const window::cursor_position& cursor_position)
                {
                    std::cout << "Storing point at " << 
                        "x: " << cursor_position.x << 
                        " y: " << cursor_position.y << std::endl << 
                        std::endl;
            	
                    cursor_positions_.push_back(cursor_position);
                });

            window_->on_key(GLFW_KEY_SPACE, GLFW_PRESS,
                [&]()
                {
                    std::cout << "Resetting stored points." << std::endl << std::endl;
            	
                    cursor_positions_.clear();
                });

            window_->on_key(GLFW_KEY_ENTER, GLFW_PRESS,
                [&]()
                {
                    std::cout << "Drawing convex polygon for the following points: " << std::endl;

                    for (const auto& cursor_position : cursor_positions_)
                    {
                        std::cout << "x: " << cursor_position.x << 
                            " y: " << cursor_position.y << std::endl;
                    }

            		if (cursor_positions_.size() > 10)
            		{
                        std::cout << "Whoa! That's a lot of points." << std::endl <<
                            "WARNING: C-PAP (TM) is not responsible " <<
                            "for any damage to your hardware due to overheating." << 
                            std::endl << std::endl;
            		}
            	
            		try
            		{
                        convex_polygon_ = get_convex_polygon_from_cursor_positions(cursor_positions_);

                        std::cout << "Drawing convex polygon:" << std::endl << 
                            convex_polygon_.value() << std::endl;

                        auto edges_for_drawing =
                            get_wires_for_drawing(convex_polygon_.value());

                        auto lines_for_drawing =
                            get_lines_for_drawing_from_convex_polygon(convex_polygon_.value());

                        lines_for_drawing.insert(
                            lines_for_drawing.end(),
                            edges_for_drawing.begin(),
                            edges_for_drawing.end());

            			artist_.set_wires_to_draw(lines_for_drawing);

                        cursor_positions_.clear();
                    }
            		catch (const std::invalid_argument& exception)
            		{
                        cursor_positions_.clear();
            			
                        std::cerr << "C-PAP(TM) ERROR: " << exception.what() << std::endl
                            << "Stored points have been reset." << std::endl << std::endl;
            		}
            		catch (const std::range_error& exception)
            		{
                        std::cerr << "C-PAP(TM) ERROR: " << exception.what() << std::endl
                            << "Set more points, please." << std::endl << std::endl;
            		}
                });

            window_->on_mouse_button(GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS,
                [&](const window::cursor_position& cursor_position)
                {
                    if (convex_polygon_.has_value())
                    {
                        if (to_homogeneous_coordinates(cursor_position) < convex_polygon_.value())
                        {
                            std::cout << "Point at x: " << cursor_position.x << 
                                " y: " << cursor_position.y <<
                                " is in the convex polygon." << std::endl << std::endl;
                        }
                        else
                        {
                            std::cout << "Point at x: " << cursor_position.x << 
                                " y: " << cursor_position.y <<
                                " is not in the convex polygon." << std::endl << std::endl;
                        }
                    }
                    else
                    {
                        std::cout << "Set a convex polygon first, please." << std::endl << std::endl;
                    }
                });

            window_->on_key(GLFW_KEY_DELETE, GLFW_PRESS,
                [&]()
                {
                    std::cout << "Resetting convex polygon." << std::endl << std::endl;
            	
                    convex_polygon_.reset();

                    artist_.set_wires_to_draw({});
                });
        }
		
        std::vector<window::cursor_position> cursor_positions_{ 0 };
        std::optional<two_dimensional::convex_polygon> convex_polygon_;

		[[nodiscard]] std::vector<artist::wire> get_lines_for_drawing_from_convex_polygon(
            const two_dimensional::convex_polygon& convex_polygon) const
		{
            const auto top =
                convex_polygon.get_vertex_on(irglab::top).y;
            const auto bottom =
                convex_polygon.get_vertex_on(irglab::bottom).y;

            const auto step = 2.0f / static_cast<float>(window_->query_extent().height);

            std::vector<artist::wire> lines_for_drawing{};

            for (
                auto y_coordinate = bottom;
                y_coordinate < top;
                y_coordinate = y_coordinate + step)  // NOLINT(cert-flp30-c)
            {
                const auto convex_polygon_edge_intersections = 
                    convex_polygon.get_edge_intersections_at_y(y_coordinate);

            	// also blends colors from red to blue (left to right)
                lines_for_drawing.push_back(
                    {
                        {
	                        two_dimensional::to_cartesian_coordinates(
                                convex_polygon_edge_intersections.left),
							{ 0.5f, 0.0f, 0.5f }
                        },
                        {
	                        two_dimensional::to_cartesian_coordinates(
                                convex_polygon_edge_intersections.right),
                            { 0.3f, 0.0f, 1.0f }
                        }
                    });
            }

            return lines_for_drawing;
		}

		[[nodiscard]] static std::vector<artist::wire> get_wires_for_drawing(
			const two_dimensional::convex_polygon& convex_polygon)
		{
            two_dimensional::owning_wireframe wireframe{};
            wireframe += convex_polygon;

            const auto wires = wireframe.wires();
            std::vector<artist::wire> artist_wires{ wires.size() };
            std::transform(
                wires.begin(), 
                wires.end(), 
                artist_wires.begin(),
                [](const two_dimensional::owning_wire& wire) -> artist::wire
                {
                    return
                    {
	                    {
	                        wire.begin(),
	                        {1.0f, 0.6f, 0.0f}
	                    },
                        {
                        	wire.end(),
							{ 0.6f, 0.0f, 1.0f }
                        }
                    };
                });

            return artist_wires;
		}
		
		[[nodiscard]] two_dimensional::convex_polygon get_convex_polygon_from_cursor_positions(
            const std::vector<window::cursor_position>& cursor_positions) const
		{
            std::vector<two_dimensional::point> points{ cursor_positions.size() };

            std::transform(
                cursor_positions.begin(),
                cursor_positions.end(),
                points.begin(),
                [&](const window::cursor_position& cursor_position)
                {
                    return to_homogeneous_coordinates(cursor_position);
                });

			return two_dimensional::convex_polygon{ points.begin(), points.end() };
		}

		[[nodiscard]] two_dimensional::homogeneous_coordinates to_homogeneous_coordinates(
            const window::cursor_position& cursor_position) const
		{
            return two_dimensional::to_homogeneous_coordinates(
                to_vulkan_friendly_coordinates(cursor_position));
		}

        [[nodiscard]] two_dimensional::cartesian_coordinates to_vulkan_friendly_coordinates(
            const window::cursor_position& cursor_position) const
        {
            const auto window_extent = window_->query_extent();

            const auto x = 2 * (cursor_position.x / window_extent.width) - 1;
            const auto y = 2 * (cursor_position.y / window_extent.height) - 1;

            const two_dimensional::cartesian_coordinates vulkan_friendly{ x, y };

            return vulkan_friendly;
        }
	};

}

#endif
