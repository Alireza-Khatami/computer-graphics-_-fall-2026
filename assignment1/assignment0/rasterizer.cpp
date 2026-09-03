#include "rasterizer.hpp"

#include <math.h>

#include <algorithm>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <stdexcept>

rst::pos_buf_id rst::rasterizer::load_positions(
    const std::vector<Eigen::Vector3f>& positions) {
  auto id = get_next_id();
  pos_buf.emplace(id, positions);

  return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(
    const std::vector<Eigen::Vector3i>& indices) {
  auto id = get_next_id();
  ind_buf.emplace(id, indices);

  return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(
    const std::vector<Eigen::Vector4i>& indices) {
  auto id = get_next_id();
  quad_ind_buf.emplace(id, indices);

  return {id};
}

// Clip a line segment against the screen rectangle (Liang-Barsky).
//
// This matters once triangles are clipped at the camera plane: a vertex that
// sits a hair in front of the camera projects to a coordinate of millions of
// pixels. Bresenham would happily walk every one of them, so the segment is cut
// down to the part that is actually on screen first. Computed in double
// precision because the incoming coordinates can be enormous.
static bool clip_line_to_viewport(Eigen::Vector3f& p0, Eigen::Vector3f& p1,
                                  double xmax, double ymax) {
  const double x0 = p0.x(), y0 = p0.y();
  const double x1 = p1.x(), y1 = p1.y();

  if (!std::isfinite(x0) || !std::isfinite(y0) || !std::isfinite(x1) ||
      !std::isfinite(y1)) {
    return false;
  }

  const double dx = x1 - x0;
  const double dy = y1 - y0;

  double t0 = 0.0, t1 = 1.0;
  const double p[4] = {-dx, dx, -dy, dy};
  const double q[4] = {x0 - 0.0, xmax - x0, y0 - 0.0, ymax - y0};

  for (int i = 0; i < 4; ++i) {
    if (p[i] == 0.0) {
      if (q[i] < 0.0) return false;  // parallel to this edge and outside it
      continue;
    }
    const double t = q[i] / p[i];
    if (p[i] < 0.0) {
      if (t > t1) return false;
      if (t > t0) t0 = t;
    } else {
      if (t < t0) return false;
      if (t < t1) t1 = t;
    }
  }

  const double z0 = p0.z(), z1 = p1.z();
  p0 = Eigen::Vector3f(x0 + t0 * dx, y0 + t0 * dy, z0 + t0 * (z1 - z0));
  p1 = Eigen::Vector3f(x0 + t1 * dx, y0 + t1 * dy, z0 + t1 * (z1 - z0));
  return true;
}

// Bresenham's line drawing algorithm
void rst::rasterizer::draw_line(Eigen::Vector3f begin, Eigen::Vector3f end, Eigen::Vector3f& color) {
  if (!clip_line_to_viewport(begin, end, width - 1.0, height - 1.0)) {
    return;
  }

  auto x1 = begin.x();
  auto y1 = begin.y();
  auto x2 = end.x();
  auto y2 = end.y();

  Eigen::Vector3f line_color = {color.x() , color.y(), color.z()};

  int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;

  dx = x2 - x1;
  dy = y2 - y1;
  dx1 = fabs(dx);
  dy1 = fabs(dy);
  px = 2 * dy1 - dx1;
  py = 2 * dx1 - dy1;

  if (dy1 <= dx1) {
    if (dx >= 0) {
      x = x1;
      y = y1;
      xe = x2;
    } else {
      x = x2;
      y = y2;
      xe = x1;
    }
    Eigen::Vector3f point = Eigen::Vector3f(x, y, 1.0f);
    set_pixel(point, line_color);
    for (i = 0; x < xe; i++) {
      x = x + 1;
      if (px < 0) {
        px = px + 2 * dy1;
      } else {
        if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
          y = y + 1;
        } else {
          y = y - 1;
        }
        px = px + 2 * (dy1 - dx1);
      }
      //            delay(0);
      Eigen::Vector3f point = Eigen::Vector3f(x, y, 1.0f);
      set_pixel(point, line_color);
    }
  } else {
    if (dy >= 0) {
      x = x1;
      y = y1;
      ye = y2;
    } else {
      x = x2;
      y = y2;
      ye = y1;
    }
    Eigen::Vector3f point = Eigen::Vector3f(x, y, 1.0f);
    set_pixel(point, line_color);
    for (i = 0; y < ye; i++) {
      y = y + 1;
      if (py <= 0) {
        py = py + 2 * dx1;
      } else {
        if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
          x = x + 1;
        } else {
          x = x - 1;
        }
        py = py + 2 * (dx1 - dy1);
      }
      //            delay(0);
      Eigen::Vector3f point = Eigen::Vector3f(x, y, 1.0f);
      set_pixel(point, line_color);
    }
  }
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f) {
  return Vector4f(v3.x(), v3.y(), v3.z(), w);
}

// Clip a convex polygon (given in VIEW space) against the camera's clip plane.
//
// The camera looks down -Z, so the plane sits at z = z_near with z_near < 0,
// and a vertex is in front of it when z <= z_near. z_near is normally a tiny
// negative epsilon, i.e. the plane of the camera itself: it cannot be exactly
// zero because the perspective divide is a division by w = z_view.
// This is the Sutherland-Hodgman algorithm applied to a single plane: walk the
// edges of the polygon and, whenever an edge crosses the plane, insert the
// intersection point.
//
// Without this step a vertex behind the eye keeps a positive w, the division
// by w mirrors it through the origin and the projected coordinates explode as
// w approaches zero.
static std::vector<Eigen::Vector4f> clip_against_near(
    const std::vector<Eigen::Vector4f>& in, float z_near) {
  std::vector<Eigen::Vector4f> out;
  out.reserve(in.size() + 1);

  auto inside = [z_near](const Eigen::Vector4f& v) { return v.z() <= z_near; };

  for (size_t i = 0; i < in.size(); ++i) {
    const Eigen::Vector4f& cur = in[i];
    const Eigen::Vector4f& prev = in[(i + in.size() - 1) % in.size()];

    const bool cur_in = inside(cur);
    const bool prev_in = inside(prev);

    // The edge prev->cur crosses the plane: add the intersection point.
    if (cur_in != prev_in) {
      const float t = (z_near - prev.z()) / (cur.z() - prev.z());
      out.push_back(prev + t * (cur - prev));
    }
    // Keep the end point of the edge if it is on the visible side.
    if (cur_in) {
      out.push_back(cur);
    }
  }
  return out;
}

// Take a polygon in object space all the way to screen space:
// model -> view -> near plane clipping -> projection -> divide -> viewport.
std::vector<Eigen::Vector3f> rst::rasterizer::project_polygon(
    const std::vector<Eigen::Vector3f>& object_space) {
  float f1 = (100 - 0.1) / 2.0;
  float f2 = (100 + 0.1) / 2.0;

  // The model and view matrices are applied first so that the polygon can be
  // clipped against the near plane while it is still in view space, where the
  // near plane is simply z == near_plane.
  Eigen::Matrix4f model_view = view * model;

  std::vector<Eigen::Vector4f> polygon;
  polygon.reserve(object_space.size());
  for (const auto& p : object_space) {
    polygon.push_back(model_view * to_vec4(p, 1.0f));
  }

  polygon = clip_against_near(polygon, near_plane);

  std::vector<Eigen::Vector3f> screen;
  if (polygon.size() < 3) {
    return screen;  // completely behind the near plane
  }

  screen.reserve(polygon.size());
  for (const auto& view_space_vertex : polygon) {
    Eigen::Vector4f v = projection * view_space_vertex;

    v /= v.w();

    v.x() = 0.5 * width * (v.x() + 1.0);
    v.y() = 0.5 * height * (v.y() + 1.0);
    v.z() = v.z() * f1 + f2;

    screen.push_back(v.head<3>());
  }

  return screen;
}

void rst::rasterizer::draw(rst::pos_buf_id pos_buffer,
                           rst::ind_buf_id ind_buffer, rst::Primitive type, Eigen::Vector3f& color) {
  auto& buf = pos_buf[pos_buffer.pos_id];

  if (type == rst::Primitive::Triangle) {
    auto& ind = ind_buf[ind_buffer.ind_id];

    for (auto& i : ind) {
      std::vector<Eigen::Vector3f> screen =
          project_polygon({buf[i[0]], buf[i[1]], buf[i[2]]});
      if (screen.empty()) {
        continue;
      }

      // Clipping a triangle can turn it into a quad, so draw the result as a
      // triangle fan.
      for (size_t k = 1; k + 1 < screen.size(); ++k) {
        Triangle t;
        t.setVertex(0, screen[0]);
        t.setVertex(1, screen[k]);
        t.setVertex(2, screen[k + 1]);

        t.setColor(0, 255.0, 0.0, 0.0);
        t.setColor(1, 0.0, 255.0, 0.0);
        t.setColor(2, 0.0, 0.0, 255.0);

        rasterize_wireframe(t, color);
      }
    }
  } else if (type == rst::Primitive::Quad) {
    auto& ind = quad_ind_buf[ind_buffer.ind_id];

    for (auto& i : ind) {
      std::vector<Eigen::Vector3f> screen =
          project_polygon({buf[i[0]], buf[i[1]], buf[i[2]], buf[i[3]]});
      if (screen.empty()) {
        continue;
      }

      if (screen.size() == 4) {
        Quad q;
        for (int k = 0; k < 4; ++k) {
          q.setVertex(k, screen[k]);
          q.setColor(k, 255.0, 255.0, 255.0);
        }
        rasterize_wireframe(q, color);
      } else {
        // Near plane clipping turned the quad into a triangle or a pentagon.
        rasterize_wireframe(screen, color);
      }
    }
  } else {
    throw std::runtime_error(
        "Drawing primitives other than triangle and quad is not implemented "
        "yet!");
  }
}

void rst::rasterizer::rasterize_wireframe(const Triangle& t, Eigen::Vector3f& color) {
  draw_line(t.c(), t.a(), color);
  draw_line(t.c(), t.b(), color);  
  draw_line(t.b(), t.a(), color);
}

// Only the four real edges of the quad - no diagonal.
void rst::rasterizer::rasterize_wireframe(const Quad& q, Eigen::Vector3f& color) {
  draw_line(q.a(), q.b(), color);
  draw_line(q.b(), q.c(), color);
  draw_line(q.c(), q.d(), color);
  draw_line(q.d(), q.a(), color);
}

void rst::rasterizer::rasterize_wireframe(
    const std::vector<Eigen::Vector3f>& poly, Eigen::Vector3f& color) {
  for (size_t i = 0; i < poly.size(); ++i) {
    draw_line(poly[i], poly[(i + 1) % poly.size()], color);
  }
}

void rst::rasterizer::set_model(const Eigen::Matrix4f& m) { model = m; }

void rst::rasterizer::set_view(const Eigen::Matrix4f& v) { view = v; }

void rst::rasterizer::set_projection(const Eigen::Matrix4f& p) {
  projection = p;
}

void rst::rasterizer::set_near(float z_near) { near_plane = z_near; }

void rst::rasterizer::clear(rst::Buffers buff) {
  if ((buff & rst::Buffers::Color) == rst::Buffers::Color) {
    std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
  }
  if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth) {
    std::fill(depth_buf.begin(), depth_buf.end(),
              std::numeric_limits<float>::infinity());
  }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h) {
  frame_buf.resize(w * h);
  depth_buf.resize(w * h);
}

int rst::rasterizer::get_index(int x, int y) {
  return (height - y) * width + x;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f& point,
                                const Eigen::Vector3f& color) {
  // old index: auto ind = point.y() + point.x() * width;
  if (point.x() < 0 || point.x() >= width || point.y() < 0 ||
      point.y() >= height)
    return;
  auto ind = (height - 1 - point.y()) * width + point.x();
  frame_buf[ind] = color;
}
