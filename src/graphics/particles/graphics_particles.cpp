
#include "graphics/graphics.h"
#include "graphics/texturehandler.h"
#include "graphics/shaders.h"
#include "graphics/hud.h"
#include "graphics/frustum/matrix4.h"
#include "graphics/texturecoordinate.h"
#include "graphics/window.h"
#include "graphics/menus/menubutton.h"

#include <vector>
#include <string>

using std::vector;
using std::string;

void GameView::drawParticles(std::vector<Particle>& viewParticles)
{
    OpenGL gl;
    if(!gl.getGL3bit())
        return;

    string depth_texture = (intVals["DEFERRED_RENDERING"] ? Graphics::Framebuffer::get("deferredFBO").depth_texture() : Graphics::Framebuffer::get("screenFBO").depth_texture());

    switch(intVals["DRAW_PARTICLES"])
    {
            case 2: drawParticles_vbo(viewParticles, depth_texture); break;
            case 3: drawParticles_old(viewParticles); break;
            default: drawParticles(viewParticles, depth_texture);
    }
}

// TODO: make this function faster than the current implementation by having the data already in the VBO format.
// TODO: Now it has to do extra copying.
void GameView::drawParticles_vbo(std::vector<Particle>& viewParticles, const std::string& depth_texture)
{
	Shader& shader = shaders.get_shader("particle_program");
	shader.start();
	GLint scale_location = shader.attribute("particleScale");
	glUniform1f(shader.uniform("near"), camera_p->nearP);
	glUniform1f(shader.uniform("far"), camera_p->farP);

	vec3<float> direction_vector = camera_p->getTarget() - camera_p->getPosition();
	depthSortParticles(direction_vector, viewParticles);

	TextureHandler::getSingleton().bindTexture(1, depth_texture);
	TextureHandler::getSingleton().bindTexture(0, "particle");
//	TextureHandler::getSingleton().bindTexture(0, "smoke");

	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDepthMask(GL_FALSE); // dont write to depth buffer.

	static vector<vec3<float> > particles;
	static vector<float> colors;
	static vector<float> scales;
	static bool buffers_loaded = false;
	static const int BUFFERS = 3;
	static GLuint locations[BUFFERS];

	if(!buffers_loaded)
	{
		buffers_loaded = true;
		glGenBuffers(BUFFERS, locations);
	}

	size_t n = viewParticles.size();
	if(particles.size() < n)
	{
		particles.resize(n);
		colors.resize(4*n);
		scales.resize(n);
	}

	for(size_t i = 0; i < viewParticles.size(); ++i)
	{
		particles[i] = viewParticles[i].pos;

		viewParticles[i].getColor(&colors[i*4]);
		colors[i*4 + 3] = viewParticles[i].getAlpha();
		scales[i] = viewParticles[i].getScale();
	}

	int buffer = 0;
	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glBufferData(GL_ARRAY_BUFFER, n * sizeof(vec3<float>), &particles[0], GL_STREAM_DRAW);
	glVertexPointer(3, GL_FLOAT, 0, 0);
	glEnableClientState(GL_VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glBufferData(GL_ARRAY_BUFFER, n * 4 * sizeof(float), &colors[0], GL_STREAM_DRAW);
	glColorPointer(4, GL_FLOAT, 0, 0);
	glEnableClientState(GL_COLOR_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glBufferData(GL_ARRAY_BUFFER, n * sizeof(float), &scales[0], GL_STREAM_DRAW);
	glVertexAttribPointer(scale_location, 1, GL_FLOAT, GL_FALSE, sizeof(float), 0);

	assert(buffer == BUFFERS);

	// Draw sent data.
	glEnableVertexAttribArray(scale_location);
	glDrawArrays(GL_POINTS, 0, viewParticles.size());
	glDisableVertexAttribArray(scale_location);
	glDisableClientState(GL_VERTEX_ARRAY);

	glDepthMask(GL_TRUE); // re-enable depth writing.
	glDisable(GL_BLEND);

	TextureHandler::getSingleton().unbindTexture(1);
	TextureHandler::getSingleton().unbindTexture(0);

	shader.stop();
}

void GameView::prepareForParticleRendering(const std::string& depth_texture)
{
	Graphics::Framebuffer::get("particlesDownScaledFBO").bind();
	glClear(GL_COLOR_BUFFER_BIT);

	TextureHandler::getSingleton().bindTexture(1, depth_texture);
	TextureHandler::getSingleton().bindTexture(0, "particle");

	// vec3<float> direction_vector = camera_p->getTarget() - camera_p->getPosition();
	// depthSortParticles(direction_vector, viewParticles);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
}

void GameView::renderParticles(std::vector<Particle>& viewParticles)
{
	Shader& shader = shaders.get_shader("particle_program");
	shader.start();
	GLint scale_location = shader.attribute("particleScale");
	glUniform1f(shader.uniform("near"), camera_p->nearP);
	glUniform1f(shader.uniform("far"), camera_p->farP);

	// The geometry shader transforms the points into quads.
	glBegin(GL_POINTS);
	for(size_t i = 0; i < viewParticles.size(); ++i)
	{
		float px = viewParticles[i].pos.x;
		float py = viewParticles[i].pos.y;
		float pz = viewParticles[i].pos.z;

		float color[4];
		viewParticles[i].getColor(color);
		color[3] = viewParticles[i].getAlpha();

		// cerr << color[0] << " " << color[1] << " " << color[2] << " " << color[3] << "\n";

		float scale = viewParticles[i].getScale();

		glVertexAttrib1f(scale_location, scale);
		glColor4fv(color);
		glVertex3f(px, py, pz);
	}
	glEnd();

	//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	TextureHandler::getSingleton().unbindTexture(1);

	shader.stop();
}

void GameView::drawParticles(std::vector<Particle>& viewParticles, const std::string& depth_texture)
{
	prepareForParticleRendering(depth_texture);
	Graphics::Framebuffer& particlesUpScaledFBO = Graphics::Framebuffer::get("particlesUpScaledFBO");

	// create a downscaled depth texture for particle rendering
	renderParticles(viewParticles);

	// upscale particle rendering result
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_BLEND);
	Graphics::Framebuffer::get("particlesUpScaledFBO").bind();

	TextureHandler::getSingleton().bindTexture(0, Graphics::Framebuffer::get("particlesDownScaledFBO").texture(0));
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// TODO: Could this perhaps be done (faster?) by directly copying from one framebuffer to another?
	// Perhaps with with glBindGraphics::Framebuffer(GL_READ_FRAMEBUFFER,...); glBindGraphics::Framebuffer(GL_DRAW_FRAMEBUFFER,...); glBlitGraphics::Framebuffer(...);??
	drawFullscreenQuad(); // Draw particlesDownScaledFBO_texture to particlesUpScaledFBO_texture (i.e. upscale).

	// blur upscaled particle texture
	if(intVals["PARTICLE_BLUR"])
	{
		// Draw particlesUpScaledFBO_texture to particlesUpScaledFBO_texture (apply blur).
		applyBlur(intVals["PARTICLE_BLUR_AMOUNT"], particlesUpScaledFBO.texture(0), particlesUpScaledFBO);

		// Draw particlesUpScaledFBO_texture to particlesUpScaledFBO_texture (apply blur).
		applyBlur(intVals["PARTICLE_BLUR_AMOUNT"], particlesUpScaledFBO.texture(0), particlesUpScaledFBO);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Graphics::Framebuffer::get("screenFBO").bind();

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	TextureHandler::getSingleton().bindTexture(0, particlesUpScaledFBO.texture(0));
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	drawFullscreenQuad(); // Draw particlesUpScaledFBO_texture to screenFBO_texture (copy directly).

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	TextureHandler::getSingleton().unbindTexture(0);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}
