#ifdef ROOM_UNIT_TEST
#include "Room.h"
#include <iostream>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
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
    130,

    in vec3 Position;
	//in vec3 Normal;
	
	uniform mat4 ModelMatrix;
	uniform mat4 ViewMatrix;
	uniform mat4 ProjectionMatrix;

	out vec3 LightIntensity;

    void main()
    {
		gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(Position, 1.0);
    }
);

const char* frag = GLSL
(
    130,

	in vec3 LightIntensity;

    out vec4 FragmentColor;

    void main()
    {
		FragmentColor = vec4(LightIntensity, 1.0);
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
		//model_.reset(new savage::model("cornell_box.obj"));
		//model_.reset(new savage::model("/home/okada/Downloads/rungholt/rungholt.obj"));
		//model_.reset(new savage::model("sponza.obj"));
		model_.reset(new savage::model("cornell2_from_blender.obj"));
		model_->initialize(program_, "Position");

		// initialize shader
		{
			vertex_object vertex_object;
			objects::compile(vertex_object, vert);
			programs::attach_object(program_, vertex_object);
		}
		{
			fragment_object fragment_object;
			objects::compile(fragment_object, frag);
			programs::attach_object(program_, fragment_object);
		}
		programs::link(program_);
		programs::use(program_);

		/*
		const glm::vec2 
			window_center = 0.5f*savage::contexts::get_window_size(context_);
		savage::contexts::set_cursor_position(context_, window_center);
		glfwPollEvents();
		*/
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	}

	template<typename Time>
	void Update(const Time& time) {
		const float delta_time = savage::times::get_seconds(time)-before_time_;

		const glm::vec2
			window_size = savage::contexts::get_window_size(context_);
		const glm::vec2 
			window_center = 0.5f*savage::contexts::get_window_size(context_);
		const glm::vec2
			cursor_position = savage::contexts::get_cursor_position(context_);

		camera_rotation_ += cursor_speed_*(cursor_position-window_center);
		const float& camera_yaw = camera_rotation_[0];
		const float& camera_pitch = camera_rotation_[1];

		glm::vec3 camera_direction(0, 0, 1.0);
		camera_direction = glm::rotateX(camera_direction, -camera_pitch);
		camera_direction = glm::rotateY(camera_direction, camera_yaw);

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
			if(is_pressed(get_key_state(context_, r_key))){ //reset camera direction
				camera_rotation_ = glm::vec2();
			}
			savage::contexts::set_cursor_position(context_, window_center);
			before_time_ = savage::times::get_seconds(time);
		}
	
		{
			using namespace savage::shader;

			glm::mat4 projection_matrix =
				glm::perspective(30.0f, 2.0f / 3.0f, 0.1f, 1000.0f);
			programs::set_uniform(program_, "ProjectionMatrix", projection_matrix);

			{
				glm::vec3 left_camera_position = glm::vec3(-0.035, 0, 0);
				left_camera_position = glm::rotateX(left_camera_position, -camera_pitch);
				left_camera_position = glm::rotateY(left_camera_position, camera_yaw);
				left_camera_position += camera_position_;

				glm::mat4 view_matrix = glm::lookAt(left_camera_position, 
						camera_direction+left_camera_position, glm::vec3(0, -1.0f, 0));
				programs::set_uniform(program_, "ViewMatrix", view_matrix);
				glViewport(0, 0, window_size[0]/2, window_size[1]);
				scene();
			}
			{
				glm::vec3 right_camera_position = glm::vec3(0.035, 0, 0);
				right_camera_position = glm::rotateX(right_camera_position, -camera_pitch);
				right_camera_position = glm::rotateY(right_camera_position, camera_yaw);
				right_camera_position += camera_position_;
				glm::mat4 view_matrix = glm::lookAt(right_camera_position,
						camera_direction+right_camera_position, glm::vec3(0, -1.0f, 0));
				programs::set_uniform(program_, "ViewMatrix", view_matrix);
				glViewport(window_size[0]/2, 0, window_size[0]/2, window_size[1]);
				scene();
			}
		}
	}

	void scene() {
		using namespace savage::shader;
		glm::mat4 id_matrix;
		programs::set_uniform(program_, "ModelMatrix", 
				glm::scale(glm::translate(id_matrix, glm::vec3(0, 0, 4.0f)), 
					glm::vec3(1.0f)));
		savage::models::draw(*model_);
		/*
		programs::set_uniform(program_, "ModelMatrix", 
				glm::translate(id_matrix, glm::vec3(0, 1.0f, 1.0f)));
		savage::models::draw(*model_);
		*/
	}
	
	Room(context& context) : 
		context_(context), program_(),
		triangle_(6), 
		before_time_(0.0f),
		cursor_speed_(0.003f),
		camera_move_speed_(1.0f),
		camera_position_(0.0f, 0.0f, 0.0f),
		camera_rotation_(0, 0),
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
