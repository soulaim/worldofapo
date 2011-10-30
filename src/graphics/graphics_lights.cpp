
#include "graphics/graphics.h"
#include "graphics/texturehandler.h"
#include "graphics/shaders.h"
#include "graphics/frustum/matrix4.h"
#include "graphics/texturecoordinate.h"
#include "graphics/window.h"
#include "graphics/menus/menubutton.h"
#include "graphics/hud/hud.h"

#include "misc/apomath.h"

#include <string>
#include <vector>
#include <sstream>


// NOTE: set the y values to be the same. lights that were high above ground were drawn poorly
// because they were never near the player, although the effect is high.
bool near(const Camera& camera, const vec3<float>& position, float dist = 100.0)
{
	vec3<float> copy1 = camera.getPosition();
	vec3<float> copy2 = position;

	copy1.y = copy2.y;

	return (copy1 - copy2).lengthSquared() < dist * dist;
}

void GameView::updateLights(const std::map<int, LightObject>& lightsContainer)
{
//	clear_errors();

	// TODO: update only the shader that is currently active. Or update lights when actually drawing.
#ifdef _WIN32
	std::vector<std::string> programs;
	programs.push_back("level_program");
	programs.push_back("deferred_lights_program");
	programs.push_back("partitioned_deferred_lights_program");
	programs.push_back("partitioned_deferred_lights_program2");
#else
	// vector<string> programs = { "level_program", "deferred_lights_program", "partitioned_deferred_lights_program", "partitioned_deferred_lights_program2" };
        std::vector<std::string> programs = { "level_program", "deferred_lights_program", "partitioned_deferred_lights_program2" };
#endif

	Matrix4 modelview = camera_p->modelview();

	for(size_t p = 0; p < programs.size(); ++p)
	{
		Shader& shader = shaders.get_shader(programs[p]);
		shader.start();

		int i = 0;

		int POSITION = 0;
		int DIFFUSE = 1;

		for(auto iter = lightsContainer.begin(); iter != lightsContainer.end(); ++iter)
		{
			float rgb[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

			iter->second.getDiffuse(rgb[0], rgb[1], rgb[2]);
			std::stringstream ss1;
			ss1 << "lights[" << i*2 + DIFFUSE << "]";
			glUniform4f(shader.uniform(ss1.str()), rgb[0], rgb[1], rgb[2], rgb[3]);
//			check_errors(__FILE__, __LINE__);

			const Location& pos = iter->second.getPosition();
			vec3<float> v(pos.x.getFloat(), pos.y.getFloat(), pos.z.getFloat());
			v = modelview * v;

			std::stringstream ss2;
			ss2 << "lights[" << i*2 + POSITION << "]";
			glUniform4f(shader.uniform(ss2.str()), v.x, v.y, v.z, 1.0f);
//			check_errors(__FILE__, __LINE__);

			++i;
			if(i >= MAX_NUM_LIGHTS)
			{
				// if there are too many lights, just ignore the rest of them
				break;
			}
		}

		// always write multiples of four
		for(int k = i % 4; (k % 4) != 0; ++k)
		{
			if(i >= MAX_NUM_LIGHTS)
			{
				// if there are too many lights, just ignore the rest of them
				break;
			}

			float rgb[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			std::stringstream ss1;
			ss1 << "lights[" << i*2 + DIFFUSE << "]";
			glUniform4f(shader.uniform(ss1.str()), rgb[0], rgb[1], rgb[2], rgb[3]);

			std::stringstream ss2;
			ss2 << "lights[" << i*2 + POSITION << "]";
			glUniform4f(shader.uniform(ss2.str()), rgb[0], rgb[1], rgb[2], rgb[3]);

			++i;
		}

		shader.stop();
	}
}

void GameView::drawLightsDeferred_multiple_passes(const Camera& camera, const std::map<int, LightObject>& lights)
{
	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	TextureHandler::getSingleton().bindTexture(0, Graphics::Framebuffer::get("deferredFBO").texture(0));
	TextureHandler::getSingleton().bindTexture(1, Graphics::Framebuffer::get("deferredFBO").texture(1));
	// TextureHandler::getSingleton().bindTexture(2, deferredFBO.texture(2));
	TextureHandler::getSingleton().bindTexture(3, Graphics::Framebuffer::get("deferredFBO").depth_texture());
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	Shader& shader = shaders.get_shader("partitioned_deferred_lights_program");
	shader.start();
	glUniform1f(shader.uniform("fullscreen"), 0.0f);

	// Draw lights that are not near the camera as point sprites. Lights are blended together one light at a time.
	int pass = 0;
	for(auto it = lights.begin(); it != lights.end(); ++it, ++pass)
	{
		const LightObject& light = it->second;

		Location loc = light.getPosition();
		vec3<float> v = vec3<float>(loc.x.getFloat(), loc.y.getFloat(), loc.z.getFloat());

		// TODO: intensity doesn't accurately reflect (in worl units) how far the light is actually seen.
		// light power is linear,
		// light attenuation is quadratic
		// so, square root of power should be sensible measure? lets approximate it with a constant number.
		float power = light.getIntensity().getFloat();
		power = 1000.0f;

		if(near(camera, v, power / intVals["DEFERRED_LIGHT_FIX_SPEED"]))
		{
			continue;
		}

		glUniform1f(shader.uniform("activeLight"), float(pass));
		glUniform1f(shader.uniform("power"), power);

		// TODO: if this thing will someday work really well, then all lights could be passed as varyings
		// here instead of updated through uniforms elsewhere. Not sure which is faster though.


        // TODO: Draw a quad that is orthogonal to the view ray of the camera.
        /*
        float light_size = power / 6.5f;

        float sin = -1;
        float cos = 0;

        glBegin(GL_QUADS);
            glVertex3f(v.x - light_size * cos, v.y-light_size * 0.5f, v.z - 0.5f * light_size * sin);
            glVertex3f(v.x + light_size * cos, v.y-light_size * 0.5f, v.z + 0.5f * light_size * sin);
            glVertex3f(v.x - light_size * cos, v.y+light_size * 0.5f, v.z - 0.5f * light_size * sin);
            glVertex3f(v.x + light_size * cos, v.y+light_size * 0.5f, v.z + 0.5f * light_size * sin);
		glEnd();
        */
	}
	shader.stop();

	if(intVals["DEFERRED_LIGHT_FIX"])
	{
		Shader& shader2 = shaders.get_shader("partitioned_deferred_lights_program2");
		shader2.start();
		glUniform1f(shader2.uniform("fullscreen"), 1.0f);
		// Draw lights that are near the camera as fullscreen quads. Lights are blended together one light at a time.
		pass = 0;
		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);

		int full_screen_lights = 0;

		for(auto it = lights.begin(); it != lights.end(); ++it, ++pass)
		{
			const LightObject& light = it->second;

			Location loc = light.getPosition();
			vec3<float> v = vec3<float>(loc.x.getFloat(), loc.y.getFloat(), loc.z.getFloat());

			// TODO: same as the todo in previous lights loop
			float power = light.getIntensity().getFloat();
			power = 1000.0f;

			if(!near(camera, v, power / intVals["DEFERRED_LIGHT_FIX_SPEED"]))
			{
				continue;
			}

			full_screen_lights++;
			glUniform1f(shader2.uniform("activeLight"), float(pass));
			drawFullscreenQuad();
		}

		std::stringstream lights_debug_str;
		lights_debug_str << "FS Lights: " << full_screen_lights;
		hud.insertDebugString(lights_debug_str.str());

		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		shader2.stop();
	}

	glDisable(GL_BLEND);

	TextureHandler::getSingleton().unbindTexture(3);
	TextureHandler::getSingleton().unbindTexture(2);
	TextureHandler::getSingleton().unbindTexture(1);
	TextureHandler::getSingleton().unbindTexture(0);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}

void GameView::applyDeferredLights(const std::map<int, LightObject>& lights, int red, int green, int blue)
{
	Graphics::Framebuffer::get("screenFBO").bind();

	applyAmbientLight(red, green, blue);

	if(intVals["DRAW_DEFERRED_LIGHTS"] == 2)
	{
		drawLightsDeferred_single_pass(lights.size());
	}
	else if(intVals["DRAW_DEFERRED_LIGHTS"] == 1)
	{
		drawLightsDeferred_multiple_passes(*camera_p, lights);
	}
}
