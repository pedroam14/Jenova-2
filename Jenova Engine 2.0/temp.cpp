if (affine)
	{
		//calculate Transformation Matrix
		Matrix4x4 matWorldView = Math::Mat_MultiplyMatrix(matWorld, matView);
		//matWorldViewProj = Math::Mat_MultiplyMatrix(matWorldView, matProj);

		//store Triangles for rastering later
		std::vector<GFX3D::Triangle> vecTrianglesToRaster;

		int nTriangleDrawnCount = 0;

		//process Triangles
		for (auto &tri : Triangles)
		{
			GFX3D::Triangle triTransformed;

			//just copy through texture coordinates
			triTransformed.t[0] = { tri.t[0].x, tri.t[0].y };
			triTransformed.t[1] = { tri.t[1].x, tri.t[1].y };
			triTransformed.t[2] = { tri.t[2].x, tri.t[2].y }; // Think!

			//transform Triangle from object into projected space
			triTransformed.p[0] = GFX3D::Math::Mat_MultiplyVector(matWorldView, tri.p[0]);
			triTransformed.p[1] = GFX3D::Math::Mat_MultiplyVector(matWorldView, tri.p[1]);
			triTransformed.p[2] = GFX3D::Math::Mat_MultiplyVector(matWorldView, tri.p[2]);

			//calculate Triangle Normal in WorldView Space
			GFX3D::Vector3D normal, line1, line2;
			line1 = GFX3D::Math::Vec_Sub(triTransformed.p[1], triTransformed.p[0]);
			line2 = GFX3D::Math::Vec_Sub(triTransformed.p[2], triTransformed.p[0]);
			normal = GFX3D::Math::Vec_CrossProduct(line1, line2);
			normal = GFX3D::Math::Vec_Normalise(normal);

			//cull Triangles that face away from viewer
			if (flags & RENDER_CULL_CW && GFX3D::Math::Vec_DotProduct(normal, triTransformed.p[0]) > 0.0f)
				continue;
			if (flags & RENDER_CULL_CCW && GFX3D::Math::Vec_DotProduct(normal, triTransformed.p[0]) < 0.0f)
				continue;

			//if Lighting, calculate shading
			triTransformed.col = JenovaSpace::WHITE;

			//clip Triangle against near plane
			int nClippedTriangles = 0;
			Triangle clipped[2];
			nClippedTriangles = GFX3D::Math::Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.0f }, triTransformed, clipped[0], clipped[1]);

			//this may yield two new Triangles
			for (int n = 0; n < nClippedTriangles; n++)
			{
				Triangle triProjected = clipped[n];

				//project new Triangle
				triProjected.p[0] = GFX3D::Math::Mat_MultiplyVector(matProj, clipped[n].p[0]);
				triProjected.p[1] = GFX3D::Math::Mat_MultiplyVector(matProj, clipped[n].p[1]);
				triProjected.p[2] = GFX3D::Math::Mat_MultiplyVector(matProj, clipped[n].p[2]);

				//apply Projection to Verts
				triProjected.p[0].x = triProjected.p[0].x / triProjected.p[0].w;
				triProjected.p[1].x = triProjected.p[1].x / triProjected.p[1].w;
				triProjected.p[2].x = triProjected.p[2].x / triProjected.p[2].w;

				triProjected.p[0].y = triProjected.p[0].y / triProjected.p[0].w;
				triProjected.p[1].y = triProjected.p[1].y / triProjected.p[1].w;
				triProjected.p[2].y = triProjected.p[2].y / triProjected.p[2].w;

				triProjected.p[0].z = triProjected.p[0].z / triProjected.p[0].w;
				triProjected.p[1].z = triProjected.p[1].z / triProjected.p[1].w;
				triProjected.p[2].z = triProjected.p[2].z / triProjected.p[2].w;

				//clip against viewport in screen space
				//clip Triangles against all four screen edges, this could yield a bunch of Triangles, so create a queue that we traverse to ensure we only test new Triangles generated against planes
				Triangle sclipped[2];
				std::list<Triangle> listTriangles;

				//add initial Triangle
				listTriangles.push_back(triProjected);
				int nNewTriangles = 1;

				for (int p = 0; p < 4; p++)
				{
					int nTrisToAdd = 0;
					while (nNewTriangles > 0)
					{
						//take Triangle from front of queue
						Triangle test = listTriangles.front();
						listTriangles.pop_front();
						nNewTriangles--;

						//clip it against a plane. We only need to test each subsequent plane, against subsequent new Triangles as all Triangles after a plane clip are guaranteed to lie on the inside of the plane. I like how this comment is almost completely and utterly justified
						switch (p)
						{
						case 0:
							nTrisToAdd = GFX3D::Math::Triangle_ClipAgainstPlane({ 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, sclipped[0], sclipped[1]);
							break;
						case 1:
							nTrisToAdd = GFX3D::Math::Triangle_ClipAgainstPlane({ 0.0f, +1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, sclipped[0], sclipped[1]);
							break;
						case 2:
							nTrisToAdd = GFX3D::Math::Triangle_ClipAgainstPlane({ -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, sclipped[0], sclipped[1]);
							break;
						case 3:
							nTrisToAdd = GFX3D::Math::Triangle_ClipAgainstPlane({ +1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, sclipped[0], sclipped[1]);
							break;
						}

						//clipping may yield a variable number of Triangles, so add these new ones to the back of the queue for subsequent clipping against next planes
						for (int w = 0; w < nTrisToAdd; w++)
							listTriangles.push_back(sclipped[w]);
					}
					nNewTriangles = listTriangles.size();
				}

				for (auto &triRaster : listTriangles)
				{
					//scale to viewport
					/*
							triRaster.p[0].x *= -1.0f;
							triRaster.p[1].x *= -1.0f;
							triRaster.p[2].x *= -1.0f;
							triRaster.p[0].y *= -1.0f;
							triRaster.p[1].y *= -1.0f;
							triRaster.p[2].y *= -1.0f;
							*/
					Vector3D vOffsetView = { 1, 1, 0 };
					triRaster.p[0] = Math::Vec_Add(triRaster.p[0], vOffsetView);
					triRaster.p[1] = Math::Vec_Add(triRaster.p[1], vOffsetView);
					triRaster.p[2] = Math::Vec_Add(triRaster.p[2], vOffsetView);
					triRaster.p[0].x *= 0.5f * fViewW;
					triRaster.p[0].y *= 0.5f * fViewH;
					triRaster.p[1].x *= 0.5f * fViewW;
					triRaster.p[1].y *= 0.5f * fViewH;
					triRaster.p[2].x *= 0.5f * fViewW;
					triRaster.p[2].y *= 0.5f * fViewH;
					vOffsetView = { fViewX, fViewY, 0 };
					triRaster.p[0] = Math::Vec_Add(triRaster.p[0], vOffsetView);
					triRaster.p[1] = Math::Vec_Add(triRaster.p[1], vOffsetView);
					triRaster.p[2] = Math::Vec_Add(triRaster.p[2], vOffsetView);

					//for now, just draw Triangle

					if (flags & RENDER_TEXTURED)
					{
						TexturedTriangle(
							triRaster.p[0].x, triRaster.p[0].y, triRaster.t[0].x, triRaster.t[0].y, triRaster.t[0].z,
							triRaster.p[1].x, triRaster.p[1].y, triRaster.t[1].x, triRaster.t[1].y, triRaster.t[1].z,
							triRaster.p[2].x, triRaster.p[2].y, triRaster.t[2].x, triRaster.t[2].y, triRaster.t[2].z,
							sprTexture, affine);
					}

					if (flags & RENDER_WIRE)
					{
						DrawTriangleWire(triRaster, JenovaSpace::RED);
					}

					if (flags & RENDER_FLAT)
					{
						DrawTriangleFlat(triRaster);
					}

					nTriangleDrawnCount++;
				}
			}
		}

		return nTriangleDrawnCount;
	}


if (affine)
	{
		if (y2 < y1)
		{
			std::swap(y1, y2);
			std::swap(x1, x2);
			std::swap(u1, u2);
			std::swap(v1, v2);
		}

		if (y3 < y1)
		{
			std::swap(y1, y3);
			std::swap(x1, x3);
			std::swap(u1, u3);
			std::swap(v1, v3);
		}

		if (y3 < y2)
		{
			std::swap(y2, y3);
			std::swap(x2, x3);
			std::swap(u2, u3);
			std::swap(v2, v3);
		}

		int dy1 = y2 - y1;
		int dx1 = x2 - x1;
		double dv1 = v2 - v1;
		double du1 = u2 - u1;

		int dy2 = y3 - y1;
		int dx2 = x3 - x1;
		double dv2 = v3 - v1;
		double du2 = u3 - u1;

		double tex_u, tex_v;

		double dax_step = 0, dbx_step = 0,
			   du1_step = 0, dv1_step = 0,
			   du2_step = 0, dv2_step = 0;

		if (dy1)
			dax_step = dx1 / (double)abs(dy1);
		if (dy2)
			dbx_step = dx2 / (double)abs(dy2);

		if (dy1)
			du1_step = du1 / (double)abs(dy1);
		if (dy1)
			dv1_step = dv1 / (double)abs(dy1);

		if (dy2)
			du2_step = du2 / (double)abs(dy2);
		if (dy2)
			dv2_step = dv2 / (double)abs(dy2);

		if (dy1)
		{
			for (int i = y1; i <= y2; i++)
			{
				int ax = x1 + (double)(i - y1) * dax_step;
				int bx = x1 + (double)(i - y1) * dbx_step;

				double tex_su = u1 + (double)(i - y1) * du1_step;
				double tex_sv = v1 + (double)(i - y1) * dv1_step;

				double tex_eu = u1 + (double)(i - y1) * du2_step;
				double tex_ev = v1 + (double)(i - y1) * dv2_step;

				if (ax > bx)
				{
					std::swap(ax, bx);
					std::swap(tex_sv, tex_ev);
				}

				tex_u = tex_su;
				tex_v = tex_sv;

				double tstep = 1.0f / ((double)(bx - ax));
				double t = 0.0f;

				for (int j = ax; j < bx; j++)
				{
					tex_u = (1.0f - t) * tex_su + t * tex_eu;
					tex_v = (1.0f - t) * tex_sv + t * tex_ev;

					pge->Draw(j, i, spr->Sample(tex_u, tex_v));
					t += tstep;
				}
			}
		}

		dy1 = y3 - y2;
		dx1 = x3 - x2;
		dv1 = v3 - v2;
		du1 = u3 - u2;

		if (dy1)
			dax_step = dx1 / (double)abs(dy1);
		if (dy2)
			dbx_step = dx2 / (double)abs(dy2);

		du1_step = 0, dv1_step = 0;
		if (dy1)
			du1_step = du1 / (double)abs(dy1);
		if (dy1)
			dv1_step = dv1 / (double)abs(dy1);

		if (dy1)
		{
			for (int i = y2; i <= y3; i++)
			{
				int ax = x2 + (double)(i - y2) * dax_step;
				int bx = x1 + (double)(i - y1) * dbx_step;

				double tex_su = u2 + (double)(i - y2) * du1_step;
				double tex_sv = v2 + (double)(i - y2) * dv1_step;

				double tex_eu = u1 + (double)(i - y1) * du2_step;
				double tex_ev = v1 + (double)(i - y1) * dv2_step;

				if (ax > bx)
				{
					std::swap(ax, bx);
					std::swap(tex_su, tex_eu);
					std::swap(tex_sv, tex_ev);
				}

				tex_u = tex_su;
				tex_v = tex_sv;

				double tstep = 1.0f / ((double)(bx - ax));
				double t = 0.0f;

				for (int j = ax; j < bx; j++)
				{
					tex_u = (1.0f - t) * tex_su + t * tex_eu;
					tex_v = (1.0f - t) * tex_sv + t * tex_ev;

					pge->Draw(j, i, spr->Sample(tex_u, tex_v));

					t += tstep;
				}
			}
		}
	}