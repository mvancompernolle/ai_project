#include <iostream>

#include <string>
#include <vector>
#include <cassert>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <limits>
#include <cstdlib>
#include "graph.h"

struct GNUplotColor {
  int M_color;			// GNUplot color index.
  int M_Red, M_Green, M_blue;	// RGB value.
  char const* M_name;		// A human readable name for the color.
  char M_c;			// Corresponding symbol used by GNUplot (@ = square, 0 = diamond, ^ = triangle).
};

GNUplotColor const gnuplot_colors[] = {
  { -1, 0, 0, 0, "black", ' ' },
  {  0, 160, 160, 160, "gray", '.' },
  {  1, 255, 0, 0, "red", '+' },
  {  2, 0, 192, 0, "mid green", 'x' },
  {  3, 0, 128, 255, "dark sky blue", '*' },
  {  4, 192, 0, 255, "purple", '@' },
  {  5, 0, 238, 238, "cyan", 'O' },
  {  6, 192, 64, 0, "red brown", '^' },
  {  7, 238, 238, 0, "dirty yellow", '+' },
  {  8, 32, 32, 192, "dark blue", 'x' },
  {  9, 255, 192, 32, "orange", '*' },
  { 10, 0, 128, 64, "forest green", '@' },
  { 11, 160, 128, 255, "light purple", 'O' },
  { 12, 128, 64, 0, "brown", '^' },
  { 13, 255, 128, 255, "pink", '+' },
  { 14, 0, 192, 96, "blueish green", 'x' },
  { 15, 0, 192, 192, "sky blue", '*' },
  { 16, 0, 96, 128, "Delfts blue", '@' },
  { 17, 192, 96, 128, "pink purple", '0' },
  { 18, 0, 128, 0, "dark green", '^' },
  { 19, 64, 255, 128, "bright light green", '+' },
  { 20, 48, 96, 128, "grayish blue", 'x' },
  { 21, 128, 96, 0, "light brown", '*' },
  { 22, 64, 64, 64, "dark gray", '@' },
  { 23, 64, 128, 0, "yellowish dark green", '0' },
  { 24, 0, 0, 128, "navy blue", '^' },
  { 25, 128, 96, 16, "light brown 2", '+' },
  { 26, 128, 96, 96, "purple brown", 'x' },
  { 27, 128, 96, 128, "brownish purple", '*' },
  { 28, 0, 0, 192, "dark blue 2", '@' },
  { 29, 0, 0, 255, "blue", '0' },
  { 30, 0, 96, 0, "dark forest green", '^' },
  { 31, 227, 176, 192, "light pink", '+' },
  { 32, 64, 192, 128, "blue green", 'x' },
  { 33, 96, 160, 192, "blue gray", '*' }
};

int Graph::M_new_color(void)
{
  color_list_type::iterator iter;
  do
  {
    for (iter = M_colors.begin(); iter != M_colors.end(); ++iter)
      if (iter->get_color() == M_color_hint)
	break;
    ++M_color_hint;
  }
  while (iter != M_colors.end());
  return M_color_hint - 1;
}

bool Graph::add_point(int x, int y, int column)
{
  color_list_type::iterator iter;
  for (iter = M_colors.begin(); iter != M_colors.end(); ++iter)
    if (column == iter->get_column())
    {
      M_data.push_back(DataPoint(x, y, iter));
      return true;
    }
  return false;
}

int Graph::add_point(int x, int y, std::string const& label, int color)
{
  int column = -1;
  color_list_type::iterator iter;
  if (color == -2)
  {
    for (iter = M_colors.begin(); iter != M_colors.end(); ++iter)
      if (label == iter->get_label())
      {
        color = iter->get_color();
	column = iter->get_column();
	break;
      }
  }
  else
  {
    for (iter = M_colors.begin(); iter != M_colors.end(); ++iter)
      if (color == iter->get_color())
      {
	assert(iter->get_label() == label);
	column = iter->get_column();
	break;
      }
  }
  if (iter == M_colors.end())
  {
    if (color == -2)
      color = M_new_color();
    column = M_next_column++;
    iter = M_colors.insert(M_colors.end(), Color(color, column, label));
  }
  M_data.push_back(DataPoint(x, y, iter));
  return column;
}

void Graph::plot(void)
{
  int total = 0;
  int min_x = std::numeric_limits<int>::max();
  int max_x = std::numeric_limits<int>::min();
  int min_y = std::numeric_limits<int>::max();
  int max_y = std::numeric_limits<int>::min();

  for (std::vector<DataPoint>::const_iterator iter = M_data.begin(); iter != M_data.end() ; ++iter)
  {
    int x = iter->get_x();
    int y = iter->get_y();
    if (x > max_x)
      max_x = x;
    if (x < min_x)
      min_x = x;
    if (y > max_y)
      max_y = y;
    if (y < min_y)
      min_y = y;
    total += y;
  }

  std::ofstream f;

  std::ostringstream plot_filename;
  plot_filename << "plot" << M_basename << ".dat";
  f.open(plot_filename.str().c_str());

  f << "set terminal png small color picsize 500 400\n"
       "set output \"" << "data" << M_basename << ".png\"\n"
       "set style fill solid 1 border -1\n"
       "set grid ytics mytics back linetype -1 linewidth 0.8, linetype 2 linewidth 0.25\n"
       "set title \"" << M_title << "\"\n"
       "set boxwidth 1 absolute\n"
       "set xlabel \"Number of points on the curve\"\n"
       "set ylabel \"Number of curves\"\n"
       "set xrange [ " << min_x - 0.5 << " : " << max_x + 0.5 << " ]\n";

  int xrange = max_x - min_x + 1;
  int yrange = (int)((max_y + 1) * 1.1);
  int xtics = 1;
  int ms = 20; // m < 7 ? 20 : 10;
  while (xrange / xtics > ms)
    xtics <<= 1;

  f << "set yrange [ 0 : " << yrange << " ]\n"
       "set label 12 \"Total number of curves: " << total << "\" at " <<
       ((min_x - 0.5) + 0.03 * xrange) << ", " << (0.97 * yrange) << " front nopoint tc lt -1\n"
       "set xtics " << xtics << "\n";
  if (xtics > 1)
    f << "set mxtics " << std::min(xtics, 4) << "\n";

#if 0
  f << "set ytics " << m << "\n"
       "set mytics " << m << "\n";
#endif

  f << "plot \\\n";
  for (int column = 0; column < (int)M_colors.size(); ++column)
  {
    for (color_list_type::const_iterator color = M_colors.begin(); color != M_colors.end(); ++color)
      if (color->get_column() == column)
      {
	f << "  \"data" << M_basename << ".dat\" using 1:" << (column + 2) <<
	     " title \"" << color->get_label() << "\" with boxes lt " << color->get_color();
	if (column != (int)M_colors.size() - 1)
	  f << ", \\";
	f << '\n';
      }
  }
  f.close();

  std::ostringstream data_filename;
  data_filename << "data" << M_basename << ".dat";
  f.open(data_filename.str().c_str());

  std::sort(M_data.begin(), M_data.end());
  int prev_x_init = M_data[0].get_x() - 1;
  int prev_x = prev_x_init;
  int prev_column = -1;
  int max_column = M_colors.size();
  int y_offset = 0;	// Initialized to avoid compiler warning.
  for (std::vector<DataPoint>::const_iterator iter = M_data.begin(); iter != M_data.end() ; ++iter)
  {
#if 0
    std::cout << "Processing: x = " << iter->get_x() << "; y = " << iter->get_y() <<
        "; column = " << iter->get_color().get_column() << "; color = " << iter->get_color().get_color() << std::endl;
#endif
    int x = iter->get_x();
    if (x != prev_x)
    {
      if (prev_x != prev_x_init)
      {
	for (int i = 1; i < max_column - prev_column; ++i)
	  f << " ?";
        f << '\n';
      }
      f << x;
      prev_x = x;
      prev_column = -1;
      y_offset = 0;
      for (std::vector<DataPoint>::const_iterator iter2 = iter; iter2 != M_data.end() ; ++iter2)
        if (iter2->get_x() == x)
	  y_offset += iter2->get_y();
        else
	  break;
    }
    int column = iter->get_color().get_column();
    assert(column - prev_column >= 1);			// The same x coordinate but the same columns?!
    for (int i = 1; i < column - prev_column; ++i)
      f << " ?";
    prev_column = column;
    f << " " << y_offset;
    y_offset -= iter->get_y();
  }

  f.close();
}

void Graph::swap_columns(int col1, int col2)
{
  if (col1 == col2)
    return;
  for (color_list_type::iterator iter1 = M_colors.begin(); iter1 != M_colors.end(); ++iter1)
    if (iter1->get_column() == col1 || iter1->get_column() == col2)
    {
      color_list_type::iterator iter2 = iter1;
      for (++iter2; iter2 != M_colors.end(); ++iter2)
	if (iter2->get_column() == col1 || iter2->get_column() == col2)
        {
	  iter1->swap_column_with(*iter2); 
	  return;
        }
    }
}
