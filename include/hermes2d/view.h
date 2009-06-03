// This file is part of Hermes2D.
//
// Copyright 2005-2008 Jakub Cerveny <jakub.cerveny@gmail.com>
// Copyright 2005-2008 Lenka Dubcova <dubcova@gmail.com>
// Copyright 2005-2008 Pavel Solin <solin@unr.edu>
//
// Hermes2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Hermes2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Hermes2D.  If not, see <http://www.gnu.org/licenses/>.

// $Id: view.h 1086 2008-10-21 09:05:44Z jakub $

#ifndef __HERMES2D_VIEW_H
#define __HERMES2D_VIEW_H

#include "common.h"
#include "linear.h"

// default window size and position
#define DEFAULT_WINDOW_POS  int x = -1, int y = -1, int width = 1000, int height = 800


// you can define NOGLUT to turn off all OpenGL stuff in Hermes2D
#ifndef NOGLUT

/// \brief Represents a simple visualization window.
///
/// View is a base class providing a simple OpenGL visualization window.
/// Its task is to define basic functionality, such as the ability of the 
/// window to be responsive even when the main program thread is busy
/// with calculations (ie., the windows are run in a background thread),
/// to provide zooming and panning capabilities for use by the descendant
/// classes, etc.
///
class View
{
public:
  
  View(const char* title, int x, int y, int width, int height);
  virtual ~View();

  int  create();
  void close();

  /// Changes the window name (in its title-bar) to 'title'.
  void set_title(const char* title);

  void set_min_max_range(double min, double max);
  void auto_min_max_range();
  void get_min_max_range(double& min, double& max);

  void show_scale(bool show = true);
  void set_scale_position(int horz, int vert);
  void set_scale_size(int width, int height, int numticks);
  void set_scale_format(const char* fmt);

  /// Saves the current content of the window to a .BMP file.
  /// If 'high_quality' is true, an anti-aliased frame is rendered and saved.
  void save_screenshot(const char* bmpname, bool high_quality = false);
  /// Like save_screenshot(), but forms the file name in printf-style using the 'number'
  /// parameter, e.g., format="screen%03d.bmp" and number=5 gives the file name "screen005.bmp".
  void save_numbered_screenshot(const char* format, int number, bool high_quality = false);

  void set_palette(int type);
  void set_num_palette_steps(int num);
  void set_palette_filter(bool linear);
  
  void wait_for_keypress();
  void wait_for_close();
  void wait_for_draw();
  
  static void wait();

protected:
    
  virtual void clear_background();
  static double get_tick_count();

  virtual void on_create();
  virtual void on_display() = 0;
  virtual void on_reshape(int width, int height);
  virtual void on_mouse_move(int x, int y);
  virtual void on_left_mouse_down(int x, int y);
  virtual void on_left_mouse_up(int x, int y);
  virtual void on_left_mouse_double_click(int x, int y) {}
  virtual void on_right_mouse_down(int x, int y);
  virtual void on_right_mouse_up(int x, int y);
  virtual void on_right_mouse_double_click(int x, int y) {}
  virtual void on_middle_mouse_down(int x, int y) {}
  virtual void on_middle_mouse_up(int x, int y) {}
  virtual void on_middle_mouse_double_click(int x, int y) {}
  virtual void on_key_down(unsigned char key, int x, int y);
  virtual void on_special_key(int key, int x, int y);
  virtual void on_entry(int state) {}
  virtual void on_close();
    
  void post_redisplay();
  void safe_post_redisplay();
  template<class TYPE> void center_mesh(TYPE* vertices, int nvert);
  const float* get_palette_color(double x);
    
protected:
  
  std::string title;
  int window_id;
  int window_x, window_y, window_width, window_height;
  float jitter_x, jitter_y;
  bool hq_frame, frame_ready;

  double scale, log_scale, trans_x, trans_y;
  double center_x, center_y;
  int margin, lspace, rspace;
  int mouse_x, mouse_y;
  int scx, scy;
  double objx, objy;
  bool dragging, scaling;

  int pal_type, pal_steps, pal_filter;
  double tex_scale, tex_shift;
  bool range_auto;
  double range_min, range_max;

  bool b_scale, b_help;
  bool scale_focused, scale_dragging;
  int pos_horz, pos_vert;
  int scale_x, scale_y;
  int scale_width, scale_height, labels_width;
  int scale_numticks, scale_box_height, scale_box_skip;
  char scale_fmt[20];
  
  bool want_screenshot;
  static int screenshot_no;
  std::string screenshot_filename;

  void update_scale();
  void update_log_scale();

  double transform_x(double x) { return (x * scale + trans_x) + center_x; } 
  double transform_y(double y) { return center_y - (y * scale + trans_y); }
  
  void pre_display();
  void display_antialiased();
  
  void set_ortho_projection(bool no_jitter = false);
  void set_3d_projection(int fov, double znear, double zfar);
  
  void draw_text(double x, double y, const char* text, int align = -1);
  int  get_text_width(const char* text);
  
  char *get_screenshot_file_name();
  void save_screenshot_internal(const char* filename);
  
  virtual void scale_dispatch();
  virtual int measure_scale_labels();
  void draw_continuous_scale(char* title, bool righttext);
  void draw_discrete_scale(int numboxes, const char* boxnames[], const float boxcolors[][3]);
  
  void create_palette();
  void update_tex_adjust();
  void set_title_internal(const char* text);
  void update_layout();
  
  void draw_help();
  virtual const char* get_help_text() const { return ""; }

  friend void on_display_stub(void);
  friend void on_reshape_stub(int, int);
  friend void on_mouse_move_stub(int, int);
  friend void on_mouse_click_stub(int, int, int, int);
  friend void on_key_down_stub(unsigned char, int, int);
  friend void on_special_key_stub(int, int, int);
  friend void on_entry_stub(int);
  friend void on_idle_stub();  
  friend void on_close_stub();
  friend int view_create_body(void* param);
  friend int view_set_title_body(void* param);
};


void glut_init(); // deprecated, don't use
void finish_glut_main_loop(bool force = false);  // deprecated, don't use


/// \brief Displays a mesh.
///
/// MeshView is a debugging tool for displaying meshes together with its element
/// id numbers, boundary markers etc.
///
class MeshView : public View
{
public:
  
  MeshView(const char* title = "MeshView", DEFAULT_WINDOW_POS);
  virtual ~MeshView();

  void show(Mesh* mesh);

protected:
  
  Linearizer lin;

  bool b_ids, b_markers;

  struct ObjInfo
  {
    double x, y;
    int id, type;
  };

  ObjInfo* nodes;
  ObjInfo* elems;
  int nn, ne;

  float* get_marker_color(int marker);
  
  virtual void on_display();
  virtual void on_key_down(unsigned char key, int x, int y);
  virtual void scale_dispatch() {}
  virtual const char* get_help_text() const;

};


/// \brief Visualizes a scalar PDE solution.
///
/// ScalarView is a visualization window for all scalar-valued PDE solutions.
///
class ScalarView : public View
{
public:

  ScalarView(const char* title = "ScalarView", DEFAULT_WINDOW_POS);
  virtual ~ScalarView();

  void show(MeshFunction* sln, double eps = EPS_NORMAL, int item = FN_VAL_0,
            MeshFunction* xdisp = NULL, MeshFunction* ydisp = NULL, double dmult = 1.0);
      
  void show_mesh(bool show = true) { lines = show; post_redisplay(); }
  void show_contours(double step, double orig = 0.0);
  void hide_contours() { contours = false; post_redisplay(); }
  void set_3d_mode(bool enable = true) { mode3d = enable; post_redisplay(); }
  
  void load_data(const char* filename);
  void save_data(const char* filename);
  void save_numbered(const char* format, int number);

protected:

  Linearizer lin;

  bool lines, pmode, mode3d, panning, contours;
  double xrot, yrot, xtrans, ytrans, ztrans;
  double xzscale, yscale, xctr, yctr, zctr;
  double cont_orig, cont_step;
  double3* normals;

  void draw_tri_contours(double3* vert, double2* tvert, int3* tri);
  void reset_3d_view();
  void calculate_normals();
  void center_3d_mesh();
  void init_lighting();

  virtual void on_display();
  virtual void on_key_down(unsigned char key, int x, int y);
  virtual void on_mouse_move(int x, int y);
  virtual void on_middle_mouse_down(int x, int y);
  virtual void on_middle_mouse_up(int x, int y);
  virtual const char* get_help_text() const;

};


/// \brief Visualizes the basis functions of a space.
///
/// BaseView is a debugging tool for the visualization of the basis functions
/// of a given space. 
///
class BaseView : public ScalarView
{
public:

  BaseView(const char* title = "BaseView", DEFAULT_WINDOW_POS);

  void show(Space* space, double eps = EPS_LOW, int item = FN_VAL_0);

  virtual ~BaseView() { free(); } 

protected:

  Space* space;
  PrecalcShapeset* pss;
  Solution* sln;

  double eps;
  int ndofs, item;
  int base_index;
  
  void free();
  void update_solution();
  void update_title();

  virtual void on_special_key(int key, int x, int y);
  virtual const char* get_help_text() const;

};


/// \brief Displays the polynomial degrees of elements.
///
/// OrderView is a tool for displaying the polynomial degrees of the elements in a space.
///
class OrderView : public View
{
public:

  OrderView(const char* title = "OrderView", DEFAULT_WINDOW_POS);

  void show(Space* space);

  void load_data(const char* filename);
  void save_data(const char* filename);
  void save_numbered(const char* format, int number);

protected:
  
  Orderizer ord;
  bool b_orders;

  int num_boxes, order_min;
  const char* box_names[11];
  char text_buffer[500];
  float order_colors[11][3];

  void init_order_palette();

  virtual void on_display();
  virtual void on_key_down(unsigned char key, int x, int y);
  virtual void scale_dispatch();
  virtual int measure_scale_labels();
  virtual const char* get_help_text() const;

};


/// \brief Visualizes a vector PDE solution.
///
/// VectorView is a visualization window for all vector-valued PDE solutions.
///
class VectorView : public View
{
public:

  VectorView(const char* title = "VectorView", DEFAULT_WINDOW_POS);

  void show(MeshFunction* vsln, double eps = EPS_NORMAL);
  void show(MeshFunction* xsln, MeshFunction* ysln, double eps = EPS_NORMAL);
  void show(MeshFunction* xsln, MeshFunction* ysln, double eps, int xitem, int yitem);
  
  void set_grid_type(bool hexa) { this->hexa = hexa; post_redisplay(); };

  void load_data(const char* filename);
  void save_data(const char* filename);
  void save_numbered(const char* format, int number);

protected:

  Vectorizer vec;
  double gx, gy, gs;
  bool hexa; // false - quad grid, true - hexa grid
  int mode;  // 0 - magnitude is on the background, 1 - arrows are colored, 2 - no arrows, just magnitude on the background
  bool lines, pmode;
  double length_coef; // for extending or shortening arrows

  void plot_arrow(double x, double y, double xval, double yval, double max, double min, double gs);

  virtual void on_display();
  virtual void on_mouse_move(int x, int y);
  virtual void on_key_down(unsigned char key, int x, int y);
  virtual const char* get_help_text() const;
  
};


class VectorBaseView : public VectorView
{
public:

  VectorBaseView(const char* title = "BaseView", DEFAULT_WINDOW_POS)
    : VectorView(title, x, y, width, height) { pss = NULL; sln = NULL; lines = false; }

  void show(Space* space);

  virtual ~VectorBaseView() { free(); } 

protected:

  Space* space;
  PrecalcShapeset* pss;
  Solution* sln;

  int ndofs, component;
  int base_index;

  void free();
  void update_solution();
  void update_title();

  virtual void on_special_key(int key, int x, int y);
  virtual const char* get_help_text() const;

};


/*class DiscreteProblem;

/// \brief Displays the sparse structure of a matrix.   TODO: update this for LinSystem
///
/// MatrixView is a debugging tool for the visualization of the sparse structure
/// of a matrix associated with a discrete problem. Nonzero matrix elements are
/// displayed as color squares, using the following color code:
/// <ul> <li> Blue - symmetric element
///      <li> Red - unsymmetric element (also structurally unsymmetric)
///      <li> Magenta - antisymmetric element
///      <li> Grey - stored zero element
/// </ul>
class MatrixView : public View
{
public:

  MatrixView(const char* title = "MatrixView", DEFAULT_WINDOW_POS);
  ~MatrixView();
  
  void show(DiscreteProblem *ep);

protected:
  
  double2 verts[4];
  
  int *Ap;
  unsigned *Ai;
  scalar *Ax;
  int size;
  double eps, max;
  double pointsize;
  
  virtual void find_symmetry(int nz, int *sz);
  virtual void assign_color(int i, int j);
  virtual void on_display();
  virtual void on_key_down(unsigned char key, int x, int y);
  virtual void update_title(int nz, int sz);
  virtual void scale_dispatch() {}
  virtual const char* get_help_text() const;

};*/



// this has to be here, unfortunatelly
template<class TYPE>
void View::center_mesh(TYPE* vertices, int nvert)
{
  if (nvert <= 0) return;
  
  // get mesh bounding box
  double xmin = 1e10, xmax = -1e10;
  double ymin = 1e10, ymax = -1e10;
  for (int i = 0; i < nvert; i++)
  {
    if (vertices[i][0] < xmin) xmin = vertices[i][0];
    if (vertices[i][0] > xmax) xmax = vertices[i][0];
    if (vertices[i][1] < ymin) ymin = vertices[i][1];
    if (vertices[i][1] > ymax) ymax = vertices[i][1];
  }
  double mesh_width  = xmax - xmin;
  double mesh_height = ymax - ymin;
  
  double usable_width = window_width - 2*margin - lspace - rspace;
  double usable_height = window_height - 2*margin;
  
  // align in the proper direction
  if (usable_width / usable_height < mesh_width / mesh_height)
    scale = usable_width / mesh_width;
  else
    scale = usable_height / mesh_height;
  
  // center
  trans_x = -scale * (xmin + xmax) / 2;
  trans_y = -scale * (ymin + ymax) / 2;

  update_log_scale();
}


////////////////////////////////////////////////////////////////////////////////////////////////////

// If compiling without OpenGL support, replace all View-based classes with empty stubs, which
// only show a warning that no OpenGL support has been compiled in. The reason for having the 
// empty classes is that you don't have to modify projects using the GL features: all visualization
// will just be skipped.

#else // NOGLUT 


class View
{
public:
  View() {}
  View(const char* title, int x, int y, int width, int height) {}
  ~View() {}
  int  create() {}
  void close() {}
  void set_title(const char* title) {}
  void set_min_max_range(double min, double max) {}
  void auto_min_max_range() {}
  void get_min_max_range(double& min, double& max) {}
  void show_scale(bool show = true) {}
  void set_scale_position(int horz, int vert) {}
  void set_scale_size(int width, int height, int numticks) {}
  void set_scale_format(const char* fmt) {}
  void save_screenshot(const char* bmpname, bool high_quality = false) {}
  void save_numbered_screenshot(const char* format, int number, bool high_quality = false) {}
  void set_palette(int type) {}
  void set_num_palette_steps(int num) {}
  void set_palette_filter(bool linear) {}
  void wait_for_keypress() {}
  void wait_for_close() {}
  void wait_for_draw() {}
  static void wait() {}
};


class MeshView : public View
{
public:
  MeshView(const char* title = "MeshView", DEFAULT_WINDOW_POS) {}
  virtual ~MeshView() {}
  void show(Mesh* mesh)
     { info("MeshView: Hermes2D compiled without OpenGL support, skipping visualization."); }
};


class ScalarView : public View
{
public:
  ScalarView(const char* title = "ScalarView", DEFAULT_WINDOW_POS) {}
  virtual ~ScalarView() {}
  void show(MeshFunction* sln, double eps = EPS_NORMAL, int item = FN_VAL_0,
            MeshFunction* xdisp = NULL, MeshFunction* ydisp = NULL, double dmult = 1.0)
     { info("ScalarView: Hermes2D compiled without OpenGL support, skipping visualization."); }
  void show_mesh(bool show = true) {}
  void show_contours(double step, double orig = 0.0) {}
  void hide_contours() {}
  void set_3d_mode(bool enable = true) {}
  void load_data(const char* filename) {}
  void save_data(const char* filename) {}
  void save_numbered(const char* format, int number) {}
};


class BaseView : public ScalarView
{
public:
  BaseView(const char* title = "BaseView", DEFAULT_WINDOW_POS) {}
  virtual ~BaseView() {}
  void show(Space* space, double eps = EPS_LOW, int item = FN_VAL_0)
     { info("BaseView: Hermes2D compiled without OpenGL support, skipping visualization."); }
};


class OrderView : public View
{
public:
  OrderView(const char* title = "OrderView", DEFAULT_WINDOW_POS) {}
  void show(Space* space)
     { info("OrderView: Hermes2D compiled without OpenGL support, skipping visualization."); }
  void load_data(const char* filename) {}
  void save_data(const char* filename) {}
  void save_numbered(const char* format, int number) {}
};


class VectorView : public View
{
public:
  VectorView(const char* title = "VectorView", DEFAULT_WINDOW_POS) {}
  void show(MeshFunction* vsln, double eps = EPS_NORMAL)
     { info("VectorView: Hermes2D compiled without OpenGL support, skipping visualization."); }
  void show(MeshFunction* xsln, MeshFunction* ysln, double eps = EPS_NORMAL)
     { info("VectorView: Hermes2D compiled without OpenGL support, skipping visualization."); }
  void show(MeshFunction* xsln, MeshFunction* ysln, double eps, int xitem, int yitem)
     { info("VectorView: Hermes2D compiled without OpenGL support, skipping visualization."); }
  void set_grid_type(bool hexa) {}
  void load_data(const char* filename) {}
  void save_data(const char* filename) {}
  void save_numbered(const char* format, int number) {}
};


class VectorBaseView : public VectorView
{
public:
  VectorBaseView(const char* title = "BaseView", DEFAULT_WINDOW_POS) {}
  virtual ~VectorBaseView() {} 
  void show(Space* space)
     { info("VectorBaseView: Hermes2D compiled without OpenGL support, skipping visualization."); }
};


#endif // NOGLUT

#endif
