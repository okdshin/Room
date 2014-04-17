#ifdef ROOM_UNIT_TEST
#include "Room.h"
#include <iostream>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <savage/shader.h>
#include <savage/context.h>
#include <savage/instance.h>
#include <savage/time.h>
#include <savage/window_size.h>
#include <savage/cursor_position.h>
#include <savage/model.h>

#define GLSL(version, shader) "#version " #version "\n" #shader
const char* vert = GLSL
( 
    150,
    in vec3 VertexPosition;
	in vec3 VertexColor;

	out vec3 Color;

	uniform mat4 ViewProjectionMatrix;
	uniform mat4 ModelMatrix;
    void main()
    {
		Color = VertexColor;
        gl_Position = ViewProjectionMatrix * ModelMatrix * vec4(VertexPosition, 1.0);
    }

);
const char* geometry = GLSL
( 
    150,
	in vec3 Color;
	out vec3 
	void main()
	{
		Color = 
	}
);

const char* frag = GLSL
( 
    150,
	in vec3 Color;
    out vec4 FragColor;
    void main()
    {
        //FragColor = vec4(Color.xy, 1.0, 1.0);
		FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    }

);

typedef savage::basic_context<glm::vec2, glm::vec2> context;

class Room {
public:
	void Initialize() {
		using namespace savage::shader;
		std::vector<glm::vec3> data;
		data.push_back(glm::vec3(0,0,0));
		data.push_back(glm::vec3(1,0,0));
		data.push_back(glm::vec3(1,1,0));
		data.push_back(glm::vec3(1,1,0));
		data.push_back(glm::vec3(0,1,0));
		data.push_back(glm::vec3(0,0,0));
		model_.reset(new savage::model());
		model_->load_from_obj_file("cube.obj");
		model_->initialize(program_, "VertexPosition");
		//model_ = models::load_from_obj_file("cube.obj");
		// initialize shader
		{
			vertex_object vertex_object;
			objects::compile(vertex_object, vert);
			programs::attach_object(program_, vertex_object);
		}
		/*
		{
			geometry_object geometry_object;
			objects::compile(geometry_object, geometry);
			programs::attach_object(program_, geometry_object);
		}
		*/
		{
			fragment_object fragment_object;
			objects::compile(fragment_object, frag);
			programs::attach_object(program_, fragment_object);
		}
		programs::link(program_);
		programs::use(program_);
	}

	template<typename Time>
	void Update(const Time& time) {
		const float delta_time = savage::times::get_seconds(time)-before_time_;

		const glm::vec2 
			window_center = 0.5f*savage::contexts::get_window_size(context_);
		const glm::vec2
			cursor_position = savage::contexts::get_cursor_position(context_);
		camera_rotation_ += cursor_speed_*(cursor_position-window_center);
		const float& camera_yaw = camera_rotation_[0];
		const float& camera_pitch = camera_rotation_[1];
		glm::vec3 camera_direction(
				std::cos(camera_pitch)*std::sin(camera_yaw),
				std::sin(camera_pitch),
				std::cos(camera_pitch)*std::cos(camera_yaw));

		{
			using namespace savage::key_states;
			using namespace savage::contexts;
			if(is_pressed(get_key_state(context_, escape))){
				std::exit(0);
			}
			if(is_pressed(get_key_state(context_, up))){
				camera_position_ += camera_move_speed_ * delta_time * camera_direction;
			}
			if(is_pressed(get_key_state(context_, down))){
				camera_position_ -= camera_move_speed_ * delta_time * camera_direction;
			}
			savage::contexts::set_cursor_position(context_, window_center);
			before_time_ = savage::times::get_seconds(time);
		}
	
		{
			using namespace savage::shader;
			glm::mat4 view_matrix = glm::lookAt(camera_position_, 
					camera_direction+camera_position_, glm::vec3(0, 1.0f, 0));

			glm::mat4 projection_matrix =
				glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);

			glm::mat4 vp = projection_matrix * view_matrix;
			programs::set_uniform(program_, "ViewProjectionMatrix", vp);
		
			{
				glm::mat4 id_matrix;
				glm::mat4 model_matrix = 
					glm::translate(id_matrix, glm::vec3(0, 0, 4.0f));
				programs::set_uniform(program_, "ModelMatrix", model_matrix);
				savage::models::draw(*model_);
			}
			/*
			{
				glm::mat4 id_matrix;
				glm::mat4 model_matrix = 
					glm::translate(id_matrix, glm::vec3(0, 0, 4.4f));
				programs::set_uniform(program_, "ModelMatrix", model_matrix);
				savage::models::draw(*model_);
			}
			*/

			/*
			vertex_array_objects::draw(model_);
			*/
		}
	}
	
	Room(context& context) : 
		context_(context), program_(),
		triangle_(6), 
		before_time_(0.0f),
		cursor_speed_(0.003f),
		camera_move_speed_(1.0f),
		camera_position_(0.0f, 0.0f, 0.0f),
		camera_rotation_(0.0f, 0.0f),
		model_(0){}

private:
	static const int num_vertices_ = 6;
	context& context_;
	savage::shader::program program_;
	//savage::shader::vertex_array_object triangle_;
	savage::shader::adapted::glsl::array_buffer position_buffer_;
	savage::shader::vertex_array triangle_;
	float before_time_;
	float cursor_speed_;
	float camera_move_speed_;
	glm::vec3 camera_position_;
	glm::vec2 camera_rotation_;
	boost::scoped_ptr<savage::model> model_;
};

//adapt Room to savage
namespace savage { namespace instances {
	namespace policy {
		template<typename Time>
		struct update<Room, Time> {
		private:
			typedef Room instance_type;
			typedef Time time_type;
		public:
			static void call(instance_type& instance, const time_type& time) {
				instance.Update(time);
			}
		};
	}// namespace policy
}// namespace instances
}// namespace savage
int main(int argc, char* argv[])
{
	context context(savage::window_size(1280, 800));
	glewInit();
	Room room(context);
	room.Initialize();
	savage::instances::main_loop(room, context);

    return 0;
}

#endif
