#include <glm/glm.hpp>
#include <iostream>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"
#include "model.hpp"

using namespace std;
using namespace glm;
 
void Model::render(mat4 cam_view, Shader* shader){
   glLineWidth(line_width);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_model);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_model);

	glVertexAttribPointer(shader->get_attr_pos(), 3, GL_FLOAT, GL_FALSE, 0, 0);
   glVertexAttribPointer(shader->get_attr_col(), 3, GL_FLOAT, GL_FALSE, 0, (void *)(sizeof(float) * 3 * n_verts));
	//if(type==COMPLEX){
   glVertexAttribPointer(shader->get_attr_norm(), 3, GL_FLOAT, GL_FALSE, 0, (void *)(sizeof(float) * 6 * n_verts));
   //}
	glDrawElements(draw_mode, n_draw_elements, GL_UNSIGNED_SHORT, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Model::load_complex(const glm::vec3* positions, const glm::vec3* colors, int res_u, int res_v){
    this->res_u = res_u;
    this->res_v = res_v;
    n_draw_elements = (res_v - 1) * 2 * res_u + (res_v - 2) * 2;
    n_draw_wire_elements = (res_v - 1) * 2 * res_u + (res_u - 1) * 2 * res_v;

    n_verts = res_u * res_v;

    // Calculate normals and fill buffer:
    float *verts = new float[3 * 3 * n_verts];
    for(int j=0; j<res_v; ++j)
    {
        for(int i=0; i<res_u; ++i)
        {
            const int pos_idx = (i + 1) + (res_u + 2) * (j + 1);
            const int col_idx = i + res_u * j;
            const int vert_idx = 3 * (i + res_u * j);

            // Approximate normal at current vertex with 4 neighboring vertices.
            // The normal is the cross product between the relative vectors:
            //
            //  .   .   .   .   .
            //  .   .  v_n  .   .
            //  .  v_w (*) v_e  .  ==> normal at (*): (v_e - v_w) x (v_n - v_s)
            //  .   .  v_s  .   .
            //  .   .   .   .   .
            //
            // If vertices on the grid fall together to make triangles (like at
            // the caps of a simple sphere), one of these relative vectors can
            // be zero which is bad.
            // If this occurs, these vertices have to be avoided, for example
            // like this:
            //
            //  .   .   .   .   .
            //  .   .  v_n  .   .
            //         (*)         <-- here a row of vertices is in one point
            //  .  v_w v_s v_e  .
            //  .   .   .   .   .

            vec3 v_w = positions[pos_idx - 1];
            vec3 v_e = positions[pos_idx + 1];
            vec3 v_s = positions[pos_idx + (res_u + 2)];
            vec3 v_n = positions[pos_idx - (res_u + 2)];

            const float tol = 1e-15;

            // Watch out for the "triangles":
            if(length(v_e - v_w) < tol)
            {
                v_w = positions[pos_idx - (res_u + 2) - 1];
                v_e = positions[pos_idx - (res_u + 2) + 1];

                if(j == 0) swap(v_w, v_e);  // this is a very hacky fix for simple spheres

                //cerr << "INFO: WE correction #1 at i=" << i << ", j=" << j << "\n";

                if(length(v_e - v_w) < tol)
                {
                    v_w = positions[pos_idx + (res_u + 2) - 1];
                    v_e = positions[pos_idx + (res_u + 2) + 1];

                    if(j == res_v - 1) swap(v_w, v_e);  // as above

                    //cerr << "INFO: WE correction #2 at i=" << i << ", j=" << j << "\n";
                }
            }

            if(length(v_n - v_s) < tol)
            {
                v_s = positions[pos_idx + (res_u + 2) - 1];
                v_n = positions[pos_idx - (res_u + 2) - 1];

                if(i == 0) swap(v_s, v_n);

                //cerr << "INFO: NS correction #1 at i=" << i << ", j=" << j << "\n";

                if(length(v_n - v_s) < tol)
                {
                    v_s = positions[pos_idx + (res_u + 2) + 1];
                    v_n = positions[pos_idx - (res_u + 2) + 1];

                    if(i == res_u - 1) swap(v_s, v_n);

                    //cerr << "INFO: NS correction #2 at i=" << i << ", j=" << j << "\n";
                }
            }

            // Do the cross product:
            vec3 norm = cross(v_e - v_w, v_n - v_s);
            const float norm_len = length(norm);

            if(norm_len < tol)
            {
                // Just give up at this point:
                norm = vec3(0, 0, 0);
                cerr << "WARNING: Null normal at i=" << i << ", j=" << j << "\n";
            }
            else
            {
                // Normal-ize!
                norm /= norm_len;
            }

            // Fill in GPU buffer data:
            verts[vert_idx + 0] = positions[pos_idx].x;
            verts[vert_idx + 1] = positions[pos_idx].y;
            verts[vert_idx + 2] = positions[pos_idx].z;

            verts[3 * n_verts + vert_idx + 0] = colors[col_idx].x;
            verts[3 * n_verts + vert_idx + 1] = colors[col_idx].y;
            verts[3 * n_verts + vert_idx + 2] = colors[col_idx].z;

            verts[6 * n_verts + vert_idx + 0] = norm.x;
            verts[6 * n_verts + vert_idx + 1] = norm.y;
            verts[6 * n_verts + vert_idx + 2] = norm.z;
        }
    }

    bind_vertices_array(verts,n_verts);

    delete[] verts;

    uint16_t *elems;

    // Calculate element indices for filled model:
    elems = new uint16_t[n_draw_elements];
    {
        size_t idx = 0;
        for(int j=0; j < res_v - 1; ++j)
        {
            if(j > 0)
            {
                // Generate degenerate triangles:
                elems[idx ++] = res_u * (j + 1) - 1;
                elems[idx ++] = res_u * j;
            }

            for(int i=0; i<res_u; ++i)
            {
                elems[idx ++] = i + res_u * j;
                elems[idx ++] = i + res_u * (j + 1);
            }
        }
        assert(idx == n_draw_elements);
    }

    bind_indices_array(elems,n_draw_elements);

    delete[] elems;

    // Calculate element indices for model wireframe:
    elems = new uint16_t[n_draw_wire_elements];
    {
        size_t idx = 0;
        for(int j=0; j < res_v; ++j)
        {
            for(int i=0; i < res_u - 1; ++i)
            {
                elems[idx ++] = i + res_u * j;
                elems[idx ++] = i + 1 + res_u * j;
            }
        }
        for(int i=0; i < res_u; ++i)
        {
            for(int j=0; j < res_v - 1; ++j)
            {
                elems[idx ++] = i + res_u * j;
                elems[idx ++] = i + res_u * (j + 1);
            }
        }
        assert(idx == n_draw_wire_elements);
    }

    glGenBuffers(1, &ibo_model_wire);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_model_wire);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * n_draw_wire_elements, elems, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    delete[] elems;

    type=COMPLEX;
}

void Model::bind_indices_array(uint16_t* elems, const size_t num_elements){
    n_draw_elements = num_elements;
    glGenBuffers(1, &ibo_model);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_model);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * n_draw_elements, elems, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);	
}

void Model::bind_vertices_array(float *verts, const size_t num_verts){
    this->n_verts = num_verts;
    glGenBuffers(1, &vbo_model);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_model);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 3 * num_verts, verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Model::set_pos(glm::vec3 position){
   pos = position;
}
