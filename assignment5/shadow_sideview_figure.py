"""Draw the side-view schematic of the shadow target scene (handout Figure 7).

The figure shows the vertical plane through the light and the pole axis: the
light at height H above its foot, the pole (half-width r, height h) at distance
D, the far point of the top cap, and the ray from the light through that point
to the shadow tip at distance s.

The proportions are consistent but deliberately NOT the scene's values
(shadow_target_scene.hpp: D = 5, r = 0.5, h = 3, s = 9), so the answer cannot
be measured from the figure.

Usage: python3 shadow_sideview_figure.py [output.png]
"""
import sys

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle

# Schematic proportions (not to scale); s follows from similar triangles.
D, r, h, H = 5.0, 0.7, 2.4, 5.6
s = (D + r) * H / (H - h)


def dimension(ax, x0, x1, y, label):
    ax.annotate("", (x0, y), (x1, y), arrowprops=dict(arrowstyle="<->", lw=0.9))
    ax.text((x0 + x1) / 2, y - 0.08, label, ha="center", va="top", fontsize=10, style="italic")


def main(out):
    fig, ax = plt.subplots(figsize=(8, 4.4), dpi=150)

    # Floor
    ax.fill_between([-1.0, s + 1.2], -0.3, 0, color="#f3e7c0")
    ax.plot([-1.0, s + 1.2], [0, 0], color="k", lw=1.5)
    ax.text(-0.95, -0.15, "floor (y = −3)", ha="left", va="center", fontsize=8)

    # Light and its foot
    ax.plot([0, 0], [0, H], ls=":", color="gray", lw=1)
    ax.plot([0], [H], marker="*", ms=17, color="#e0a000", mec="k", zorder=5)
    ax.text(0.3, H, "light (y = kLightY)", fontsize=9, va="center")
    ax.plot([0], [0], marker="o", ms=4, color="k", zorder=5)
    ax.text(0.15, 0.12, "light foot", ha="left", va="bottom", fontsize=8)

    # Pole and its axis
    ax.add_patch(Rectangle((D - r, 0), 2 * r, h, facecolor="#c9cbe8", edgecolor="#3b3b9a", lw=1.5, zorder=3))
    ax.plot([D, D], [0, h + 0.35], ls="-.", color="#3b3b9a", lw=0.8, zorder=4)
    ax.text(D - 0.1, h + 0.4, "pole axis", ha="right", va="bottom", fontsize=8, color="#3b3b9a")

    # Far point of the top cap
    ax.plot([D + r], [h], marker="o", ms=7, color="red", zorder=6)
    ax.annotate("far point of the top cap\n(the point of the cap edge\nfarthest from the light)",
                (D + r, h), xytext=(D + 2.0, h + 1.5), fontsize=8.5, color="red",
                arrowprops=dict(arrowstyle="->", color="red", lw=0.9))

    # Ray from the light through the far point, and the shadow it bounds
    ax.plot([0, s], [H, 0], color="#e0a000", lw=1.6, zorder=4)
    ax.text(s - 1.0, 0.75, "ray from the light\nthrough the far point", fontsize=8, color="#a07000", ha="left")
    ax.plot([D + r, s], [0, 0], color="k", lw=5, solid_capstyle="butt", zorder=4)
    ax.text(D + r + 0.9, 0.12, "shadow", ha="center", va="bottom", fontsize=8)
    ax.plot([s], [0], marker="v", ms=7, color="k", zorder=6)
    ax.text(s, -0.45, "marker centre = shadow tip", ha="center", va="top", fontsize=8.5)

    # Dimensions
    dimension(ax, 0, D, -1.15, "D")
    dimension(ax, D, D + r, -1.15, "r")
    dimension(ax, 0, s, -1.85, "s")
    for x, y0, y1 in [(0, -2.0, -0.35), (D, -1.3, -0.35), (D + r, -1.3, -0.35), (s, -2.0, -0.9)]:
        ax.plot([x, x], [y0, y1], color="gray", lw=0.6, ls=":")
    ax.annotate("", (D - r - 0.3, 0), (D - r - 0.3, h), arrowprops=dict(arrowstyle="<->", lw=0.9))
    ax.text(D - r - 0.45, h / 2, "h", ha="right", va="center", fontsize=10, style="italic")
    ax.annotate("", (-0.4, 0), (-0.4, H), arrowprops=dict(arrowstyle="<->", lw=0.9))
    ax.text(-0.55, H / 2, "H", ha="right", va="center", fontsize=10, style="italic")

    ax.set_xlim(-1.2, s + 1.4)
    ax.set_ylim(-2.4, H + 0.5)
    ax.set_aspect("equal")
    ax.axis("off")
    ax.set_title("Side view: the vertical plane through the light and the pole axis (not to scale)", fontsize=9)
    plt.tight_layout()
    plt.savefig(out)
    print("saved", out)


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "fig_shadow_sideview.png")
