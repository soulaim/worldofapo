
struct VisualLevelPart
{
	Vec3 bounding_sphere_center;
	float bounding_sphere_radius;
	
	static const int BUFFERS = 5;
	GLuint locations[BUFFERS];
	bool buffers_loaded;

	vector<unsigned> indices;
	vector<Vec3> vertices;
	vector<Vec3> normals;
	vector<TextureCoordinate> texture_coordinates1;
	vector<TextureCoordinate> texture_coordinates2;
//	vector<TextureCoordinate> texture_coordinates3; // All texture coordinates are actually same, so we'll let shader handle the third.

	VisualLevelPart():
		bounding_sphere_radius(99999999999.9f),
		buffers_loaded(false)
	{
	}

	void preload()
	{
		if(buffers_loaded)
			return;

		cerr << "PRELOADING LEVEL PART ";

		buffers_loaded = true;
		
		glGenBuffers(BUFFERS, locations);

		int buffer = 0;
		glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vec3), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
		glBufferData(GL_ARRAY_BUFFER, texture_coordinates1.size() * sizeof(TextureCoordinate), &texture_coordinates1[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
		glBufferData(GL_ARRAY_BUFFER, texture_coordinates2.size() * sizeof(TextureCoordinate), &texture_coordinates2[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(Vec3), &normals[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, locations[buffer++]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), &indices[0], GL_STATIC_DRAW);

		assert(buffer == BUFFERS);

		calculate_bounding_sphere();
		cerr << "CENTER: " << bounding_sphere_center << ", RADIUS: " << bounding_sphere_radius << endl;
	}

	void calculate_bounding_sphere()
	{
		bounding_sphere_center = accumulate(vertices.begin(), vertices.end(), Vec3());
		bounding_sphere_center /= vertices.size();

		bounding_sphere_radius = 0.0f;
		for(size_t i = 0; i < vertices.size(); ++i)
		{
			bounding_sphere_radius = max(bounding_sphere_radius, (bounding_sphere_center - vertices[i]).length());
		}
	}

	void draw() const
	{
		assert(texture_coordinates1.size() == vertices.size());
		assert(texture_coordinates2.size() == vertices.size());
		assert(normals.size() == vertices.size());
		assert(indices.size() % 3 == 0);
		
		int buffer = 0;
		
		// Bind static data and send dynamic data to graphics card.
		glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
		glVertexPointer(3, GL_FLOAT, 0, 0);
		glEnableClientState(GL_VERTEX_ARRAY);
		
		glClientActiveTexture(GL_TEXTURE1);
		glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
		glTexCoordPointer(2, GL_FLOAT, 0, 0);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		
		glClientActiveTexture(GL_TEXTURE0);
		glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
		glTexCoordPointer(2, GL_FLOAT, 0, 0);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		
		glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
		glNormalPointer(GL_FLOAT, 0, 0);
		glEnableClientState(GL_NORMAL_ARRAY);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, locations[buffer++]);

		assert(buffer == BUFFERS);

		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);

		TRIANGLES_DRAWN_THIS_FRAME += indices.size() / 3;
	}
};
