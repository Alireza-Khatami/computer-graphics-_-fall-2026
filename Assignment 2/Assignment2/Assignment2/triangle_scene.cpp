#include "triangle_scene.hpp"

#include <iostream>
#include <vector>

#include "transforms.hpp"

TriangleScene::TriangleScene(rst::rasterizer& r) {
  std::vector<Eigen::Vector3f> pos{{2, -3, -3},    {0, 4, -3},
                                   {-2, -3, -3},   {3.5, -1, -3.5},
                                   {2.5, 1.5, -3.5}, {-1, 0.5, -3.5},
                                   //Front-facing triangle (CCW winding)
                                   {-4, 2, -3}, {-3, 4, -3}, {-5, 4, -3},
                                   // Back-facing triangle (CW winding)
                                   {4, -1 , -3}, {5, 2, -3}, {3, 2, -3}};

  std::vector<Eigen::Vector3i> ind{{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {11, 10, 9}};
  std::vector<float> alpha = {0.5f, 0.5f};
  std::vector<Eigen::Vector4f> cols{{232, 117, 166, alpha[0]}, {232, 117, 166, alpha[0]},
                                     {232, 117, 166, alpha[0]}, {0, 100, 0, alpha[1]},
                                     {0, 100, 0, alpha[1]},     {0, 100, 0, alpha[1]},
                                     // Front-facing triangle (blue, alpha=1)
                                     {0, 100, 255, 1.0f}, {0, 100, 255, 1.0f}, {0, 100, 255, 1.0f},
                                     // Back-facing triangle (yellow, alpha=1)
                                     {255, 255, 0, 1.0f}, {255, 255, 0, 1.0f}, {255, 255, 0, 1.0f}};

  pos_id_ = r.load_positions(pos);
  ind_id_ = r.load_indices(ind);
  col_id_ = r.load_colors(cols);
}

void TriangleScene::render(rst::rasterizer& r) {
  render_with(r, 0, 0);  // depth_test, no culling
}

void TriangleScene::render_with(rst::rasterizer& r, int blend_mode, int cull_mode) {
  r.set_cull_mode(cull_mode);
  r.set_alpha_blend_mode(blend_mode);
  r.set_model(get_model_matrix(0));
  r.set_view(get_view_matrix(eye_pos_, angle_));
  r.set_projection(get_projection_matrix(90, 1, -0.1, -50));
  r.draw(pos_id_, ind_id_, col_id_, rst::Primitive::Triangle);
}

bool TriangleScene::on_key(int key) {
  if (key == 'a') { angle_ += 10; return true; }
  if (key == 'd') { angle_ -= 10; return true; }
  return false;
}

void TriangleScene::print_controls() const {
  std::cout << "\n-- Triangle scene --\n"
            << "  a / d : orbit camera left / right\n"
            << "  TAB   : switch to cube scene\n"
            << "  Esc   : quit\n";
}
