#include "rasterizer.hpp"

#include <math.h>

#include <algorithm>
#include <opencv2/opencv.hpp>
#include <vector>

rst::pos_buf_id rst::rasterizer::load_positions(
    const std::vector<Eigen::Vector3f>& positions) {
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return { id };
}

rst::ind_buf_id rst::rasterizer::load_indices(
    const std::vector<Eigen::Vector3i>& indices) {
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return { id };
}

rst::col_buf_id rst::rasterizer::load_colors(
    const std::vector<Eigen::Vector4f>& cols) {
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return { id };
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f) {
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}

bool rst::rasterizer::is_back_facing(const Eigen::Vector4f* v) const {
    // Compute signed area using cross product of edges in screen space
    // If signed area < 0, vertices are clockwise (back-facing)
    // If signed area > 0, vertices are counter-clockwise (front-facing)
    float signed_area = - ((v[1].x() - v[0].x()) * (v[2].y() - v[0].y())
                      - (v[1].y() - v[0].y()) * (v[2].x() - v[0].x()));
    return signed_area < 0;
}


static bool insideTriangle(int x, int y, const Vector3f* _v) {
    auto cross2D = [](const Eigen::Vector3f& v1, const Eigen::Vector3f& v2) {
        return v1.x() * v2.y() - v1.y() * v2.x();  // ignore z
        };
    Vector3f A = _v[0], B = _v[1], C = _v[2];
    bool inside = false;
    Vector3f P(x + .5, y + .5, 0);
    Eigen::Vector3f AB = B - A, AP = P - A;
    Eigen::Vector3f BC = C - B, BP = P - B;
    Eigen::Vector3f CA = A - C, CP = P - C;

    float cross1 = cross2D(AB, AP);
    float cross2 = cross2D(BC, BP);
    float cross3 = cross2D(CA, CP);

    return (cross1 >= 0 && cross2 >= 0 && cross3 >= 0) ||
        (cross1 <= 0 && cross2 <= 0 && cross3 <= 0);
    // TODO : Implement this function to check if the point (x, y) is inside the
    // triangle represented by three vertices: _v[0], _v[1], _v[2]

}


static std::tuple<float, float, float> computeBarycentric2D(float x, float y,
    const Vector3f* v) {
    float c1 =
        (x * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * y +
            v[1].x() * v[2].y() - v[2].x() * v[1].y()) /
        (v[0].x() * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * v[0].y() +
            v[1].x() * v[2].y() - v[2].x() * v[1].y());
    float c2 =
        (x * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * y +
            v[2].x() * v[0].y() - v[0].x() * v[2].y()) /
        (v[1].x() * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * v[1].y() +
            v[2].x() * v[0].y() - v[0].x() * v[2].y());
    float c3 =
        (x * (v[0].y() - v[1].y()) + (v[1].x() - v[0].x()) * y +
            v[0].x() * v[1].y() - v[1].x() * v[0].y()) /
        (v[2].x() * (v[0].y() - v[1].y()) + (v[1].x() - v[0].x()) * v[2].y() +
            v[0].x() * v[1].y() - v[1].x() * v[0].y());
    return { c1, c2, c3 };
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer,
    col_buf_id col_buffer, Primitive type) {
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];
    auto& col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto& i : ind) {
        Triangle t;
        Eigen::Vector4f v[] = { mvp * to_vec4(buf[i[0]], 1.0f),
                               mvp * to_vec4(buf[i[1]], 1.0f),
                               mvp * to_vec4(buf[i[2]], 1.0f) };
        // Homogeneous division
        for (auto& vec : v) {
            vec /= vec.w();
        }
        // Viewport transformation
        for (auto& vert : v) {
            vert.x() = 0.5 * width * (vert.x() + 1.0);
            vert.y() = 0.5 * height * (vert.y() + 1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        // Back face culling
        // TODO:
        // Step 1: Figure out the orientation of the triangle in screen space.
        //
        // Step 2: Based on the orientation and the active cull mode,
        //         either discard the triangle and move on to the next one,
        //         or let it through to rasterization.
        if (cull_mode != CullMode::None) {
            bool back_facing = is_back_facing(v);
            if ((cull_mode == CullMode::Back && back_facing) ||
                (cull_mode == CullMode::Front && !back_facing)) {
                continue;  // Skip this triangle
            }
        }

        for (int i = 0; i < 3; ++i) {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2],  col_x[3]);
        t.setColor(1, col_y[0], col_y[1], col_y[2],  col_y[3]);
        t.setColor(2, col_z[0], col_z[1], col_z[2],  col_z[3]);

        rasterize_triangle(t);
    }
}



// Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    auto v = t.toVector4();
    // TODO:
    //
    // Step 1: Find out the bounding box of current triangle.
    int  min_x = std::floor(std::min({ v[0].x(), v[1].x(), v[2].x() }));
    int  max_x = std::ceil(std::max({ v[0].x(), v[1].x(), v[2].x() }));
    int  min_y = std::floor(std::min({ v[0].y(), v[1].y(), v[2].y() }));
    int  max_y = std::ceil(std::max({ v[0].y(), v[1].y(), v[2].y() }));

    // Step 2: Iterate through the pixel and find if the current pixel is inside
    //         the triangle.

    for (int x = min_x; x <= max_x; x++ ) {
        for (int y = min_y; y <= max_y; y++) {
            if (x < 0 || x >= width || y < 0 || y >= height) continue;
            if (insideTriangle(x, y, t.v)) {
                auto [alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
                float w_reciprocal = 1.0 / (alpha / v[0].w() + beta / v[1].w() +
                    gamma / v[2].w());
                float z_interpolated = alpha * v[0].z() / v[0].w() + beta
                    * v[1].z() / v[1].w() + gamma * v[2].z()
                    / v[2].w();
                z_interpolated *= w_reciprocal;

                // Step 2.2: Set the current pixel (use the set_pixel() function) to the color
                // of the triangle (use getColor() function) if it should be painted.
                int ind = get_index(x, y);

                //     //set_pixel(Eigen::Vector3f(x, y, z_interpolated), t.getColor());
				// 	frame_buf[ind] = t.getColor();
                // }

                if (blend_mode == blenmode::depth_test) {
                        if (depth_buf[ind] < z_interpolated) {
                            depth_buf[ind] = z_interpolated;
                            frame_buf[ind] = t.getColor();
                        }
                }
                else if (blend_mode == blenmode::None) {
                    frame_buf[ind] = t.getColor();
                }
                else
                    alpha_blend( t.getColor(), frame_buf[ind]);
            }
        }
    }
}

void rst::rasterizer::alpha_blend( Vector4f src, Vector4f& dst){
    float src_alpha = src[3];
    float dst_alpha = dst[3];
    //check to see if the dst is the background, (0,0,0) -> just return src
    // if (dst.head<3>() == Vector3f(0, 0, 0)) {
    //     dst = src;
    //     return;
    // }
    switch (this->blend_mode) {
    case rst::blenmode::src_alpha:
        // Classic alpha blending: src * alpha + dst * (1 - alpha)
        dst.head<3>() = src.head<3>() * src_alpha + dst.head<3>() * (1.0f - src_alpha);
        dst[3] = src_alpha + dst_alpha * (1.0f - src_alpha);  // update alpha correctly
        break;

    case rst::blenmode::one:
        dst = src + dst; // additive blending
        dst[3] = std::min(1.0f, dst[3]); // clamp alpha to 1
        break;

    default:
        break;
    }
}

void rst::rasterizer::set_model(const Eigen::Matrix4f& m) { model = m; }

void rst::rasterizer::set_view(const Eigen::Matrix4f& v) { view = v; }

void rst::rasterizer::set_projection(const Eigen::Matrix4f& p) {
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff) {
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color) {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector4f{ 0, 0, 0, 0 });
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth) {
        std::fill(depth_buf.begin(), depth_buf.end(),
            std::numeric_limits<float>::infinity() * -1.0);
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h) {
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);
}

int rst::rasterizer::get_index(int x, int y) {
    return (height - 1 - y) * width + x;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f& point,
    const Eigen::Vector4f& color) {
    // old index: auto ind = point.y() + point.x() * width;
    auto ind = (height - 1 - point.y()) * width + point.x();
    frame_buf[ind] = color;
}
